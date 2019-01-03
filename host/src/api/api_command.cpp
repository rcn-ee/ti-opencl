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
 * \file api_command.cpp
 * \brief Command queues
 */

#include <core/commandqueue.h>
#include <core/deviceinterface.h>
#include <core/context.h>

#include <CL/TI/cl.h>

// Command Queue APIs
cl_command_queue
clCreateCommandQueue(cl_context                     d_context,
                     cl_device_id                   d_device,
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret)
{
    cl_int default_errcode_ret;
    auto device = pobj(d_device);
    auto context = pobj(d_context);

    // No errcode_ret ?
    if (!errcode_ret)
        errcode_ret = &default_errcode_ret;

    if (!device->isA(Coal::Object::T_Device))
    {
        *errcode_ret = CL_INVALID_DEVICE;
        return 0;
    }

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    *errcode_ret = CL_SUCCESS;
    Coal::CommandQueue *queue = new Coal::CommandQueue(
                                        context,
                                        device,
                                        properties,
                                        errcode_ret);

    if (*errcode_ret != CL_SUCCESS)
    {
        // Initialization failed, destroy context
        delete queue;
        return 0;
    }

    return desc(queue);
}

cl_int
clRetainCommandQueue(cl_command_queue d_command_queue)
{
    auto command_queue = pobj(d_command_queue);

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    command_queue->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseCommandQueue(cl_command_queue d_command_queue)
{
    auto command_queue = pobj(d_command_queue);

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    command_queue->flush();

    if (command_queue->dereference())
        delete command_queue;

    return CL_SUCCESS;
}

cl_int
clGetCommandQueueInfo(cl_command_queue      d_command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret)
{
    auto command_queue = pobj(d_command_queue);

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    return command_queue->info(param_name, param_value_size, param_value,
                               param_value_size_ret);
}

cl_int
clSetCommandQueueProperty(cl_command_queue              d_command_queue,
                          cl_command_queue_properties   properties,
                          cl_bool                       enable,
                          cl_command_queue_properties * old_properties)
{
    auto command_queue = pobj(d_command_queue);

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    return command_queue->setProperty(properties, enable, old_properties);
}
