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
 * \file api_program.cpp
 * \brief Programs
 */

#include "CL/cl.h"
#include <core/program.h>
#include <core/builtinprogram.h>
#include <core/context.h>
#include <core/platform.h>
#include <core/deviceinterface.h>

#include <cstdlib>
#include <set>
#include <map>

/**
 * Helper function to check whether the devices in the context match the
 * devices provided as arguments
 */
cl_int
checkDeviceComplianceWithContext(const Coal::Context* context,
                                 cl_uint              num_devices,
                                 cl_uint              context_num_devices,
                                 const cl_device_id*  device_list)
{
    cl_int errcode;
    cl_device_id* context_devices;

    context_devices =
        (cl_device_id *)std::malloc(context_num_devices * sizeof(cl_device_id));

    if(!context_devices) return CL_OUT_OF_RESOURCES;

    errcode = context->info(CL_CONTEXT_DEVICES,
                            context_num_devices * sizeof(cl_device_id),
                            context_devices, 0);

    if (errcode != CL_SUCCESS)
    {
        std::free(context_devices);
        return CL_INVALID_DEVICE;
    }

    for (cl_uint i=0; i<num_devices; ++i)
    {
        bool found = false;

        for (cl_uint j=0; j<context_num_devices; ++j)
        {
            if (device_list[i] == context_devices[j])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::free(context_devices);
            return CL_INVALID_DEVICE;
        }
    }

    std::free(context_devices);
    return CL_SUCCESS;
}

// Program Object APIs
cl_program
clCreateProgramWithSource(cl_context        d_context,
                          cl_uint           count,
                          const char **     strings,
                          const size_t *    lengths,
                          cl_int *          errcode_ret)
{
    cl_int dummy_errcode;
    auto context = pobj(d_context);

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    if (!count || !strings)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    Coal::Program *program = new Coal::Program(context);

    *errcode_ret = CL_SUCCESS;
    *errcode_ret = program->loadSources(count, strings, lengths);

    if (*errcode_ret != CL_SUCCESS)
    {
        delete program;
        return 0;
    }

    return desc(program);
}

cl_program
clCreateProgramWithBuiltInKernels(cl_context                d_context,
                                  cl_uint                   num_devices,
                                  const cl_device_id *      device_list,
                                  const char *              kernel_names,
                                  cl_int *                  errcode_ret)
{
    cl_int dummy_errcode;
    cl_uint context_num_devices = 0;

    auto context = pobj(d_context);

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    if (!num_devices || !device_list || !kernel_names)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    // Get number of devices in context
    *errcode_ret = context->info(CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint),
                                 &context_num_devices, 0);

    if (*errcode_ret != CL_SUCCESS) return 0;

    // Check device compliance with context
    *errcode_ret = checkDeviceComplianceWithContext(context,
                                                    num_devices,
                                                    context_num_devices,
                                                    device_list);
    if (*errcode_ret != CL_SUCCESS) return 0;

    // Create program
    Coal::BuiltInProgram *program = new Coal::BuiltInProgram(context);
    *errcode_ret = CL_SUCCESS;

    // Init Program
    *errcode_ret = program->loadBuiltInKernels(num_devices, device_list,
                                               kernel_names);

    if (*errcode_ret != CL_SUCCESS)
    {
        delete program;
        return 0;
    }

    return desc(program);
}

cl_program
clCreateProgramWithBinary(cl_context            d_context,
                          cl_uint               num_devices,
                          const cl_device_id *  device_list,
                          const size_t *        lengths,
                          const unsigned char **binaries,
                          cl_int *              binary_status,
                          cl_int *              errcode_ret)
{
    cl_int dummy_errcode;
    cl_uint context_num_devices = 0;
    auto context = pobj(d_context);

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    if (!num_devices || !device_list || !lengths || !binaries)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    // Get number of devices in context
    *errcode_ret = context->info(CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint),
                                 &context_num_devices, 0);

    if (*errcode_ret != CL_SUCCESS) return 0;

    // Check devices for compliance with context
    *errcode_ret = checkDeviceComplianceWithContext(context,
                                                    num_devices,
                                                    context_num_devices,
                                                    device_list);
    if (*errcode_ret != CL_SUCCESS) return 0;

    // Check binary status
    for (cl_uint i=0; i<num_devices; ++i)
    {
        if (!lengths[i] || !binaries[i])
        {
            if (binary_status)
                binary_status[i] = CL_INVALID_VALUE;

            *errcode_ret = CL_INVALID_VALUE;
            return 0;
        }
    }

    // Create a program
    Coal::Program *program = new Coal::Program(context);
    *errcode_ret = CL_SUCCESS;

    // Init program
    *errcode_ret = program->loadBinaries(binaries, lengths, binary_status,
                                         num_devices, device_list);

    if (*errcode_ret != CL_SUCCESS)
    {
        delete program;
        return 0;
    }

    return desc(program);
}

