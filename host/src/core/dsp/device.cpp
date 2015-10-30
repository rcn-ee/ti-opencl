/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "../platform.h"
#include "device.h"
#include "buffer.h"
#include "kernel.h"
#include "program.h"
#include <cstdlib>
#include <algorithm>
#include <limits.h>
#include "CL/cl_ext.h"

#include <core/config.h>
#include "../propertylist.h"
#include "../commandqueue.h"
#include "../events.h"
#include "../memobject.h"
#include "../kernel.h"
#include "../program.h"
#include "../util.h"

#include "driver.h"

#if defined(DEVICE_K2X)
extern "C"
{
    #include <ti/runtime/mmap/include/mmap_resource.h>
    extern void free_ocl_qmss_res();
}
#endif

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Coal;


/******************************************************************************
* On DSPC868X the mailboxes are remote on the device DDR. On Hawking the 
* mailboxes are in shared DDR
******************************************************************************/
#ifdef DSPC868X
#define MAILBOX_LOCATION MPM_MAILBOX_MEMORY_LOCATION_REMOTE 
#else
#define MAILBOX_LOCATION MPM_MAILBOX_MEMORY_LOCATION_LOCAL 

#include "shmem.h"
unsigned dsp_speed()
{
    const unsigned DSP_PLL = 122880000;
    const unsigned pagesize = 0x1000;

    shmem_persistent bootcfg_page;
    shmem_persistent clock_page;

    bootcfg_page.configure(0x02620000, pagesize);
    clock_page.configure(0x02310000, pagesize);

    char *BOOTCFG_BASE_ADDR = (char*)bootcfg_page.map(0x02620000, pagesize);
    char *CLOCK_BASE_ADDR   = (char*)clock_page.map(0x02310000, pagesize);

    int MAINPLLCTL0 = (*(int*)(BOOTCFG_BASE_ADDR + 0x350));
    int MULT        = (*(int*)(CLOCK_BASE_ADDR + 0x110));
    int OUTDIV      = (*(int*)(CLOCK_BASE_ADDR + 0x108));

    unsigned mult = 1 + ((MULT & 0x3F) | ((MAINPLLCTL0 & 0x7F000) >> 6));
    unsigned prediv = 1 + (MAINPLLCTL0 & 0x3F);
    unsigned output_div = 1 + ((OUTDIV >> 19) & 0xF);
    unsigned speed = DSP_PLL * mult / prediv / output_div;

    bootcfg_page.unmap(BOOTCFG_BASE_ADDR, pagesize);
    clock_page.unmap(CLOCK_BASE_ADDR, pagesize);

    return speed / 1000000;
}
#endif

/*-----------------------------------------------------------------------------
* Declare our threaded dsp handler function
*----------------------------------------------------------------------------*/
void *dsp_worker_event_dispatch   (void* data);
void *dsp_worker_event_completion (void* data);
void HOSTwait   (unsigned char dsp_id);


#if defined (DSPC868X) || defined (DEVICE_K2X)
#include "device_keystone.cpp"
#elif defined (DEVICE_AM57)
#include "device_am57x.cpp"
#else
#error "Device not supported"
#endif

/*-----------------------------------------------------------------------------
* If ULM library was not available don't emit ULM trace messages
*----------------------------------------------------------------------------*/
#if !defined (ULM_ENABLED)
#define ulm_put_mem(a,b,c)
#define ulm_config()
#define ulm_term()
#endif
void DSPDevice::init_ulm(uint64_t gsize1, uint64_t gsize2, uint64_t gsize3)
{
    ulm_config();
    ulm_put_mem(ULM_MEM_IN_DATA_ONLY,     p_size_msmc_mem >> 16  , 1.0f);
    ulm_put_mem(ULM_MEM_EX_DATA_ONLY,     (gsize2 + gsize3) >> 16, 1.0f);
    ulm_put_mem(ULM_MEM_EX_CODE_AND_DATA, gsize1 >> 16           , 1.0f);
}


void DSPDevice::setup_memory(DSPDevicePtr64 &global1, DSPDevicePtr64 &global2,
                             DSPDevicePtr64 &global3,
                             uint64_t &gsize1, uint64_t &gsize2,
                             uint64_t &gsize3)
{
    Driver *driver = Driver::instance();

    driver->split_ddr_memory(p_addr64_global_mem, p_size64_global_mem,
                             global1, gsize1, global2, gsize2, gsize3);

    driver->shmem_configure(global1,              gsize1, 0);
    if (gsize2 > 0) driver->shmem_configure(global2, gsize2, 0);
    if (gsize3 > 0) driver->shmem_configure(global3, gsize3, 0);
    driver->shmem_configure(p_addr_msmc_mem,      p_size_msmc_mem, 1);

    // Moved to mbox_impl_mph.h
    // driver->shmem_configure(p_addr_mbox_d2h_phys, p_size_mbox_d2h);
    // driver->shmem_configure(p_addr_mbox_h2d_phys, p_size_mbox_h2d);

    for (int core=0; core < dspCores(); core++)
        driver->shmem_configure(((0x10 + core) << 24) + p_addr_local_mem,    
                                p_size_local_mem);


    /*-------------------------------------------------------------------------
    * Setup the DSP heaps for memory allocation
    *------------------------------------------------------------------------*/
    p_device_ddr_heap1.configure(global1,          gsize1);
    p_device_ddr_heap2.configure(global2,          gsize2, true);
    p_device_ddr_heap3.configure(global3,          gsize3, true);
    p_device_msmc_heap.configure(p_addr_msmc_mem,  p_size_msmc_mem);
}



