/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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
#include "../error_report.h"
#include "../oclenv.h"

#include "device_info.h"


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
void *eve_worker_event_dispatch  (void* data);
void *eve_worker_event_completion(void* data);


/******************************************************************************
* EVEDevice::EVEDevice(unsigned char eve_id, SharedMemory *shm)
******************************************************************************/
EVEDevice::EVEDevice(unsigned char eve_id, SharedMemory* shm)
    : DeviceInterface       (),
      p_eve_id_             (eve_id),
      p_cores               (1),
      p_num_events          (0),
      p_eve_mhz_            (650),
      p_worker_dispatch     (0),
      p_worker_completion   (0),
      p_stop                (false),
      p_exit_acked          (false),
      p_initialized         (false),
      p_complete_pending    (),
      p_shmHandler          (shm),
      device_manager_       (nullptr),
      p_pid                 (getpid())
{
    /*-------------------------------------------------------------------------
    * YUAN TODO: get EVE device built in kernels information
    *------------------------------------------------------------------------*/
    // const DeviceInfo& device_info = DeviceInfo::Instance();
    // p_builtInKernels = device_info.GetBuiltInKernels();

    /*-------------------------------------------------------------------------
    * initialize the mailboxes on the cores, so they can receive an exit cmd
    *------------------------------------------------------------------------*/
    p_mb = MBoxFactory::CreateMailbox(this);
    
    /*-------------------------------------------------------------------------
    * Initialize BuiltIn Kernels
    *------------------------------------------------------------------------*/
    KernelEntry *k;

    k = new KernelEntry("tiocl_bik_memcpy_test", 0);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Buffer, false);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Buffer, false);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Int32, false);
    p_kernel_entries.push_back(k);
    
    k = new KernelEntry("tiocl_bik_vecadd", 1);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Buffer, false);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Buffer, false);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Buffer, false);
    k->addArg(1, Kernel::Arg::Global, Kernel::Arg::Int32, false);
    p_kernel_entries.push_back(k);
}

bool EVEDevice::hostSchedule() const
{
    return true;
}


/******************************************************************************
* void EVEDevice::init()
******************************************************************************/
void EVEDevice::init()
{
    if (p_initialized) return;
    /*-------------------------------------------------------------------------
    * Initialize the locking machinery and create worker threads
    *------------------------------------------------------------------------*/
    pthread_cond_init(&p_events_cond, 0);
    pthread_mutex_init(&p_events_mutex, 0);
    pthread_cond_init(&p_worker_cond, 0);
    pthread_mutex_init(&p_worker_mutex, 0);

    pthread_create(&p_worker_dispatch,   0, &eve_worker_event_dispatch,   this);
    pthread_create(&p_worker_completion, 0, &eve_worker_event_completion, this);

    p_initialized = true;
}

/******************************************************************************
* EVEDevice::~EVEDevice()
******************************************************************************/
EVEDevice::~EVEDevice()
{

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

    /*-------------------------------------------------------------------------
    * Inform the cores on the device to stop listening for commands
    * Send exit message after worker threads terminate to avoid a race condition
    * on p_exit_acked. The race condition is caused by a worker thread receiving
    * the exit response and setting p_exit_acked to true. It's possible that the
    * master thread can read p_exit_acked before write from the worker thread
    * lands and enter the while loop. It then is stuck at the mail_query() while
    * loop because the worker has already read the message.
    *
    * Sending the exit message after the worker threads terminate eliminates
    * the race condition.
    *------------------------------------------------------------------------*/
    exitMsg.u.k_eve.eve_id = GetEveId();
    mail_to(exitMsg);

    /*-------------------------------------------------------------------------
    * Wait for the EXIT acknowledgement from device
    * YUAN TODO: may skip this for EVE
    *------------------------------------------------------------------------*/
#if 1
    while (! p_exit_acked)
    {
        while (! mail_query()) usleep(1);
        mail_from();
    }
#endif

    delete p_mb;
    p_mb = NULL;

    /*-------------------------------------------------------------------------
    * Only need to close the driver for one of the devices
    *------------------------------------------------------------------------*/
    delete device_manager_;

    /*-------------------------------------------------------------------------
    * Remove BuiltIn kernel entries
    *------------------------------------------------------------------------*/
    for(KernelEntry *k : p_kernel_entries) delete k;

}

/******************************************************************************
* DeviceBuffer *EVEDevice::createDeviceBuffer(MemObject *buffer)
******************************************************************************/
DeviceBuffer *EVEDevice::createDeviceBuffer(MemObject *buffer, cl_int *rs)
{ 
    return (DeviceBuffer *)new DSPBuffer(p_shmHandler, buffer, rs); 
}

