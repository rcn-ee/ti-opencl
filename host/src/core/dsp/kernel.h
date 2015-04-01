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
#ifndef __DSP_KERNEL_H__
#define __DSP_KERNEL_H__

#include "../events.h"
#include "../memobject.h"
#include "../deviceinterface.h"
#include "../kernel.h"
#include "message.h"
#include "device.h"
#include <core/config.h>
#include <llvm/IR/Function.h>

#include <vector>
#include <string>
#include <pthread.h>
#include <stdint.h>

namespace llvm
{
    class Function;
}

typedef std::pair<DSPDevicePtr64, DSPVirtPtr *> DSPPtrPair;
typedef std::pair<DSPPtrPair,   uint32_t> DSPMemRange;
typedef std::pair<DSPVirtPtr *, uint32_t> LocalPair;
typedef std::pair<Coal::MemObject *, DSPPtrPair> HostptrPair;


namespace Coal
{
class DSPDevice;
class Kernel;
class KernelEvent;

class DSPKernel : public DeviceKernel
{
    public:
        DSPKernel(DSPDevice *device, Kernel *kernel, llvm::Function *function);
        ~DSPKernel();

        size_t       workGroupSize()          const ;

        
        cl_ulong     localMemSize()           const ;
        cl_ulong     privateMemSize()         const { return p_kernel->get_wi_alloca_size(); }
        size_t       preferredWorkGroupSizeMultiple() const { return 8; }

        size_t       guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                                        size_t global_work_size) const;
        DSPDevicePtr device_entry_pt();
        DSPDevicePtr data_page_ptr();
        cl_int       preAllocBuffers();

        Kernel *     kernel() const;     
        DSPDevice *  device() const;  

        llvm::Function *function() const { return p_function; }
        static size_t typeOffset(size_t &offset, size_t type_len);

    private:
        DSPDevice *     p_device;
        Kernel *        p_kernel;
        DSPDevicePtr    p_device_entry_pt;
        DSPDevicePtr    p_data_page_ptr;
        llvm::Function *p_function;
};

class DSPKernelEvent
{
    public:
        DSPKernelEvent  (DSPDevice *device, KernelEvent *event);
        ~DSPKernelEvent ();

        cl_int run      (Event::Type evtype);
        cl_int callArgs (unsigned rs_size);

        cl_int get_ret_code() { return p_ret_code;  }
        DSPDevice* device()   { return p_device;    }
        uint32_t kernel_id()  { return p_kernel_id; }

        void free_tmp_bufs();

    private:
        cl_int                    p_ret_code;
        DSPDevice *               p_device;
        KernelEvent *             p_event;
        DSPKernel *               p_kernel;
        uint32_t                  p_kernel_id;
        bool                      p_debug_kernel;
        int                       p_num_arg_words;
        Msg_t                     p_msg;
        DSPDevicePtr64            p_WG_alloca_start;
        std::vector<DSPMemRange>  p_flush_bufs;
        std::vector<LocalPair>    p_local_bufs;
        std::vector<HostptrPair>  p_hostptr_tmpbufs;
        std::vector<MemObject *>  p_hostptr_clMalloced_bufs;
        std::vector<DSPMemRange>  p_64bit_bufs;

        char args_on_stack[ MAX_ARGS_TOTAL_SIZE*2];
        char args_of_argref[MAX_ARGS_TOTAL_SIZE*2];
        int  argref_offset;
        std::vector<LocalPair>    p_argrefs;

        /*---------------------------------------------------------------------
        * Helpers for run member function
        *--------------------------------------------------------------------*/
        cl_int allocate_and_assign_local_buffers(uint32_t &, DSPDevicePtr &);
        cl_int init_kernel_runtime_variables(Event::Type evtype, uint32_t, DSPDevicePtr);
        cl_int allocate_temp_global (void);
        cl_int flush_special_use_host_ptr_buffers(void);
        cl_int setup_extended_memory_mappings(void);
        cl_int setup_stack_based_arguments(void);
        int debug_kernel_dispatch();

};
}
#endif
