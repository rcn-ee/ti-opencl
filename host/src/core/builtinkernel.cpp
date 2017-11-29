/*
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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
 * \file core/builtinkernel.cpp
 * \brief BuiltInKernel
 */

#include "builtinkernel.h"

namespace Coal
{

BuiltInKernel::BuiltInKernel(BuiltInProgram *program) : Kernel(program)
{
}

void BuiltInKernel::reqdWorkGroupSize(llvm::Module *module, cl_uint dims[3]) const
{
    dims[0] = dims[1] = dims[2] = 1;
}

cl_int BuiltInKernel::addBuiltInFunction(DeviceInterface *device, KernelEntry *kernel)
{
    DeviceDependent dep;
    dep.device = device;
    dep.function = NULL;
    dep.module = NULL;
    dep.kernel = device->createDeviceBuiltInKernel(this, kernel);
    p_device_dependent.push_back(dep);

    if(!p_args.size())
    {
        std::vector<Kernel::Arg> &args = kernel->args;
        for(int i=0; i<args.size(); i++)
        {
            p_args.push_back(args[i]);
        }
    }

    return CL_SUCCESS;
}


}
