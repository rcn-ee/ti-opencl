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
#ifndef __DSP_DEVICE_H__
#define __DSP_DEVICE_H__

#include "../deviceinterface.h"
#include "message.h"
#include "u_lockable.h"
#include "u_concurrent_map.h"
#include "kernel.h"
#include "../tiocl_thread.h"
#include <string>
#include <list>
#include "mbox_interface.h"
#include "../shared_memory_interface.h"
#include "device_manager_interface.h"
#include "core_scheduler.h"
#include "device_info.h"

namespace Coal
{

class MemObject;
class Event;
class Program;
class Kernel;

using tiocl::SharedMemory;
using tiocl::MemoryRange;
using tiocl::DeviceManager;

/* Maximum size of properties array for partition_type
 * For each partition unit -> MAX_NUM_CORES
 * Type, End Marker, End 0 -> 3
 * */
#define MAX_PARTITION_COUNT            (MAX_NUM_CORES+3)
/* CL_DEVICE_PARTITION_EQUALLY            -> Supported
 * CL_DEVICE_PARTITION_BY_COUNTS          -> Supported
 * CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN -> Not Supported
 * */
#define NUM_PARTITION_TYPES_SUPPORTED  2

class DSPDevice : public DeviceInterface, public Lockable
{
public:
    DSPDevice(SharedMemory* shm);
    virtual ~DSPDevice();

    /* Disable default constructor, copy constructor, and assignment */
    DSPDevice()                                         = delete;
    DSPDevice(const DSPDevice&)                         = delete;
    DSPDevice& operator=(const DSPDevice&)              = delete;

    /*-------------------------------------------------------------------------
    * Methods dealing with Host <-> DSP messaging infrastructure
    * To be implemented by DSPRootDevice and DSPSubDevice
    *------------------------------------------------------------------------*/
    virtual void             init()                  = 0;
    virtual void             pushEvent(Event* event) = 0;
    virtual bool             stop()                  = 0;
    virtual bool             availableEvent()        = 0;
    virtual Event*           getEvent(bool& stop)    = 0;
    virtual bool             gotEnoughToWorkOn()     = 0;
    virtual bool             mail_query()            = 0;
    virtual int              mail_from(const DSPCoreSet& compute_units,
                                       int* retcode = nullptr) = 0;
    virtual void             mail_to(Msg_t& msg,
                                     const DSPCoreSet& compute_units,
                                     unsigned core = 0) = 0;
    virtual int              num_complete_pending()     = 0;
    virtual void             dump_complete_pending()    = 0;
    virtual bool             any_complete_pending()     = 0;
    virtual void             push_complete_pending(uint32_t idx,
                                                   class Event* const data,
                                                   unsigned int cnt = 1) = 0;
    virtual bool             get_complete_pending(uint32_t idx,
                                                  class Event*& data) = 0;
    virtual pthread_cond_t*  get_worker_cond()          = 0;
    virtual pthread_mutex_t* get_worker_mutex()         = 0;
    virtual float            dspMhz()            const  { return p_dsp_mhz; }
    virtual unsigned char    dspID()             const  { return p_dsp_id;  }
    virtual const DSPDevice* GetRootDSPDevice()  const  = 0;

    /*-------------------------------------------------------------------------
    * Methods common to both DSPRootDevice and DSPSubDevice
    *------------------------------------------------------------------------*/
    DSPDevicePtr             get_addr_kernel_config()    { return p_addr_kernel_config;}
    profiling_t&             getProfiling()              { return p_profiling;         }
    std::ostream*            getProfilingOut()           { return p_profiling_out;     }
    std::string              builtinsHeader(void) const  { return "dsp.h";             }
    SharedMemory*            GetSHMHandler()      const  { return p_shmHandler;        }
    const DSPCoreSet&        GetComputeUnits()    const  { return p_compute_units;     }
    DSPDevice*               GetParent()          const  { return p_parent;            }
    bool                     isProfilingEnabled() const
    {
        return p_profiling.event_type >= 1
               && p_profiling.event_type <= 2
               && p_profiling.event_number1 >= 0;
    }

    cl_int                   info(cl_device_info param_name,
                                  size_t param_value_size,
                                  void* param_value,
                                  size_t* param_value_size_ret)      const;
    unsigned int             dspCores()                              const;
    bool                     addr_is_l2(DSPDevicePtr addr)           const;
    int                      numHostMails(Msg_t& msg)                const;
    MemoryRange::Location    ClFlagToLocation(cl_mem_flags flags)    const;

    DeviceBuffer*            createDeviceBuffer(MemObject* buffer, cl_int* rs);
    DeviceProgram*           createDeviceProgram(Program* program);
    DeviceKernel*            createDeviceKernel(Kernel* kernel, llvm::Function* function);
    void                     recordProfilingData(command_retcode_t*, uint32_t core);
    cl_int                   initEventDeviceData(Event* event);
    void                     freeEventDeviceData(Event* event);
    DSPDevicePtr             get_L2_extent(uint32_t& size);
    bool                     isInClMallocedRegion(void* ptr);

#if defined(DEVICE_K2X)
    void*                    get_mpax_default_res();
#endif

protected:
    uint32_t                        p_pid;
    unsigned char                   p_dsp_id;
    float                           p_dsp_mhz;
    DSPDevice*                      p_parent;
    DSPCoreSet                      p_compute_units;
    DSPDevicePtr                    p_addr_kernel_config;
    DSPDevicePtr                    p_addr_local_mem;
    uint32_t                        p_size_local_mem;
    profiling_t                     p_profiling;
    std::ostream*                   p_profiling_out;
    void*                           p_mpax_default_res;
    SharedMemory*                   p_shmHandler;
    cl_device_partition_property    p_partitions_supported[NUM_PARTITION_TYPES_SUPPORTED];
    cl_device_partition_property    p_partition_type[MAX_PARTITION_COUNT];
};

}
#endif