cl_int
clRetainProgram(cl_program d_program)
{
    auto program = pobj(d_program);
    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    program->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseProgram(cl_program d_program)
{
    auto program = pobj(d_program);
    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    if (program->dereference())
        delete program;

    return CL_SUCCESS;
}

cl_int
clBuildProgram(cl_program           d_program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options,
               void (*pfn_notify)(cl_program program, void * user_data),
               void *               user_data)
{
    auto program = pobj(d_program);

    if ((!device_list && num_devices > 0) ||
        (!num_devices && device_list)     ||
        (!pfn_notify && user_data)         )
    {
        return CL_INVALID_VALUE;
    }

    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    cl_uint context_num_devices = 0;
    cl_device_id *context_devices;
    Coal::Context *context = (Coal::Context *)program->parent();
    cl_int result;

    result = context->info(CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint),
                                 &context_num_devices, 0);

    if (result != CL_SUCCESS) return result;

    context_devices =
        (cl_device_id *)std::malloc(context_num_devices * sizeof(cl_device_id));

    if(!context_devices) return CL_OUT_OF_RESOURCES;

    result = context->info(CL_CONTEXT_DEVICES,
                                 context_num_devices * sizeof(cl_device_id),
                                 context_devices, 0);

    if (result != CL_SUCCESS)
    {
        std::free(context_devices);
        return result;
    }


    // Check the devices for compliance
    if (num_devices)
    {
        for (cl_uint i=0; i<num_devices; ++i)
        {
            bool found = false;

            for (cl_uint j=0; j<context_num_devices; ++j)
            {
                if (device_list[i] == context_devices[j])
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                std::free(context_devices);
                return CL_INVALID_DEVICE;
            }
        }
    }
    else
    {
        num_devices = context_num_devices;
        device_list = context_devices;
    }

    // We cannot try to build a previously-failed program or a BuiltInProgram
    if (!(program->state() == Coal::Program::Loaded ||
          program->state() == Coal::Program::Built  ||
          program->state() == Coal::Program::Linked) ||
         program->type() == Coal::Program::BuiltIn)
    {
        std::free(context_devices);
        return CL_INVALID_OPERATION;
    }

    /* Build program */
    result =  program->build(options, pfn_notify, user_data,
                             num_devices, device_list);

    std::free(context_devices);

    return result;
}

#ifndef _SYS_BIOS
cl_int
clCompileProgram(cl_program           d_program,
                 cl_uint              num_devices,
                 const cl_device_id * device_list,
                 const char *         options,
                 cl_uint              num_input_headers,
                 const cl_program *   input_headers,
                 const char **        header_include_names,
                 void (*pfn_notify)(cl_program program, void * user_data),
                 void *               user_data)
{
    auto program = pobj(d_program);

    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    if (!device_list && num_devices > 0)
        return CL_INVALID_VALUE;

    if (!num_devices && device_list)
        return CL_INVALID_VALUE;

    if (!pfn_notify && user_data)
        return CL_INVALID_VALUE;

    cl_uint context_num_devices = 0;
    Coal::Context *context      = (Coal::Context *)program->parent();
    cl_int result;

    result = context->info(CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint),
                           &context_num_devices, 0);

    if (result != CL_SUCCESS) return result;

    if (num_devices !=0)
    {
        // Check device compliance with context
        result = checkDeviceComplianceWithContext(context,
                                                  num_devices,
                                                  context_num_devices,
                                                  device_list);
    }

    if (result != CL_SUCCESS) return result;

    // We cannot try to compile a previously-failed program or a BuiltInProgram
    if (!(program->state() == Coal::Program::Loaded ||
          program->state() == Coal::Program::Built  ||
          program->state() == Coal::Program::Linked ) ||
         program->type() == Coal::Program::BuiltIn)
    {
        return CL_INVALID_OPERATION;
    }

    // Check errors in input_headers
    std::map<std::string, std::string> input_header_src;
    for(cl_uint i=0; i<num_input_headers; i++)
    {
        auto ih_program = pobj(input_headers[i]);
        if (!ih_program->isA(Coal::Object::T_Program))
            return CL_INVALID_PROGRAM;

        // The input header program must have been created from source
        if (ih_program->type() != Coal::Program::Source)
            return CL_INVALID_PROGRAM;

        auto ih_name       = std::string(header_include_names[i]);
        std::string ih_src = ih_program->source();

        input_header_src[ih_name] = ih_src;
    }

    /* If device_list is NULL and num_devices is 0, then compile for all
     * devices in context*/
    if(device_list == nullptr && num_devices == 0)
    {
        cl_device_id* context_devices;

        context_devices =
        (cl_device_id *)std::malloc(context_num_devices * sizeof(cl_device_id));

        if(!context_devices) return CL_OUT_OF_RESOURCES;

        result = context->info(CL_CONTEXT_DEVICES,
                               context_num_devices * sizeof(cl_device_id),
                               context_devices, 0);

        /* Build program */
        result =  program->compile(options,
                                   pfn_notify,
                                   user_data,
                                   input_header_src,
                                   context_num_devices,
                                   context_devices);
        std::free(context_devices);

    }
    else
    {
        /* Build program */
        result =  program->compile(options,
                                   pfn_notify,
                                   user_data,
                                   input_header_src,
                                   num_devices,
                                   device_list);
    }

    return result;
}

