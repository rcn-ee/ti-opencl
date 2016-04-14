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

#include "device_info.h"

#if defined(DEVICE_K2X)
extern "C"
{
    #include <ti/runtime/mmap/include/mmap_resource.h> // For MPAX
    extern int get_ocl_qmss_res(Msg_t *msg);
    extern void free_ocl_qmss_res();
}
#endif

extern "C" {
/*-----------------------------------------------------------------------------
* Add ULM memory state messages if ULM library is available
*----------------------------------------------------------------------------*/
#if defined (ULM_ENABLED)
   #include "tiulm.h"
#endif
}

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Coal;
using namespace tiocl;


/*-----------------------------------------------------------------------------
* Declare our threaded dsp handler function
*----------------------------------------------------------------------------*/
void *dsp_worker_event_dispatch   (void* data);
void *dsp_worker_event_completion (void* data);
void HOSTwait   (unsigned char dsp_id);

/******************************************************************************
* DSPDevice::DSPDevice(unsigned char dsp_id)
******************************************************************************/
DSPDevice::DSPDevice(unsigned char dsp_id, SharedMemory* shm)
    : DeviceInterface       (),
      p_cores               (0),
      p_num_events          (0),
      p_dsp_mhz             (0),
      p_worker_dispatch     (0),
      p_worker_completion   (0),
      p_stop                (false),
      p_exit_acked          (false),
      p_initialized         (false),
      p_dsp_id              (dsp_id),
      p_complete_pending    (),
      p_mpax_default_res    (NULL),
      p_shmHandler          (shm),
      device_manager_       (nullptr)
{
    const DeviceInfo& device_info = DeviceInfo::Instance();

    p_cores = device_info.GetComputeUnitsPerDevice(dsp_id);

    device_manager_ = DeviceManagerFactory::CreateDeviceManager(dsp_id, p_cores,
                                                    device_info.FullyQualifiedPathToDspMonitor());

    device_manager_->Reset();
    device_manager_->Load();
    device_manager_->Run();

    p_addr_kernel_config = device_info.GetSymbolAddress("kernel_config_l2");
    p_addr_local_mem     = device_info.GetSymbolAddress("ocl_local_mem_start");
    p_size_local_mem     = device_info.GetSymbolAddress("ocl_local_mem_size");


    init_ulm();

    /*-------------------------------------------------------------------------
    * initialize the mailboxes on the cores, so they can receive an exit cmd
    *------------------------------------------------------------------------*/
    p_mb = MBoxFactory::CreateMailbox(this);

#if defined(DEVICE_K2X) && !defined(DEVICE_K2G)
    /*-------------------------------------------------------------------------
    * Send monitor configuration
    *------------------------------------------------------------------------*/
    Msg_t msg = {CONFIGURE_MONITOR};
    msg.u.configure_monitor.n_cores = dspCores();

    // Keystone2: get QMSS resources from RM, mail to DSP monitor
    if (get_ocl_qmss_res(&msg) == 0)
    {
        printf("Unable to allocate resource from RM server!\n");
        exit(-1);
    }

    mail_to(msg);
#endif

    /*-------------------------------------------------------------------------
    * Query DSP frequency; monitor is in message loop task after this point.
    *------------------------------------------------------------------------*/
    setup_dsp_mhz();
}

bool DSPDevice::hostSchedule() const
{
#if defined(DEVICE_K2G) || defined(DEVICE_AM57)
    return true;
#else
    return false;
#endif
}

void DSPDevice::setup_dsp_mhz(void)
{
#ifdef DSPC868X
    char *ghz1 = getenv("TI_OCL_DSP_1_25GHZ");
    if (ghz1) p_dsp_mhz = 1250;  // 1.25 GHz
#else
    mail_to(frequencyMsg);

    int ret = 0;
    do
    {
        while (!mail_query())  ;
        ret = mail_from();
    } while (ret == -1);

    p_dsp_mhz = ret;
#endif
}