/******************************************************************************
* DeviceProgram *EVEDevice::createDeviceProgram(Program *program)
******************************************************************************/
DeviceProgram *EVEDevice::createDeviceProgram(Program *program)
    { return (DeviceProgram *)new EVEProgram(this, program); }

/******************************************************************************
* DeviceKernel *EVEDevice::createDeviceKernel(Kernel *kernel,
*                                             llvm::Function *function)      
******************************************************************************/
DeviceKernel *EVEDevice::createDeviceKernel(Kernel *kernel,
                                llvm::Function *function)
    { return (DeviceKernel *)new EVEKernel(this, kernel, function); }

/******************************************************************************
* DeviceKernel *EVEDevice::createDeviceBuiltInKernel(Kernel *kernel,
*                                                    KernelEntry *kernel_entry)
******************************************************************************/
DeviceKernel *EVEDevice::createDeviceBuiltInKernel(Kernel *kernel,
                                                   KernelEntry *kernel_entry)
    { return (DeviceKernel *)new EVEKernel(this, kernel, kernel_entry); }

/******************************************************************************
* cl_int EVEDevice::initEventDeviceData(Event *event)
******************************************************************************/
cl_int EVEDevice::initEventDeviceData(Event *event)
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
            KernelEvent *e          = (KernelEvent *)event;
            EVEKernel   *eve_kernel = (EVEKernel*)e->deviceKernel();

            cl_int ret = eve_kernel->preAllocBuffers();
            if (ret != CL_SUCCESS) return ret;

            // Set device-specific data
            EVEKernelEvent *eve_e = new EVEKernelEvent(this, e);
            ret_code = eve_e->get_ret_code();
            e->setDeviceData((void *)eve_e);
            break;
        }
        default: break;
    }

    return ret_code;
}

/******************************************************************************
* void EVEDevice::freeEventDeviceData(Event *event)
******************************************************************************/
void EVEDevice::freeEventDeviceData(Event *event)
{
    switch (event->type())
    {
        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
            EVEKernelEvent *dsp_e = (EVEKernelEvent *)event->deviceData();
            if (dsp_e) delete dsp_e;
        }
        default: break;
    }
}

/******************************************************************************
* void EVEDevice::pushEvent(Event *event)
******************************************************************************/
void EVEDevice::pushEvent(Event *event)
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

bool EVEDevice::stop()           { return p_stop; }
bool EVEDevice::availableEvent() { return p_num_events > 0; }

/******************************************************************************
* Event *EVEDevice::getEvent(bool &stop)
******************************************************************************/
Event *EVEDevice::getEvent(bool &stop)
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

void EVEDevice::push_complete_pending(uint32_t idx, Event* const data,
                                      unsigned int cnt)
    { p_complete_pending.push(idx, data, cnt); }

bool EVEDevice::get_complete_pending(uint32_t idx, Event*& data)
    { return p_complete_pending.try_pop(idx, data); }

int  EVEDevice::num_complete_pending()
    { return p_complete_pending.size(); }

void EVEDevice::dump_complete_pending() { p_complete_pending.dump(); }

bool EVEDevice::any_complete_pending() { return !p_complete_pending.empty(); }

/******************************************************************************
* Device's decision about whether CommandQueue should push more events over
* This number could be tuned (e.g. using ooo example).  Note that p_num_events
* are in device's queue, but not yet executed.
******************************************************************************/
bool EVEDevice::gotEnoughToWorkOn() { return p_num_events > 0; }

/******************************************************************************
* Getter functions
******************************************************************************/

MemoryRange::Location EVEDevice::ClFlagToLocation(cl_mem_flags flags) const
{
    return ((flags & CL_MEM_USE_MSMC_TI) != 0) ?
                                    MemoryRange::Location::ONCHIP :
                                    MemoryRange::Location::OFFCHIP;
}

bool EVEDevice::isInClMallocedRegion(void *ptr)
{
    return GetSHMHandler()->clMallocQuery(ptr, NULL, NULL);
}

int EVEDevice::numHostMails(Msg_t &msg) const
{
    return 1;
}

