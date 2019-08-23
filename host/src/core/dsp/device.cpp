/******************************************************************************
 * Copyright (c) 2013-2018, Texas Instruments Incorporated - http://www.ti.com/
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
#include "rootdevice.h"
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

#if defined(DEVICE_K2X)
extern "C"
{
    #include <ti/runtime/mmap/include/mmap_resource.h> // For MPAX
}
#endif

extern "C" {
#if defined(_SYS_BIOS)
    extern uint32_t ti_opencl_get_OCL_LOCAL_base();
    extern uint32_t ti_opencl_get_OCL_LOCAL_len();
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
* osal_getpid:  get a process ID.  Under BIOS this is a don't care value
*----------------------------------------------------------------------------*/
uint32_t osal_getpid()
{
#ifdef _SYS_BIOS
    return 0;
#else
    return getpid();
#endif
}

#define PROFILING_AETDATA_DIR   "profiling"
#define PROFILING_AETDATA_FILE  "profiling/aetdata.txt"

/******************************************************************************
* DSPDevice::DSPDevice(SharedMemory* shm)
******************************************************************************/
DSPDevice::DSPDevice(DeviceInterface::Type type, SharedMemory* shm)
    : DeviceInterface        (type),
      p_dsp_mhz              (0),
      p_shmHandler           (shm),
      p_pid                  (osal_getpid()),
      p_mpax_default_res     (NULL),
      p_profiling_out        (nullptr),
      p_parent               (nullptr),
      p_partitions_supported (),
      p_partition_type       ()
{
    /*-------------------------------------------------------------------------
    * Setup profiling
    *------------------------------------------------------------------------*/
    EnvVar& env                       = EnvVar::Instance();
    p_profiling.event_type            = env.GetEnv <
                                      EnvVar::Var::TI_OCL_PROFILING_EVENT_TYPE > (0);
    p_profiling.event_number1         = env.GetEnv <
                                      EnvVar::Var::TI_OCL_PROFILING_EVENT_NUMBER1 > (-1);
    p_profiling.event_number2         = env.GetEnv <
                                      EnvVar::Var::TI_OCL_PROFILING_EVENT_NUMBER2 > (-1);
    p_profiling.stall_cycle_threshold = env.GetEnv <
                                      EnvVar::Var::TI_OCL_PROFILING_STALL_CYCLE_THRESHOLD > (1);
    if (isProfilingEnabled())
    {
#if defined(_SYS_BIOS)
        p_profiling_out = &std::cout;
#else
        system("mkdir -p "PROFILING_AETDATA_DIR);
        std::ofstream* tmp = new std::ofstream;
        tmp->open(PROFILING_AETDATA_FILE, std::ios_base::app);
        p_profiling_out = tmp;
#endif
    }

    /*-------------------------------------------------------------------------
    * Get local mem size and address
    *------------------------------------------------------------------------*/
    const DeviceInfo& device_info = DeviceInfo::Instance();
#if !defined(_SYS_BIOS)
    p_addr_local_mem     = device_info.GetSymbolAddress("ocl_local_mem_start");
    p_size_local_mem     = device_info.GetSymbolAddress("ocl_local_mem_size");
#else
    p_addr_local_mem     = ti_opencl_get_OCL_LOCAL_base();
    p_size_local_mem     = ti_opencl_get_OCL_LOCAL_len();
#endif
}

/******************************************************************************
* DSPDevice::~DSPDevice()
******************************************************************************/
DSPDevice::~DSPDevice()
{
#if !defined(_SYS_BIOS)
    delete p_profiling_out;
#endif
}

/******************************************************************************
* DeviceBuffer *DSPDevice::createDeviceBuffer(MemObject *buffer)
******************************************************************************/
DeviceBuffer *DSPDevice::createDeviceBuffer(MemObject *buffer, cl_int *rs)
    { return (DeviceBuffer *)new DSPBuffer(p_shmHandler, buffer, rs); }