/*-----------------------------------------------------------------------------
* If ULM library was not available don't emit ULM trace messages
*----------------------------------------------------------------------------*/
#if !defined (ULM_ENABLED)
#define ulm_put_mem(a,b,c)
#define ulm_config()
#define ulm_term()
#endif
void DSPDevice::init_ulm()
{
    ulm_config();

    uint64_t cmem_persistent_sz =
        GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_PERSISTENT,
                                  MemoryRange::Location::OFFCHIP);

    uint64_t cmem_ondemand_sz =
        GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_ONDEMAND,
                                  MemoryRange::Location::OFFCHIP);

    uint64_t msmc_mem_size =
        GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_PERSISTENT,
                                  MemoryRange::Location::ONCHIP);

    ulm_put_mem(ULM_MEM_IN_DATA_ONLY,     msmc_mem_size >> 16  , 1.0f);
    ulm_put_mem(ULM_MEM_EX_DATA_ONLY,     cmem_ondemand_sz >> 16, 1.0f);
    ulm_put_mem(ULM_MEM_EX_CODE_AND_DATA, cmem_persistent_sz >> 16, 1.0f);
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

#if defined(DEVICE_K2X) || defined(DEVICE_K2G) || defined(DSPC868X)
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
    delete device_manager_;

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
            void* host_addr = GetSHMHandler()->Map(data, e->cb(),
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
unsigned int  DSPDevice::dspCores()      const { return p_cores;   }
float         DSPDevice::dspMhz()       const { return p_dsp_mhz; }
unsigned char DSPDevice::dspID()        const { return p_dsp_id;  }



DSPDevicePtr DSPDevice::get_L2_extent(uint32_t &size)
{
    size       = (uint32_t) p_size_local_mem;
    return (DSPDevicePtr) p_addr_local_mem;
}


MemoryRange::Location DSPDevice::ClFlagToLocation(cl_mem_flags flags) const
{
    return ((flags & CL_MEM_USE_MSMC_TI) != 0) ?
                                    MemoryRange::Location::ONCHIP :
                                    MemoryRange::Location::OFFCHIP;
}

bool DSPDevice::isInClMallocedRegion(void *ptr)
{
    return GetSHMHandler()->clMallocQuery(ptr, NULL, NULL);
}

int DSPDevice::numHostMails(Msg_t &msg) const
{
    if (hostSchedule() && (msg.command == EXIT || msg.command == CACHEINV ||
                           msg.command == SETUP_DEBUG ||
                           (msg.command == NDRKERNEL && !IS_DEBUG_MODE(msg))))
        return dspCores();
    return 1;
}

void DSPDevice::mail_to(Msg_t &msg, unsigned int core)
{
    if(hostSchedule())
    {
        switch(msg.command)
        {
            /*-----------------------------------------------------------------
            * for hostScheduled platforms, broadcast the following messages
            * according to numHostMails()
            *----------------------------------------------------------------*/
            case EXIT:
            case CACHEINV:
            case NDRKERNEL:
            case SETUP_DEBUG:
            {
                for (int i = 0; i < numHostMails(msg); i++)
                    p_mb->to((uint8_t*)&msg, sizeof(Msg_t), i);

                break;
            }

            /*-----------------------------------------------------------------
            * for hostScheduled platforms, OoO TASKs are sent to cores in
            * round-robin order, in-order TASKs are broadcast to all cores
            *----------------------------------------------------------------*/
            case TASK:
            {
                if (IS_OOO_TASK(msg))
                {
                    static int counter = 0;
                    int dsp_id = counter++ % dspCores();
                    p_mb->to((uint8_t*)&msg, sizeof(Msg_t), dsp_id);
                }
                else
                {
                    /*---------------------------------------------------------
                    * note: for in-order TASKs this is a trick to enable the
                    * OpenMP runtime. OpenMP kernels run as in-order tasks.
                    *-------------------------------------------------------*/
                    for (int i = 0; i < dspCores(); i++)
                        p_mb->to((uint8_t*)&msg, sizeof(Msg_t), i);
                }

                break;
            }

            /*-----------------------------------------------------------------
            * otherwise send it to the designated core
            *---------------------------------------------------------------*/
            default:
            {
                p_mb->to((uint8_t*)&msg, sizeof(Msg_t), core);
                break;
            }
        }
    }
    else
    {
        /*---------------------------------------------------------------------
        * non-hostScheduled platforms always send directly to the given core
        *-------------------------------------------------------------------*/
        p_mb->to((uint8_t*)&msg, sizeof(Msg_t), core);
    }
}

bool DSPDevice::mail_query()
{
    return p_mb->query();
}

int DSPDevice::mail_from()
{
    uint32_t size_rx;
    int32_t  trans_id_rx;
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

            uint64_t heap1_size =
               GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_PERSISTENT,
                                         MemoryRange::Location::OFFCHIP);

            if (GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_ONDEMAND,
                                     MemoryRange::Location::OFFCHIP) > 0)
                SIMPLE_ASSIGN(cl_ulong,
                    std::min(heap1_size, cap))
            else
                SIMPLE_ASSIGN(cl_ulong,
                    std::min(heap1_size - (1<<24), cap))

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
            SIMPLE_ASSIGN(cl_ulong,
                          GetSHMHandler()->HeapSize(
                                            MemoryRange::Kind::CMEM_PERSISTENT,
                                            MemoryRange::Location::OFFCHIP));

            break;

        case CL_DEVICE_GLOBAL_EXT1_MEM_SIZE_TI:
            SIMPLE_ASSIGN(cl_ulong,
                          GetSHMHandler()->HeapSize(
                                            MemoryRange::Kind::CMEM_ONDEMAND,
                                            MemoryRange::Location::OFFCHIP));
            break;

        case CL_DEVICE_GLOBAL_EXT2_MEM_SIZE_TI:
            SIMPLE_ASSIGN(cl_ulong, 0);
            break;

        case CL_DEVICE_MSMC_MEM_SIZE_TI:
            SIMPLE_ASSIGN(cl_ulong,
                          GetSHMHandler()->HeapSize(
                                            MemoryRange::Kind::CMEM_PERSISTENT,
                                            MemoryRange::Location::ONCHIP));
            break;

        case CL_DEVICE_GLOBAL_MEM_MAX_ALLOC_TI:
            SIMPLE_ASSIGN(cl_ulong,
                          GetSHMHandler()->HeapMaxAllocSize(
                                            MemoryRange::Kind::CMEM_PERSISTENT,
                                            MemoryRange::Location::OFFCHIP));
            break;

        case CL_DEVICE_GLOBAL_EXT1_MEM_MAX_ALLOC_TI:
            SIMPLE_ASSIGN(cl_ulong,
                          GetSHMHandler()->HeapMaxAllocSize(
                                            MemoryRange::Kind::CMEM_ONDEMAND,
                                            MemoryRange::Location::OFFCHIP));
            break;

        case CL_DEVICE_GLOBAL_EXT2_MEM_MAX_ALLOC_TI:
            SIMPLE_ASSIGN(cl_ulong, 0);
            break;

        case CL_DEVICE_MSMC_MEM_MAX_ALLOC_TI:
            SIMPLE_ASSIGN(cl_ulong,
                          GetSHMHandler()->HeapMaxAllocSize(
                                            MemoryRange::Kind::CMEM_PERSISTENT,
                                            MemoryRange::Location::ONCHIP));
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

bool DSPDevice::addr_is_l2  (DSPDevicePtr addr)const { return (addr >> 20 == 0x008); }

// TODO: Fix msmc address for AM57
bool DSPDevice::addr_is_msmc(DSPDevicePtr addr)const { return (addr >> 24 == 0x0C); }

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



