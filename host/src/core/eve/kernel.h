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
#ifndef __EVE_KERNEL_H__
#define __EVE_KERNEL_H__

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
#include "../tiocl_thread.h"
#include "../tiocl_types.h"
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
class EVEDevice;
class Kernel;
class KernelEvent;

class EVEKernel : public DeviceKernel
{
    public:
        EVEKernel(EVEDevice *device, Kernel *kernel, llvm::Function *function);
        ~EVEKernel();

        size_t       workGroupSize()   const                { return 1; }
        cl_ulong     localMemSize()    const                { return 0; }
        cl_ulong     privateMemSize()  const                { return 0; }
        size_t       preferredWorkGroupSizeMultiple() const { return 1; }
        size_t       guessWorkGroupSize(cl_uint num_dims, cl_uint dim,
                                        size_t global_work_size) const
                                                            { return 1; }

        Kernel *     kernel() const      { return p_kernel; }
        EVEDevice *  device() const      { return p_device; }
        llvm::Function *function() const { return p_function; }
        uint32_t     GetBuiltInKernelIndex() const
                                         { return p_builtin_kernel_index_; }

        cl_int       preAllocBuffers();

    private:
        EVEDevice *     p_device;
        Kernel *        p_kernel;
        llvm::Function *p_function;
        uint32_t        p_builtin_kernel_index_;
};

class EVEKernelEvent
{
    public:
        EVEKernelEvent  (EVEDevice *device, KernelEvent *event);
        ~EVEKernelEvent ();

        cl_int callArgs (unsigned rs_size);
        cl_int run      (Event::Type evtype);

        cl_int get_ret_code() { return p_ret_code;  }
        EVEDevice* device()   { return p_device;    }
        uint32_t kernel_id()  { return p_kernel_id; }

        void free_tmp_bufs();

    private:
        cl_int                    p_ret_code;
        EVEDevice *               p_device;
        KernelEvent *             p_event;
        EVEKernel *               p_kernel;
        uint32_t                  p_kernel_id;
        Msg_t                     p_msg;
        std::vector<HostptrPair>  p_hostptr_tmpbufs;
        std::vector<MemObject *>  p_hostptr_clMalloced_bufs;

        /*---------------------------------------------------------------------
        * Helpers for run member function
        *--------------------------------------------------------------------*/
        cl_int init_kernel_runtime_variables(Event::Type evtype);
        cl_int allocate_temp_global (void);
        cl_int flush_special_use_host_ptr_buffers(void);
        cl_int setup_stack_based_arguments(void);
};
}
#endif