/******************************************************************************
* DeviceProgram *DSPDevice::createDeviceProgram(Program *program)
******************************************************************************/
DeviceProgram* DSPDevice::createDeviceProgram(Program* program)
{ return (DeviceProgram*)new DSPProgram(this, program); }

/******************************************************************************
* DeviceKernel *DSPDevice::createDeviceKernel(Kernel *kernel,
******************************************************************************/
DeviceKernel* DSPDevice::createDeviceKernel(Kernel* kernel,
        llvm::Function* function)
{ return (DeviceKernel*)new DSPKernel(this, kernel, function); }

DeviceKernel* DSPDevice::createDeviceBuiltInKernel(Kernel *kernel,
                                                   KernelEntry *kernel_entry)
{ return (DeviceKernel *)new DSPKernel(this, kernel, kernel_entry); }

/******************************************************************************
* cl_int DSPDevice::initEventDeviceData(Event *event)
******************************************************************************/
cl_int DSPDevice::initEventDeviceData(Event* event)
{
    cl_int ret_code = CL_SUCCESS;
    switch (event->type())
    {
        case Event::MapBuffer:
            {
                MapBufferEvent* e = (MapBufferEvent*) event;
                if (e->buffer()->flags() & CL_MEM_USE_HOST_PTR)
                {
                    e->setPtr((char*)e->buffer()->host_ptr() + e->offset());
                    break;
                }
                DSPBuffer*      buf  = (DSPBuffer*) e->buffer()->deviceBuffer(this);
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
                KernelEvent* e    = (KernelEvent*)event;
                Program*     p    = (Program*)e->kernel()->parent();
                DSPProgram*  prog = (DSPProgram*)p->deviceDependentProgram(this);
                /*-----------------------------------------------------------------
                * Just in time loading
                *----------------------------------------------------------------*/
                if (!prog->is_loaded() && !prog->load())
                    return CL_MEM_OBJECT_ALLOCATION_FAILURE;
                DSPKernel* dspkernel = (DSPKernel*)e->deviceKernel();
                cl_int ret = dspkernel->preAllocBuffers();
                if (ret != CL_SUCCESS) return ret;
                // ASW TODO do something
                // Set device-specific data
                DSPKernelEvent* dsp_e = new DSPKernelEvent(this, e);
                ret_code = dsp_e->get_ret_code();
                e->setDeviceData((void*)dsp_e);
                break;
            }
        default: break;
    }
    return ret_code;
}

/******************************************************************************
* void DSPDevice::freeEventDeviceData(Event *event)
******************************************************************************/
void DSPDevice::freeEventDeviceData(Event* event)
{
    switch (event->type())
    {
        case Event::NDRangeKernel:
        case Event::TaskKernel:
            {
                DSPKernelEvent* dsp_e = (DSPKernelEvent*)event->deviceData();
                if (dsp_e) delete dsp_e;
            }
        default: break;
    }
}

/******************************************************************************
* Getter functions
******************************************************************************/
unsigned int  DSPDevice::dspCores()     const { return p_compute_units.size(); }

DSPDevicePtr DSPDevice::get_L2_extent(uint32_t& size)
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

bool DSPDevice::isInClMallocedRegion(void* ptr)
{
    return GetSHMHandler()->clMallocQuery(ptr, NULL, NULL);
}

int DSPDevice::numHostMails(Msg_t& msg) const
{
    if ((msg.command == EXIT || msg.command == CACHEINV ||
         msg.command == SETUP_DEBUG || msg.command == CONFIGURE_MONITOR ||
         (msg.command == NDRKERNEL && !IS_DEBUG_MODE(msg))))
        return dspCores();
    return 1;
}