void EVEDevice::mail_to(Msg_t &msg, unsigned int core)
{
    msg.pid = p_pid;

    ReportTrace("Sending message %d, pid %d\n", msg.command, msg.pid);

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
            case CONFIGURE_MONITOR:
            case TASK:
            {
                for (int i = 0; i < numHostMails(msg); i++)
                    p_mb->to((uint8_t*)&msg, sizeof(Msg_t), i);

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

bool EVEDevice::mail_query()
{
    return p_mb->query();
}

int EVEDevice::mail_from(int *retcode)
{
    uint32_t size_rx;
    int32_t  trans_id_rx;
    Msg_t    rxmsg;
    uint8_t  core;

    trans_id_rx = p_mb->from((uint8_t*)&rxmsg, &size_rx, &core);

    ReportTrace("Received message %d, for pid %d\n", rxmsg.command, rxmsg.pid);

    if (rxmsg.command == ERROR)
    {
        std::cout << rxmsg.u.message;
        return -1;
    }

    if (rxmsg.command == PRINT)
    {
        std::cout << "[eve " << rxmsg.u.message[0] << "] "
                             << rxmsg.u.message+1;
        return -1;
    }

    if (rxmsg.command == EXIT)
    {
        p_exit_acked = true;
        return -1;
    }

    if (retcode != nullptr)  *retcode = rxmsg.u.command_retcode.retcode;

    return trans_id_rx;
}


/******************************************************************************
* cl_int EVEDevice::info
******************************************************************************/
cl_int EVEDevice::info(cl_device_info param_name,
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
            SIMPLE_ASSIGN(cl_device_type, CL_DEVICE_TYPE_CUSTOM);
            break;

        case CL_DEVICE_VENDOR_ID:
            SIMPLE_ASSIGN(cl_uint, 0); // TODO
            break;

        case CL_DEVICE_MAX_COMPUTE_UNITS:
            SIMPLE_ASSIGN(cl_uint, p_cores);
            break;

        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
            SIMPLE_ASSIGN(cl_uint, MAX_WORK_DIMS);
            break;

        case CL_DEVICE_MAX_WORK_GROUP_SIZE: {
	    size_t wgsize = 0x40000000;
            int env_wgsize = EnvVar::Instance().GetEnv<
                                         EnvVar::Var::TI_OCL_WG_SIZE_LIMIT>(0);
	    if (env_wgsize > 0 && env_wgsize < wgsize)  wgsize = env_wgsize;
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
            SIMPLE_ASSIGN(cl_uint, p_eve_mhz_);
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
            cl_ulong cap = EnvVar::Instance().GetEnv<
                         EnvVar::Var::TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE>(
                                                          (cl_ulong)1ul << 30);
            if (cap == 0)
            {
                printf("ERROR: TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE "
                       "must be a positive integer\n");
                exit(1);
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
            SIMPLE_ASSIGN(cl_ulong, p_size_local_mem_);
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
            SIMPLE_ASSIGN(cl_ulong, p_size_local_mem_);
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
            SIMPLE_ASSIGN(cl_device_exec_capabilities, CL_EXEC_NATIVE_KERNEL);
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            SIMPLE_ASSIGN(cl_command_queue_properties,
                          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                          CL_QUEUE_PROFILING_ENABLE |
                          CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI);
            break;
        
        case CL_DEVICE_BUILT_IN_KERNELS:
            STRING_ASSIGN("tiocl_bik_memcpy_test;tiocl_bik_vecadd");
            break;

        case CL_DEVICE_NAME:
            STRING_ASSIGN("TI Embedded Vision Engin (EVE)");
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
            if (EnvVar::Instance().GetEnv<
                                     EnvVar::Var::TI_OCL_ENABLE_FP64>(nullptr))
                STRING_ASSIGN("cl_khr_byte_addressable_store"
                              " cl_khr_global_int32_base_atomics"
                              " cl_khr_global_int32_extended_atomics"
                              " cl_khr_local_int32_base_atomics"
                              " cl_khr_local_int32_extended_atomics"
                              " cl_khr_fp64"
                              " cl_ti_msmc_buffers"
                              " cl_ti_clmalloc"
                              " cl_ti_kernel_timeout_compute_unit")
            else
                STRING_ASSIGN("cl_khr_byte_addressable_store"
                              " cl_khr_global_int32_base_atomics"
                              " cl_khr_global_int32_extended_atomics"
                              " cl_khr_local_int32_base_atomics"
                              " cl_khr_local_int32_extended_atomics"
                              " cl_ti_msmc_buffers"
                              " cl_ti_clmalloc"
                              " cl_ti_kernel_timeout_compute_unit")
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

std::vector<KernelEntry*> *EVEDevice::getKernelEntries()
    { return &p_kernel_entries;}