/******************************************************************************
* void DSPDevice::init()
******************************************************************************/
void DSPDevice::init()
{
    if (p_initialized) return;

    /*-------------------------------------------------------------------------
    * Initialize the locking machinery and create worker threads
    *------------------------------------------------------------------------*/
    pthread_cond_init(&p_events_cond, 0);
    pthread_mutex_init(&p_events_mutex, 0);
    pthread_cond_init(&p_worker_cond, 0);
    pthread_mutex_init(&p_worker_mutex, 0);
    pthread_create(&p_worker_dispatch,   0, &dsp_worker_event_dispatch,   this);
    pthread_create(&p_worker_completion, 0, &dsp_worker_event_completion, this);

    p_initialized = true;
}

/******************************************************************************
* DSPDevice::~DSPDevice()
******************************************************************************/
DSPDevice::~DSPDevice()
{
    /*-------------------------------------------------------------------------
    * Inform the cores on the device to stop listening for commands
    *------------------------------------------------------------------------*/
    mail_to(exitMsg);

    if (p_initialized)
    {
        /*---------------------------------------------------------------------
        * Terminate the workers and wait for them
        *--------------------------------------------------------------------*/
        pthread_mutex_lock(&p_events_mutex);

        p_stop = true;

        pthread_cond_broadcast(&p_events_cond);
        pthread_mutex_unlock(&p_events_mutex);

        pthread_join(p_worker_dispatch, 0);
        pthread_join(p_worker_completion, 0);

        pthread_mutex_destroy(&p_events_mutex);
        pthread_cond_destroy(&p_events_cond);
        pthread_mutex_destroy(&p_worker_mutex);
        pthread_cond_destroy(&p_worker_cond);
    }

#if defined(DEVICE_K2X) || defined(DSPC868X)
    /*-------------------------------------------------------------------------
    * Wait for the EXIT acknowledgement from device
    *------------------------------------------------------------------------*/
    while (! p_exit_acked)
    {
        while (! mail_query()) usleep(1);
        mail_from();
    }
#endif

    delete p_mb;
    p_mb = NULL;

    /*-------------------------------------------------------------------------
    * Free any ulm resources used.
    *------------------------------------------------------------------------*/
    ulm_term();

    /*-------------------------------------------------------------------------
    * Only need to close the driver for one of the devices
    *------------------------------------------------------------------------*/
    if (p_dsp_id == 0) Driver::instance()->close(); 

#if defined(DEVICE_K2X)
    free_ocl_qmss_res();
#endif
}

/******************************************************************************
* DeviceBuffer *DSPDevice::createDeviceBuffer(MemObject *buffer)
******************************************************************************/
DeviceBuffer *DSPDevice::createDeviceBuffer(MemObject *buffer, cl_int *rs)
    { return (DeviceBuffer *)new DSPBuffer(this, buffer, rs); }

/******************************************************************************
* DeviceProgram *DSPDevice::createDeviceProgram(Program *program)
******************************************************************************/
DeviceProgram *DSPDevice::createDeviceProgram(Program *program)
    { return (DeviceProgram *)new DSPProgram(this, program); }

/******************************************************************************
* DeviceKernel *DSPDevice::createDeviceKernel(Kernel *kernel,
******************************************************************************/
DeviceKernel *DSPDevice::createDeviceKernel(Kernel *kernel,
                                llvm::Function *function)
    { return (DeviceKernel *)new DSPKernel(this, kernel, function); }

/******************************************************************************
* cl_int DSPDevice::initEventDeviceData(Event *event)
******************************************************************************/
cl_int DSPDevice::initEventDeviceData(Event *event)
{
    cl_int ret_code = CL_SUCCESS;

    switch (event->type())
    {
        case Event::MapBuffer:
        {
            MapBufferEvent *e = (MapBufferEvent*) event;

            if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR)
            {
                e->setPtr((char*)e->buffer()->host_ptr() + e->offset());
                break;
            }

            DSPBuffer      *buf  = (DSPBuffer*) e->buffer()->deviceBuffer(this);
            DSPDevicePtr64  data = buf->data() + e->offset();

            // DO NOT INVALIDATE! Here only initializes host_addr, it cannot
            // be used before MapBuffer event is scheduled and processed!
            void* host_addr = Driver::instance()->map(this, data, e->cb(),
                                                      false, true);
            e->setPtr(host_addr);

            // (main thread) Retain this event, to be saved in buffer mapped
            // events, and later to be released by UnmapMemObject()
            if (host_addr != NULL) clRetainEvent(desc(e));
            else                   ret_code = CL_MAP_FAILURE;
            break;
        }

        case Event::MapImage: break;

        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
            KernelEvent *e    = (KernelEvent *)event;
            Program     *p    = (Program *)e->kernel()->parent();
            DSPProgram  *prog = (DSPProgram *)p->deviceDependentProgram(this);

            /*-----------------------------------------------------------------
            * Just in time loading 
            *----------------------------------------------------------------*/
            if (!prog->is_loaded() && !prog->load()) 
                return CL_MEM_OBJECT_ALLOCATION_FAILURE;

            DSPKernel *dspkernel = (DSPKernel*)e->deviceKernel();

            cl_int ret = dspkernel->preAllocBuffers();
            if (ret != CL_SUCCESS) return ret;

            // ASW TODO do something

            // Set device-specific data
            DSPKernelEvent *dsp_e = new DSPKernelEvent(this, e);
            ret_code = dsp_e->get_ret_code();
            e->setDeviceData((void *)dsp_e);
            break;
        }
        default: break;
    }

    return ret_code;
}