void DSPDevice::recordProfilingData(command_retcode_t* profiling_data,
                                    uint32_t core)
{
    if (! isProfilingEnabled()) return;
    (*p_profiling_out) << (int)  p_profiling.event_type << '\n'
                       << (int)  p_profiling.event_number1 << '\n'
                       << (int)  p_profiling.event_number2 << '\n'
                       << (uint32_t) p_profiling.stall_cycle_threshold << '\n'
                       << (uint32_t) core << '\n';
    // Output counter diffs corresponding to event numbers (use -1 if failed)
    if (profiling_data->profiling_status != 0)
    {
        std::cout << "Profiling Failed on core " << core << " : "
                  << (int) profiling_data->profiling_status << std::endl;
        (*p_profiling_out) << (int) - 1 << '\n'
                           << (int) - 1 << '\n';
    }
    else
    {
        std::cout << "Profiling Successful on core " << core << std::endl;
        (*p_profiling_out) << (uint32_t) profiling_data->profiling_counter0_val
                           << '\n'
                           << (uint32_t) profiling_data->profiling_counter1_val
                           << '\n';
    }
    // mark end of core's data
    (*p_profiling_out) << "~~~~End Core" <<  '\n';
    return;
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
        // Stay away from k2g dsp_common_mpm_pool: 0x9D000000 to 0x9F800000
        // Stay away from k2x dsp_common_cma_pool: 0x9F800000 to 0xA0000000
        uint32_t heap_base[NUM_VIRT_HEAPS]  = {0xC0000000, 0x80000000};
        uint32_t heap_size[NUM_VIRT_HEAPS]  = {0x40000000, 0x18000000};
        for (int i = 0; i < MAX_XMCSES_MPAXS; i++)
        {
            xmc_regs[i] = FIRST_FREE_XMC_MPAX + i;
            ses_regs[i] = FIRST_FREE_SES_MPAX + i;
        }
        keystone_mmap_resource_init(MAX_XMCSES_MPAXS, xmc_regs, ses_regs,
                                    NUM_VIRT_HEAPS, heap_base, heap_size,
                                    (keystone_mmap_resources_t*) p_mpax_default_res);
    }
    return p_mpax_default_res;
}
#endif  // #ifdef DEVICE_K2X

