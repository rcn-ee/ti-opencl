/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file api_kernel.cpp
 * \brief Kernels
 */
#ifdef _SYS_BIOS
#include <sys/types.h>
#include <xdc/std.h>
#endif
#include "CL/cl.h"

#include <core/program.h>
#include <core/kernel.h>

// Kernel Object APIs
cl_kernel
clCreateKernel(cl_program      program,
               const char *    kernel_name,
               cl_int *        errcode_ret)
{
    cl_int dummy_errcode;

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!kernel_name)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    if (!program->isA(Coal::Object::T_Program))
    {
        *errcode_ret = CL_INVALID_PROGRAM;
        return 0;
    }

    if (program->state() != Coal::Program::Built)
    {
        *errcode_ret = CL_INVALID_PROGRAM_EXECUTABLE;
        return 0;
    }

    Coal::Kernel *kernel = program->createKernel(kernel_name, errcode_ret);

    if (*errcode_ret != CL_SUCCESS)
    {
        delete kernel;
        return 0;
    }

    return (cl_kernel)kernel;
}

cl_int
clCreateKernelsInProgram(cl_program     program,
                         cl_uint        num_kernels,
                         cl_kernel *    kernels,
                         cl_uint *      num_kernels_ret)
{
    cl_int rs = CL_SUCCESS;

    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    if (program->state() != Coal::Program::Built)
        return CL_INVALID_PROGRAM_EXECUTABLE;

    std::vector<Coal::Kernel *> ks = program->createKernels(&rs);

    if (rs != CL_SUCCESS)
    {
        while (ks.size())
        {
            delete ks.back();
            ks.pop_back();
        }

        return rs;
    }

    // Check that the kernels will fit in the array, if needed
    if (num_kernels_ret)
        *num_kernels_ret = ks.size();

    if (kernels && num_kernels < ks.size())
    {
        while (ks.size())
        {
            delete ks.back();
            ks.pop_back();
        }

        return CL_INVALID_VALUE;
    }

    if (!kernels)
    {
        // We don't need the kernels in fact
        while (ks.size())
        {
            delete ks.back();
            ks.pop_back();
        }
    }
    else
    {
        // Copy the kernels
        for (size_t i=0; i<ks.size(); ++i)
        {
            kernels[i] = (cl_kernel)ks[i];
        }
    }

    return CL_SUCCESS;
}

cl_int
clRetainKernel(cl_kernel    kernel)
{
    if (!kernel->isA(Coal::Object::T_Kernel))
        return CL_INVALID_KERNEL;

    kernel->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseKernel(cl_kernel   kernel)
{
    if (!kernel->isA(Coal::Object::T_Kernel))
        return CL_INVALID_KERNEL;

    if (kernel->dereference())
        delete kernel;

    return CL_SUCCESS;
}

cl_int
clSetKernelArg(cl_kernel    kernel,
               cl_uint      arg_indx,
               size_t       arg_size,
               const void * arg_value)
{
    if (!kernel->isA(Coal::Object::T_Kernel))
        return CL_INVALID_KERNEL;

    return kernel->setArg(arg_indx, arg_size, arg_value);
}

cl_int
clGetKernelInfo(cl_kernel       kernel,
                cl_kernel_info  param_name,
                std::size_t          param_value_size,
                void *          param_value,
				std::size_t *        param_value_size_ret)
{
    if (!kernel->isA(Coal::Object::T_Kernel))
        return CL_INVALID_KERNEL;

    return kernel->info(param_name, param_value_size, param_value,
                        param_value_size_ret);
}

cl_int
clGetKernelWorkGroupInfo(cl_kernel                  kernel,
                         cl_device_id               device,
                         cl_kernel_work_group_info  param_name,
                         size_t                     param_value_size,
                         void *                     param_value,
                         size_t *                   param_value_size_ret)
{
    if (!kernel->isA(Coal::Object::T_Kernel))
        return CL_INVALID_KERNEL;

    return kernel->workGroupInfo((Coal::DeviceInterface *)device, param_name,
                                 param_value_size, param_value,
                                 param_value_size_ret);
}
