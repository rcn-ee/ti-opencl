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
 * \file api_context.cpp
 * \brief Contexts
 */

#include <CL/cl.h>
#include <core/context.h>
#include <stdlib.h>
#include "api_platform.h"

// Context APIs

cl_context
clCreateContext(const cl_context_properties  *properties,
                cl_uint                       num_devices,
                const cl_device_id *          devices,
                void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                void *                        user_data,
                cl_int *                      errcode_ret)
{
    cl_int default_errcode_ret;

    // No errcode_ret ?
    if (!errcode_ret)
        errcode_ret = &default_errcode_ret;

    if (!devices ||
        !num_devices ||
        (!pfn_notify && user_data))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    *errcode_ret = CL_SUCCESS;
    Coal::Context *ctx = new Coal::Context(properties, num_devices, devices,
                                           pfn_notify, user_data, errcode_ret);

    if (*errcode_ret != CL_SUCCESS)
    {
        // Initialization failed, destroy context
        delete ctx;
        return 0;
    }

    return (_cl_context *)ctx;
}

cl_context
clCreateContextFromType(const cl_context_properties   *properties,
                        cl_device_type          device_type,
                        void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                        void *                  user_data,
                        cl_int *                errcode_ret)
{
    cl_device_id* devices;
    cl_uint      num_devices;

    *errcode_ret = clGetDeviceIDs(PLATFORM_ID, device_type, 0, NULL, &num_devices);
    if (!num_devices) { *errcode_ret = CL_INVALID_DEVICE; return NULL; }

    devices = (cl_device_id*) malloc(num_devices * sizeof(cl_device_id));
    if (!devices) { *errcode_ret = CL_OUT_OF_HOST_MEMORY; return NULL; }

    *errcode_ret = clGetDeviceIDs(PLATFORM_ID, device_type, num_devices, devices, 0);

    if (*errcode_ret != CL_SUCCESS) { free (devices); return NULL; }

    cl_context result = clCreateContext(properties, num_devices, devices, pfn_notify, user_data,
                                       errcode_ret);

    free (devices); 
    return result;
}

cl_int
clRetainContext(cl_context context)
{
    if (!context->isA(Coal::Object::T_Context))
        return CL_INVALID_CONTEXT;

    context->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseContext(cl_context context)
{
    if (!context->isA(Coal::Object::T_Context))
        return CL_INVALID_CONTEXT;

    if (context->dereference())
        delete context;

    return CL_SUCCESS;
}

cl_int
clGetContextInfo(cl_context         context,
                 cl_context_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret)
{
    if (!context->isA(Coal::Object::T_Context))
        return CL_INVALID_CONTEXT;

    return context->info(param_name, param_value_size, param_value,
                         param_value_size_ret);
}