cl_program
clLinkProgram(cl_context           d_context,
              cl_uint              num_devices,
              const cl_device_id * device_list,
              const char *         options,
              cl_uint              num_input_programs,
              const cl_program *   input_programs,
              void (*pfn_notify)(cl_program program, void * user_data),
              void *               user_data,
              cl_int *             errcode_ret)
{
    cl_uint context_num_devices = 0;
    auto context = pobj(d_context);

    if ((!device_list && num_devices > 0) ||
        (!num_devices && device_list)     ||
        (!pfn_notify && user_data)         )
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    // Get number of devices in context
    *errcode_ret = context->info(CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint),
                                 &context_num_devices, 0);

    if (*errcode_ret != CL_SUCCESS) return 0;

    // Check devices for compliance with context
    *errcode_ret = checkDeviceComplianceWithContext(context,
                                                    num_devices,
                                                    context_num_devices,
                                                    device_list);
    if (*errcode_ret != CL_SUCCESS) return 0;

    // Check errors in input_programs
    std::vector<Coal::Program*> input_program_list;
    for(cl_uint i=0; i<num_input_programs; i++)
    {
        auto i_program = pobj(input_programs[i]);
        if (!i_program->isA(Coal::Object::T_Program))
        {
            *errcode_ret = CL_INVALID_PROGRAM;
            return 0;
        }

        input_program_list.push_back(i_program);
    }

    Coal::Program *program = new Coal::Program(context);

    /* If device_list is NULL and num_devices is 0, then link for all
     * devices in context*/
    if(device_list == nullptr && num_devices == 0)
    {
        cl_device_id* context_devices;

        context_devices =
        (cl_device_id *)std::malloc(context_num_devices * sizeof(cl_device_id));

        if(!context_devices)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
            delete program;
            return 0;
        }

        *errcode_ret = context->info(CL_CONTEXT_DEVICES,
                                     context_num_devices * sizeof(cl_device_id),
                                     context_devices, 0);

        /* Link program */
        *errcode_ret =  program->link(options, pfn_notify, user_data,
                                      input_program_list,
                                      context_num_devices, context_devices);

        std::free(context_devices);
    }
    else
    {
        /* Link program */
        *errcode_ret =  program->link(options, pfn_notify, user_data,
                                      input_program_list,
                                      num_devices, device_list);
    }

    if (*errcode_ret != CL_SUCCESS)
    {
        delete program;
        return 0;
    }

    return desc(program);
}
#endif

cl_int
clUnloadCompiler(void)
{
    return CL_SUCCESS;
}

cl_int
clUnloadPlatformCompiler(cl_platform_id platform)
{
    if (platform != (cl_platform_id) &the_platform::Instance())
        return CL_INVALID_PLATFORM;
    return CL_SUCCESS;
}

cl_int
clGetProgramInfo(cl_program         d_program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret)
{
    auto program = pobj(d_program);
    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    return program->info(param_name, param_value_size, param_value,
                         param_value_size_ret);
}

cl_int
clGetProgramBuildInfo(cl_program            d_program,
                      cl_device_id          d_device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret)
{
    auto program = pobj(d_program);
    auto device = pobj(d_device);

    if (!program->isA(Coal::Object::T_Program))
        return CL_INVALID_PROGRAM;

    if (!device)
        return CL_INVALID_DEVICE;

    return program->buildInfo(device, param_name,
                              param_value_size, param_value,
                              param_value_size_ret);
}