/******************************************************************************
* void DSPDevice::freeEventDeviceData(Event *event)
******************************************************************************/
void DSPDevice::freeEventDeviceData(Event *event)
{
    switch (event->type())
    {
        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
            DSPKernelEvent *dsp_e = (DSPKernelEvent *)event->deviceData();
            if (dsp_e) delete dsp_e;
        }
        default: break;
    }
}

/******************************************************************************
* void DSPDevice::pushEvent(Event *event)
******************************************************************************/
void DSPDevice::pushEvent(Event *event)
{
    /*-------------------------------------------------------------------------
    * Add an event in the list
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);

    p_events.push_back(event);
    p_num_events++;                 // Way faster than STL list::size() !

    pthread_cond_broadcast(&p_events_cond);
    pthread_mutex_unlock(&p_events_mutex);
}

bool DSPDevice::stop()           { return p_stop; }
bool DSPDevice::availableEvent() { return p_num_events > 0; }

/******************************************************************************
* Event *DSPDevice::getEvent(bool &stop)
******************************************************************************/
Event *DSPDevice::getEvent(bool &stop)
{
    /*-------------------------------------------------------------------------
    * Return the first event in the list, if any. Remove it if it is a
    * single-shot event.
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);

    while (p_num_events == 0 && !p_stop)
        pthread_cond_wait(&p_events_cond, &p_events_mutex);

    if (p_stop)
    {
        pthread_mutex_unlock(&p_events_mutex);
        stop = true;
        return 0;
    }

    Event *event = p_events.front();
    p_num_events--;
    p_events.pop_front();

    pthread_mutex_unlock(&p_events_mutex);

    return event;
}

void DSPDevice::push_complete_pending(uint32_t idx, Event* const data, 
                                      unsigned int cnt)
    { p_complete_pending.push(idx, data, cnt); }

bool DSPDevice::get_complete_pending(uint32_t idx, Event*& data)
    { return p_complete_pending.try_pop(idx, data); }

int  DSPDevice::num_complete_pending()
    { return p_complete_pending.size(); }

void DSPDevice::dump_complete_pending() { p_complete_pending.dump(); }

bool DSPDevice::any_complete_pending() { return !p_complete_pending.empty(); }

/******************************************************************************
* Device's decision about whether CommandQueue should push more events over
* This number could be tuned (e.g. using ooo example).  Note that p_num_events
* are in device's queue, but not yet executed.
******************************************************************************/
bool DSPDevice::gotEnoughToWorkOn() { return p_num_events > 0; }

/******************************************************************************
* Getter functions
******************************************************************************/
bool          DSPDevice::hostSchedule() const { return p_core_mail;   }
unsigned int  DSPDevice::dspCores()      const { return p_cores;   }
float         DSPDevice::dspMhz()       const { return p_dsp_mhz; }
unsigned char DSPDevice::dspID()        const { return p_dsp_id;  }
DLOAD_HANDLE  DSPDevice::dload_handle() const { return p_dload_handle;  }


int DSPDevice::load(const char *filename)
{ 
   if (!p_dload_handle)
   {
       p_dload_handle = DLOAD_create((void*)this);
       DLOAD_initialize(p_dload_handle);
   }

   FILE *fp = fopen(filename, "rb");
   if (!fp) { printf("can't open OpenCL Program file\n"); exit(1); }

   // for multiple application host threads
   Lock lock(this);
   int prog_handle = DLOAD_load(p_dload_handle, fp);

   fclose(fp);
   return prog_handle;
}

bool DSPDevice::unload(int file_handle)
{ 
   if (p_dload_handle)
   {
       // for multiple application host threads
       Lock lock(this);
       bool retval = DLOAD_unload(p_dload_handle, file_handle);

       return retval;
   }
   return false;
}

DSPDevicePtr DSPDevice::get_L2_extent(uint32_t &size)
{ 
    size       = (uint32_t) p_size_local_mem;
    return (DSPDevicePtr) p_addr_local_mem;
}

DSPDevicePtr DSPDevice::malloc_msmc(size_t size)  
{ 
    DSPDevicePtr ret = p_device_msmc_heap.malloc(size,true); 
    if (ret) dsptop_msmc();
    return ret;
}

void DSPDevice::free_msmc(DSPDevicePtr addr)       
{ 
    p_device_msmc_heap.free(addr); 
    dsptop_msmc();
}

// TODO: examine the flag, the logic, etc
#define FRACTION_PERSISTENT_FOR_BUFFER	8
DSPDevicePtr64 DSPDevice::malloc_global(size_t size, bool prefer_32bit)  
{ 
    if (prefer_32bit) 
    {
        DSPDevicePtr64 ret = p_device_ddr_heap1.malloc(size, true);
        if (ret) dsptop_ddr_fixed();
        return ret;
    }

    DSPDevicePtr64 addr = 0;
    uint64_t size64 = 0;
    uint32_t block_size;

    p_device_ddr_heap1.max_block_size(size64, block_size);

    if (size64 / size > FRACTION_PERSISTENT_FOR_BUFFER)
    {
        addr = p_device_ddr_heap1.malloc(size, true);
        if (addr) dsptop_ddr_fixed();
    }
    if (!addr)
    {
        // addr = Driver::instance()->cmem_ondemand_malloc(size);
        addr = p_device_ddr_heap2.malloc(size, true);
        if (addr) dsptop_ddr_extended();
    }
    if (!addr)
    {
        addr = p_device_ddr_heap3.malloc(size, true);
        if (addr) dsptop_ddr_extended();
    }
    if (!addr)
    {
        addr = p_device_ddr_heap1.malloc(size, true); // give it another chance
        if (addr) dsptop_ddr_fixed();
    }

    return addr;
}

void DSPDevice::free_global(DSPDevicePtr64 addr)       
{
    if (addr < DSP_36BIT_ADDR)
    {
        p_device_ddr_heap1.free(addr); 
        dsptop_ddr_fixed();
    }
    else 
    {
        // Driver::instance()->cmem_ondemand_free(addr);
        if (p_device_ddr_heap2.free(addr) == -1)
            p_device_ddr_heap3.free(addr);
        dsptop_ddr_extended();
    }
}

void DSPDevice::dsptop_msmc()
{
    uint64_t k64block_size = p_device_msmc_heap.size() >> 16;
    float    pctfree  =  p_device_msmc_heap.available(); 
             pctfree /=  p_device_msmc_heap.size();

    ulm_put_mem(ULM_MEM_IN_DATA_ONLY, k64block_size, pctfree);
}

void DSPDevice::dsptop_ddr_fixed()
{
    uint64_t k64block_size = p_device_ddr_heap1.size() >> 16;
    float    pctfree  =  p_device_ddr_heap1.available(); 
             pctfree /=  p_device_ddr_heap1.size();

    ulm_put_mem(ULM_MEM_EX_CODE_AND_DATA, k64block_size, pctfree);
}

void DSPDevice::dsptop_ddr_extended()
{
    uint64_t ext_size = p_device_ddr_heap2.size() + p_device_ddr_heap3.size();
    uint64_t k64block_size = ext_size >> 16;
    float    pctfree  =  p_device_ddr_heap2.available() + p_device_ddr_heap3.available();

    if (ext_size != 0) pctfree /= ext_size;
    else               pctfree  = 0.0f;

    ulm_put_mem(ULM_MEM_EX_DATA_ONLY, k64block_size, pctfree);
}


void* DSPDevice::clMalloc(size_t size, cl_mem_flags flags)
{
    DSPDevicePtr64 phys_addr = 0;
    void          *host_addr = NULL;
    bool           use_msmc  = ((flags & CL_MEM_USE_MSMC_TI) != 0);

    if (use_msmc)  phys_addr =   malloc_msmc(size);
    else           phys_addr = malloc_global(size, false);

    if (phys_addr != 0)
    {
        host_addr = Driver::instance()->map(this, phys_addr, size, false, true);
        if (host_addr)
        {
            Lock lock(this);
            p_clMalloc_mapping[host_addr] = PhysAddrSizeFlagsTriple(
                           PhysAddrSizePair(phys_addr, size), flags);
        }
        else
        {
            if (use_msmc)   free_msmc(phys_addr);
            else          free_global(phys_addr);
        }
    }
    return host_addr;
}

void DSPDevice::clFree(void* ptr)
{
    Lock lock(this);
    clMallocMapping::iterator it = p_clMalloc_mapping.find(ptr);
    if (it != p_clMalloc_mapping.end())
    {
        PhysAddrSizeFlagsTriple phys_a_s_f = (*it).second;
        DSPDevicePtr64 phys_addr = phys_a_s_f.first.first;
        size_t         size      = phys_a_s_f.first.second;
        cl_mem_flags   flags     = phys_a_s_f.second;
        p_clMalloc_mapping.erase(it);
        Driver::instance()->unmap(this, ptr, phys_addr, size, false);
        if (flags & CL_MEM_USE_MSMC_TI)  free_msmc(phys_addr);
        else                           free_global(phys_addr);
    }
    else printf("clFree invalid pointer\n");
}

// Support query of host ptr in the middle of a clMalloced region
bool DSPDevice::clMallocQuery(void* ptr, DSPDevicePtr64* p_addr, size_t* p_size)
{
    Lock lock(this);
    if (p_clMalloc_mapping.empty())  return false;

    clMallocMapping::iterator it = p_clMalloc_mapping.upper_bound(ptr);
    if (it == p_clMalloc_mapping.begin())  return false;

    // map has bidirectional iterator, so --it is defined, even at end()
    PhysAddrSizeFlagsTriple phys_a_s_f = (*--it).second;
    // (ptr >= (*it).first) must hold because of upper_bound() call
    if (ptr >= (char*)(*it).first + phys_a_s_f.first.second)  return false;

    ptrdiff_t offset = ((char *) ptr) - ((char *) (*it).first);
    if (p_addr)  *p_addr = phys_a_s_f.first.first + offset;
    if (p_size)  *p_size = phys_a_s_f.first.second - offset;
    return true;
}

bool DSPDevice::isInClMallocedRegion(void *ptr)
{
    return clMallocQuery(ptr, NULL, NULL);
}

int DSPDevice::numHostMails(Msg_t &msg) const
{
    if (hostSchedule() && (msg.command == EXIT || msg.command == CACHEINV ||
                           (msg.command == NDRKERNEL && !IS_DEBUG_MODE(msg))))
        return dspCores();
    return 1;
}

void DSPDevice::mail_to(Msg_t &msg, unsigned int core)
{
    switch(msg.command)
    {
        /*---------------------------------------------------------------------
        * for hostScheduled platforms, broadcast the following messages
        *--------------------------------------------------------------------*/
        case EXIT:
        case CACHEINV:
        case NDRKERNEL:
            if (hostSchedule())
            {
                for (int i = 0; i < numHostMails(msg); i++)
                    p_mb->to((uint8_t*)&msg, sizeof(Msg_t), i);
                return;
            }
            // fall through

       case TASK:
           if (hostSchedule() && IS_OOO_TASK(msg))
           {
               static int counter = 0;
               int dsp_id = ((counter++ & 0x1) == 0) ? 0 : 1;
               p_mb->to((uint8_t*)&msg, sizeof(Msg_t), dsp_id);
               return;
           }
           // fall through
          
        /*---------------------------------------------------------------------
        * otherwise send it to the designated core
        *--------------------------------------------------------------------*/
        default: 
            p_mb->to((uint8_t*)&msg, sizeof(Msg_t), core);
    }
}

bool DSPDevice::mail_query()
{
    return p_mb->query();
}

int DSPDevice::mail_from()
{
    uint32_t size_rx, trans_id_rx;
    Msg_t    rxmsg;

    trans_id_rx = p_mb->from((uint8_t*)&rxmsg, &size_rx);
    
    if (rxmsg.command == ERROR)
    {
        printf("%s", rxmsg.u.message);
        return -1;
    }

    if (rxmsg.command == PRINT)
    {
        printf("[core %c] %s", rxmsg.u.message[0], rxmsg.u.message+1);
        return -1;
    }

    if (rxmsg.command == EXIT)
    {
        p_exit_acked = true;
        return -1;
    }

    return trans_id_rx;
}

#if defined(DEVICE_K2X)
/******************************************************************************
* void* DSPDevice::get_mpax_default_res, only need to be computed once
******************************************************************************/
void* DSPDevice::get_mpax_default_res()
{
    if (p_mpax_default_res == NULL)
    {
        p_mpax_default_res = malloc(sizeof(keystone_mmap_resources_t));
        memset(p_mpax_default_res, 0, sizeof(keystone_mmap_resources_t));

#define NUM_VIRT_HEAPS  2
        uint32_t xmc_regs[MAX_XMCSES_MPAXS] = {3, 4, 5, 6, 7, 8, 9};
        uint32_t ses_regs[MAX_XMCSES_MPAXS] = {1, 2, 3, 4, 5, 6, 7};
        uint32_t heap_base[NUM_VIRT_HEAPS]  = {0xC0000000, 0x80000000};
        uint32_t heap_size[NUM_VIRT_HEAPS]  = {0x40000000, 0x20000000};
        for (int i = 0; i < MAX_XMCSES_MPAXS; i++)
        {
            xmc_regs[i] = FIRST_FREE_XMC_MPAX + i;
            ses_regs[i] = FIRST_FREE_SES_MPAX + i;
        }
        keystone_mmap_resource_init(MAX_XMCSES_MPAXS, xmc_regs, ses_regs,
				    NUM_VIRT_HEAPS, heap_base, heap_size,
                           (keystone_mmap_resources_t *) p_mpax_default_res);

    }
    return p_mpax_default_res;
}
#endif  // #ifdef DEVICE_K2X

/******************************************************************************
* cl_int DSPDevice::info
******************************************************************************/
cl_int DSPDevice::info(cl_device_info param_name,
                       size_t param_value_size,
                       void *param_value,
                       size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union 
    {
        cl_device_type cl_device_type_var;
        cl_uint cl_uint_var;
        size_t size_t_var;
        cl_ulong cl_ulong_var;
        cl_bool cl_bool_var;
        cl_device_fp_config cl_device_fp_config_var;
        cl_device_mem_cache_type cl_device_mem_cache_type_var;
        cl_device_local_mem_type cl_device_local_mem_type_var;
        cl_device_exec_capabilities cl_device_exec_capabilities_var;
        cl_command_queue_properties cl_command_queue_properties_var;
        cl_platform_id cl_platform_id_var;
        size_t work_dims[MAX_WORK_DIMS];
    };

    uint64_t  maxblock;
    uint32_t  dummy;

    switch (param_name)
    {
        case CL_DEVICE_TYPE:
            SIMPLE_ASSIGN(cl_device_type, CL_DEVICE_TYPE_ACCELERATOR);
            break;

        case CL_DEVICE_VENDOR_ID:
            SIMPLE_ASSIGN(cl_uint, 0); // TODO
            break;

        case CL_DEVICE_MAX_COMPUTE_UNITS:
            SIMPLE_ASSIGN(cl_uint, dspCores());
            break;

        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
            SIMPLE_ASSIGN(cl_uint, MAX_WORK_DIMS);
            break;

        case CL_DEVICE_MAX_WORK_GROUP_SIZE: {
	    size_t wgsize = 0x40000000;
	    char *str_wgsize = getenv("TI_OCL_WG_SIZE_LIMIT");
	    if (str_wgsize)
	    {
	       size_t env_wgsize = atoi(str_wgsize);
	       if (env_wgsize > 0 && env_wgsize < 0x40000000)
		  wgsize = env_wgsize;
	    }
            SIMPLE_ASSIGN(size_t, wgsize); 
            break;                           }

        case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            for (int i=0; i<MAX_WORK_DIMS; ++i) work_dims[i] = 0x40000000;
            value_length = MAX_WORK_DIMS * sizeof(size_t);
            value        = &work_dims;
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
            SIMPLE_ASSIGN(cl_uint, 1);
            break;

        case CL_DEVICE_MAX_CLOCK_FREQUENCY:
            SIMPLE_ASSIGN(cl_uint, dspMhz());
            break;

        case CL_DEVICE_ADDRESS_BITS:
            SIMPLE_ASSIGN(cl_uint, 32);
            break;

        case CL_DEVICE_MAX_READ_IMAGE_ARGS:
            SIMPLE_ASSIGN(cl_uint, 0);          //images not supported
            break;

        case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
            SIMPLE_ASSIGN(cl_uint, 0);          // images not supported
            break;

        /*---------------------------------------------------------------------
        * Capped at 1GB, primarily because that is the max buffer that can be 
        * mapped into DSP addr space using mpax.  If there are no extended
        * memory available, reserve 16MB for loading OCL program code.
        *--------------------------------------------------------------------*/
        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
        {
            static cl_ulong cap = 0;
            if(cap == 0)
            {
                cap = (cl_ulong)1ul << 30;

                char *cap_str = getenv("TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE");
                if(cap_str)
                {
                    errno = 0;
                    char *tmp;
                    cap = strtoul(cap_str, &tmp, 10);
                    if(errno != 0 || tmp == cap_str || *tmp != '\0' || cap == 0)
                    {
                        printf("ERROR: TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE "
                               "must be a positive integer\n");
                        exit(1);
                    }
                }
            }

            if (p_device_ddr_heap2.size() + p_device_ddr_heap3.size() > 0)
                SIMPLE_ASSIGN(cl_ulong,
                    std::min(p_device_ddr_heap1.size(), cap))
            else
                SIMPLE_ASSIGN(cl_ulong,
                    std::min(p_device_ddr_heap1.size() - (1<<24), cap))

            break;
        }

        case CL_DEVICE_IMAGE2D_MAX_WIDTH:
            SIMPLE_ASSIGN(size_t, 0);           // images not supported
            break;

        case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE3D_MAX_WIDTH:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE3D_MAX_DEPTH:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE_SUPPORT:
            SIMPLE_ASSIGN(cl_bool, CL_FALSE);   //images not supported
            break;

        case CL_DEVICE_MAX_PARAMETER_SIZE:
            SIMPLE_ASSIGN(size_t, 1024);
            break;

        case CL_DEVICE_MAX_SAMPLERS:
            SIMPLE_ASSIGN(cl_uint, 0);          //images not supported
            break;

        case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
            SIMPLE_ASSIGN(cl_uint, 1024);       // 128 byte aligned
            break;

        case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
            SIMPLE_ASSIGN(cl_uint, 128);
            break;

        case CL_DEVICE_SINGLE_FP_CONFIG:
           SIMPLE_ASSIGN(cl_device_fp_config, 
                    CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO |
                    CL_FP_ROUND_TO_INF | CL_FP_INF_NAN );
            break;

        case CL_DEVICE_DOUBLE_FP_CONFIG:
            SIMPLE_ASSIGN(cl_device_fp_config,
                    CL_FP_FMA | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO |
                    CL_FP_ROUND_TO_INF | CL_FP_INF_NAN | CL_FP_DENORM);
           break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            SIMPLE_ASSIGN(cl_device_mem_cache_type, CL_READ_WRITE_CACHE);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
            SIMPLE_ASSIGN(cl_uint, 128);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
            SIMPLE_ASSIGN(cl_ulong, 128*1024);
            break;

        case CL_DEVICE_GLOBAL_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, p_device_ddr_heap1.size());
            break;
            
        case CL_DEVICE_GLOBAL_EXT1_MEM_SIZE_TI:
            SIMPLE_ASSIGN(cl_ulong, p_device_ddr_heap2.size());
            break;

        case CL_DEVICE_GLOBAL_EXT2_MEM_SIZE_TI:
            SIMPLE_ASSIGN(cl_ulong, p_device_ddr_heap3.size());
            break;

        case CL_DEVICE_MSMC_MEM_SIZE_TI:
            SIMPLE_ASSIGN(cl_ulong, p_device_msmc_heap.size());
            break;

        case CL_DEVICE_GLOBAL_MEM_MAX_ALLOC_TI:
            p_device_ddr_heap1.max_block_size(maxblock, dummy);
            SIMPLE_ASSIGN(cl_ulong, maxblock);
            break;

        case CL_DEVICE_GLOBAL_EXT1_MEM_MAX_ALLOC_TI:
            p_device_ddr_heap2.max_block_size(maxblock, dummy);
            SIMPLE_ASSIGN(cl_ulong, maxblock);
            break;

        case CL_DEVICE_GLOBAL_EXT2_MEM_MAX_ALLOC_TI:
            p_device_ddr_heap3.max_block_size(maxblock, dummy);
            SIMPLE_ASSIGN(cl_ulong, maxblock);
            break;

        case CL_DEVICE_MSMC_MEM_MAX_ALLOC_TI:
            p_device_msmc_heap.max_block_size(maxblock, dummy);
            SIMPLE_ASSIGN(cl_ulong, maxblock);
            break;

        case CL_DEVICE_LOCAL_MEM_MAX_ALLOC_TI:
            SIMPLE_ASSIGN(cl_ulong, p_size_local_mem);
            break;

        case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
            SIMPLE_ASSIGN(cl_ulong, 1<<20);
            break;

        case CL_DEVICE_MAX_CONSTANT_ARGS:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_LOCAL_MEM_TYPE:
            SIMPLE_ASSIGN(cl_device_local_mem_type, CL_LOCAL);
            break;

        case CL_DEVICE_LOCAL_MEM_SIZE:
            {
            SIMPLE_ASSIGN(cl_ulong, p_size_local_mem);
            break;
            }

        case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_HOST_UNIFIED_MEMORY:
#ifdef DSPC868X
            SIMPLE_ASSIGN(cl_bool, CL_FALSE);
#else
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
#endif
            break;

        case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
            SIMPLE_ASSIGN(size_t, 1000);   // 1000 nanoseconds = 1 microsecond
            break;

        case CL_DEVICE_ENDIAN_LITTLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_COMPILER_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_EXECUTION_CAPABILITIES:
            SIMPLE_ASSIGN(cl_device_exec_capabilities, CL_EXEC_KERNEL);
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            SIMPLE_ASSIGN(cl_command_queue_properties,
                          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                          CL_QUEUE_PROFILING_ENABLE);
            break;

        case CL_DEVICE_NAME:
            STRING_ASSIGN("TI Multicore C66 DSP");
            break;

        case CL_DEVICE_VENDOR:
            STRING_ASSIGN("Texas Instruments, Inc.");
            break;

        case CL_DRIVER_VERSION:
            STRING_ASSIGN("" COAL_VERSION);
            break;

        case CL_DEVICE_PROFILE:
            STRING_ASSIGN("FULL_PROFILE");
            break;

        case CL_DEVICE_VERSION:
            STRING_ASSIGN("OpenCL 1.1 TI " COAL_VERSION);
            break;

        case CL_DEVICE_EXTENSIONS:
            if (getenv("TI_OCL_ENABLE_FP64"))
                STRING_ASSIGN("cl_khr_byte_addressable_store"
                              " cl_khr_global_int32_base_atomics"
                              " cl_khr_global_int32_extended_atomics"
                              " cl_khr_local_int32_base_atomics"
                              " cl_khr_local_int32_extended_atomics"
                              " cl_khr_fp64"
                              " cl_ti_msmc_buffers"
                              " cl_ti_clmalloc")
            else
                STRING_ASSIGN("cl_khr_byte_addressable_store"
                              " cl_khr_global_int32_base_atomics"
                              " cl_khr_global_int32_extended_atomics"
                              " cl_khr_local_int32_base_atomics"
                              " cl_khr_local_int32_extended_atomics"
                              " cl_ti_msmc_buffers"
                              " cl_ti_clmalloc")
            break;

        case CL_DEVICE_PLATFORM:
            SIMPLE_ASSIGN(cl_platform_id, &the_platform::Instance());
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
            SIMPLE_ASSIGN(cl_uint, 1);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_OPENCL_C_VERSION:
            STRING_ASSIGN("OpenCL C 1.1 LLVM " LLVM_VERSION);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/******************************************************************************
* Call back functions from the target loader
******************************************************************************/
extern "C"
{

static bool load_kernels_onchip()
{
   return (getenv("TI_OCL_LOAD_KERNELS_ONCHIP") != NULL);
}

/*****************************************************************************/
/* DLIF_ALLOCATE() - Return the load address of the segment/section          */
/*      described in its parameters and record the run address in            */
/*      run_address field of DLOAD_MEMORY_REQUEST.                           */
/*****************************************************************************/
bool DSPDevice::addr_is_l2  (DSPDevicePtr addr)const { return (addr >> 20 == 0x008); }
bool DSPDevice::addr_is_msmc(DSPDevicePtr addr)const { return (addr >> 20 == 0x0C0); }

BOOL DLIF_allocate(void* client_handle, struct DLOAD_MEMORY_REQUEST *targ_req)
{
   DSPDevice* device = (DSPDevice*) client_handle;

   /*------------------------------------------------------------------------*/
   /* Get pointers to API segment and file descriptors.                      */
   /*------------------------------------------------------------------------*/
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;

   uint32_t addr;

   if (device->addr_is_l2(obj_desc->target_address))
        addr = obj_desc->target_address; // do not allocate L2
   else if (device->addr_is_msmc(obj_desc->target_address) || load_kernels_onchip())
        addr = (uint32_t)device->malloc_msmc  (obj_desc->memsz_in_bytes);
   else addr = (uint32_t)device->malloc_global(obj_desc->memsz_in_bytes);

#if DEBUG
   printf("DLIF_allocate: %d bytes starting at 0x%x (relocated from 0x%x)\n",
                      obj_desc->memsz_in_bytes, (uint32_t)addr, 
                      (uint32_t)obj_desc->target_address);
#endif

   obj_desc->target_address = (TARGET_ADDRESS) addr;

   /*------------------------------------------------------------------------*/
   /* Target memory request was successful.                                  */
   /*------------------------------------------------------------------------*/
   return addr == 0 ? 0 : 1;
}

/*****************************************************************************/
/* DLIF_RELEASE() - Unmap or free target memory that was previously          */
/*      allocated by DLIF_allocate().                                        */
/*****************************************************************************/
BOOL DLIF_release(void* client_handle, struct DLOAD_MEMORY_SEGMENT* ptr)
{
   DSPDevice* device = (DSPDevice*) client_handle;

   if (device->addr_is_l2(ptr->target_address))
        ; // local was not allocated
   else if (device->addr_is_msmc(ptr->target_address) || load_kernels_onchip())
        device->free_msmc  ((DSPDevicePtr)ptr->target_address);
   else device->free_global((DSPDevicePtr)ptr->target_address);

#if DEBUG
   printf("DLIF_free: %d bytes starting at 0x%x\n",
                      ptr->memsz_in_bytes, (uint32_t)ptr->target_address);
#endif

   return 1;
}

/*****************************************************************************/
/* DLIF_WRITE() - Write updated (relocated) segment contents to target       */
/*      memory.                                                              */
/*****************************************************************************/
BOOL DLIF_write(void* client_handle, struct DLOAD_MEMORY_REQUEST* req)
{
   struct DLOAD_MEMORY_SEGMENT* obj_desc = req->segment;
   DSPDevice* device = (DSPDevice*) client_handle;
   int dsp_id = device->dspID();

   if (device->addr_is_l2(obj_desc->target_address))
   {
       if (req->host_address)
           printf("Warning: Initialized data for objects in .mem_l2 sections will be ignored.\n");
   }
   else if (req->host_address)
       Driver::instance()->write (dsp_id,
                             (uint32_t)obj_desc->target_address,
                             (uint8_t*)req->host_address,
                             obj_desc->memsz_in_bytes);

#if DEBUG
    printf("DLIF_write (dsp:%d): %d bytes starting at 0x%x\n",
               dsp_id, obj_desc->memsz_in_bytes, 
               (uint32_t)obj_desc->target_address);
#endif
    
    extern DSPProgram::segment_list *segments;

    if (segments) segments->push_back
        (DSPProgram::seg_desc((DSPDevicePtr)obj_desc->target_address, obj_desc->memsz_in_bytes, req->flags));

    return 1;
}

/*****************************************************************************/
/* DLIF_COPY() - Copy data from file to host-accessible memory.              */
/*      Returns a host pointer to the data in the host_address field of the  */
/*      DLOAD_MEMORY_REQUEST object.                                         */
/*****************************************************************************/
BOOL DLIF_copy(void* client_handle, struct DLOAD_MEMORY_REQUEST* targ_req)
{
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;
   DSPDevice* device = (DSPDevice*) client_handle;
   LOADER_FILE_DESC* f = targ_req->fp;
   void *buf = NULL;

   int result = 1;
   if (obj_desc->objsz_in_bytes)
   {
       buf = calloc(obj_desc->memsz_in_bytes, 1); 
       fseek(f, targ_req->offset, SEEK_SET);
       result = fread(buf, obj_desc->objsz_in_bytes, 1, f);
   }
   else  if (!device->addr_is_l2(obj_desc->target_address))
       buf = calloc(obj_desc->memsz_in_bytes, 1); 

   assert(result == 1);

   targ_req->host_address = buf;

   return 1;
}


/******************************************************************************
* DLIF_LOAD_DEPENDENT() 
******************************************************************************/
int DLIF_load_dependent(void* client_handle, const char* so_name)
{
   DSPDevice* device = (DSPDevice*) client_handle;
   FILE* fp = fopen(so_name, "rb");
   
   if (!fp)
   {
      DLIF_error(DLET_FILE, "Can't open dependent file '%s'.\n", so_name);
      return 0;
   }

   int to_ret = DLOAD_load(device->dload_handle(), fp);

   if (!to_ret)  
       DLIF_error(DLET_MISC, "Failed load of dependent file '%s'.\n", so_name);

   fclose(fp);
   return to_ret;
}

/******************************************************************************
* DLIF_UNLOAD_DEPENDENT() 
******************************************************************************/
void DLIF_unload_dependent(void* client_handle, uint32_t file_handle)
{
   DSPDevice* device = (DSPDevice*) client_handle;
   DLOAD_unload(device->dload_handle(), file_handle);
}

}

void dump_hex(char *addr, int bytes)
{
    int cnt = 0;

    printf("\n");
    while (cnt < bytes)
    {
        for (int col = 0; col < 16; ++col)
        {
            printf("%02x ", addr[cnt++] & 0xff);
            if (cnt >= bytes) break;
        }
        printf("\n");
    }
}



