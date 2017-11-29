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
 * \file core/builtinprogram.cpp
 * \brief BuiltInProgram
 */

#include "builtinprogram.h"
#include "context.h"
#include "builtinkernel.h"
#include "propertylist.h"
#include "deviceinterface.h"
#include "util.h"

#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>

namespace Coal
{

BuiltInProgram::BuiltInProgram(Context *ctx) : Program(ctx){}

/******************************************************************************
* Kernel *BuiltInProgram::createKernel(const std::string &name, 
*                                      cl_int *errcode_ret)
******************************************************************************/
Kernel *BuiltInProgram::createKernel(const std::string &name, 
                                     cl_int *errcode_ret)
{
    BuiltInKernel *rs = new BuiltInKernel(this);
    
    /*-------------------------------------------------------------------------
    * Add a function definition for each device
    *------------------------------------------------------------------------*/
    for(size_t i=0; i<p_device_dependent.size(); ++i)
    {
        bool found = false;
        DeviceDependent &dep = p_device_dependent[i];
        
        /*---------------------------------------------------------------------
        * Find the one with the good name
        *--------------------------------------------------------------------*/
        for(KernelEntry *k : dep.loaded_kernel_entries)
        {
            if(name == k->name)
            {
                found = true;
                *errcode_ret = rs->addBuiltInFunction(dep.device, k);
                if (*errcode_ret != CL_SUCCESS) return rs;
                break;
            }
        }
        
        /*---------------------------------------------------------------------
        * Kernel unavailable for this device
        *--------------------------------------------------------------------*/
        if(!found)
        {
            *errcode_ret = CL_INVALID_KERNEL_NAME;
            return rs;
        }
    }

    return rs;
}

std::vector<std::string> splitString(std::string s, char delim)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream ts(s);
    while(std::getline(ts,token,delim))
    { tokens.push_back(token); }
    return tokens;
}

/******************************************************************************
* cl_int BuiltInProgram::loadBuiltInKernels(cl_uint num_devices, 
*                                           DeviceInterface * const *device_list,
*                                           cost char *kernel_names)
******************************************************************************/
cl_int BuiltInProgram::loadBuiltInKernels(cl_uint num_devices,
                                          DeviceInterface * const *device_list,
                                          const char *kernel_names)
{
    setDevices(num_devices, device_list);

    std::vector<std::string> requested_kernels = splitString(std::string(kernel_names), ';');
    std::unordered_set<std::string> requested_kernels_set(requested_kernels.begin(), requested_kernels.end());
    std::unordered_set<std::string> loaded_kernel_names;
    
    for(cl_uint i=0; i<num_devices; i++)
    {
        DeviceInterface *dev = device_list[i];
        DeviceDependent &dep = deviceDependent(device_list[i]);

        std::vector<KernelEntry *> *device_builtin_kernels = dev->getKernelEntries();

        for(KernelEntry *k : *device_builtin_kernels)
        {
            if(requested_kernels_set.count(k->name))
            {
                dep.loaded_kernel_entries.push_back(k);
                /* Used for checking that each requested kernel was found for at least 1 device */
                loaded_kernel_names.insert(k->name);
            }
        }
    }
    
    /* Error if a requested kernel was not found for any device */
    if(requested_kernels.size() != loaded_kernel_names.size()) return CL_INVALID_VALUE;

    p_type = BuiltIn;
    p_state = Built;

    return CL_SUCCESS;
}

}