/******************************************************************************
* cl_int DSPDevice::info
******************************************************************************/
cl_int DSPDevice::info(cl_device_info param_name,
                       size_t param_value_size,
                       void* param_value,
                       size_t* param_value_size_ret) const
{
    void* value = 0;
    size_t value_length = 0;
    std::string stmp;

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
        cl_device_id cl_device_id_var;
        cl_device_affinity_domain cl_device_affinity_domain_var;
        size_t work_dims[MAX_WORK_DIMS];
    };
    switch (param_name)
    {
        case CL_DEVICE_TYPE:
            {
                cl_device_type device_type = CL_DEVICE_TYPE_ACCELERATOR;
                if (GetRootDevice()->getKernelEntries()->size() > 0)
                    device_type |= CL_DEVICE_TYPE_CUSTOM;
                SIMPLE_ASSIGN(cl_device_type, device_type);
                break;
            }
        case CL_DEVICE_VENDOR_ID:
            SIMPLE_ASSIGN(cl_uint, 0); // TODO
            break;
        case CL_DEVICE_MAX_COMPUTE_UNITS:
            SIMPLE_ASSIGN(cl_uint, dspCores());
            break;
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
            SIMPLE_ASSIGN(cl_uint, MAX_WORK_DIMS);
            break;
        case CL_DEVICE_MAX_WORK_GROUP_SIZE:
            {
                size_t wgsize = 0x40000000;
                int env_wgsize = EnvVar::Instance().GetEnv <
                                 EnvVar::Var::TI_OCL_WG_SIZE_LIMIT > (0);
                if (env_wgsize > 0 && env_wgsize < wgsize)  wgsize = env_wgsize;
                SIMPLE_ASSIGN(size_t, wgsize);
                break;
            }
        case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            for (int i = 0; i < MAX_WORK_DIMS; ++i) work_dims[i] = 0x40000000;
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
                cl_ulong cap = EnvVar::Instance().GetEnv <
                               EnvVar::Var::TI_OCL_LIMIT_DEVICE_MAX_MEM_ALLOC_SIZE > (
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
                                      std::min(heap1_size - (1 << 24), cap))
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
        case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;
        case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
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
                          CL_FP_ROUND_TO_INF | CL_FP_INF_NAN);
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
            SIMPLE_ASSIGN(cl_ulong, 128 * 1024);
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
            SIMPLE_ASSIGN(cl_ulong, 1 << 20);
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
        case CL_DEVICE_LINKER_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;
        case CL_DEVICE_EXECUTION_CAPABILITIES:
            SIMPLE_ASSIGN(cl_device_exec_capabilities, CL_EXEC_KERNEL);
            break;
        case CL_DEVICE_QUEUE_PROPERTIES:
            SIMPLE_ASSIGN(cl_command_queue_properties,
                          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                          CL_QUEUE_PROFILING_ENABLE |
                          CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI);
            break;
        case CL_DEVICE_BUILT_IN_KERNELS:
            {
                const std::vector<KernelEntry*>* biks =
                                        GetRootDevice()->getKernelEntries();
                bool first = true;
                for(KernelEntry *k : *biks)
                {
                    if (!first) stmp += ";";
                    stmp += k->name;
                    first = false;
                }
                value        = (void *) stmp.c_str();
                value_length =          stmp.size() + 1;
            }
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
            STRING_ASSIGN("OpenCL 1.1 ");
            break;
        case CL_DEVICE_EXTENSIONS:
            if (EnvVar::Instance().GetEnv <
                EnvVar::Var::TI_OCL_ENABLE_FP64 > (nullptr))
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
        case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;
        case CL_DEVICE_PRINTF_BUFFER_SIZE:
            /* The format conversion buffer size is defined to be 510 bytes in
             * the RTS printf implementation. For each format specifier in the
             * printf call, this buffer is reused.
             * 1MB (1048576 bytes) is the minimum value for FULL profile as
             * stated in the OpenCL 1.2 specification.
             * */
            SIMPLE_ASSIGN(size_t, 1048576);
            break;
        case CL_DEVICE_OPENCL_C_VERSION:
            STRING_ASSIGN("OpenCL C 1.1 ");
            break;
        case CL_DEVICE_PARENT_DEVICE:
            if (p_parent) {SIMPLE_ASSIGN(cl_device_id, desc(p_parent));}
            else          {SIMPLE_ASSIGN(cl_device_id, NULL);          }
            break;
        case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
            if (p_compute_units.size() == 1) {SIMPLE_ASSIGN(cl_uint, 0);                     }
            else                             {SIMPLE_ASSIGN(cl_uint, p_compute_units.size());}
            break;
        case CL_DEVICE_PARTITION_PROPERTIES:
            if (p_partitions_supported[0] != 0){MEM_ASSIGN(sizeof(p_partitions_supported), &p_partitions_supported);}
            else                               {MEM_ASSIGN(0, 0);                                                   }
            break;
        case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
            SIMPLE_ASSIGN(cl_device_affinity_domain, 0); // Not supported
            break;
        case CL_DEVICE_PARTITION_TYPE:
        /* If this device is a sub device, it has partition properties */
        if (p_partition_type[0] != 0)
        {
            /* Calculate the size of the p_partition_type array
             * Starting num_elements=2 because +1 for End Marker, +1 for End 0 */
            int size_of_partition_type = 0, it = 0, num_elements = 2;
            while (p_partition_type[it++] != 0) ++num_elements;
            size_of_partition_type = sizeof(cl_device_partition_property) * num_elements;
            MEM_ASSIGN(size_of_partition_type, &p_partition_type);
        }
            else {MEM_ASSIGN(0, 0);}
            break;
        case CL_DEVICE_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
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

bool DSPDevice::addr_is_l2(DSPDevicePtr addr) const
{
    return (addr >= p_addr_local_mem &&
            addr < (p_addr_local_mem + p_size_local_mem));
}

void dump_hex(char* addr, int bytes)
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



