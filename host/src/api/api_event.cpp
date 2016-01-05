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
 * \file api_event.cpp
 * \brief Special events and event management
 */

#include <CL/cl.h>

#include <core/commandqueue.h>
#include <core/events.h>
#include <core/context.h>
#include <stdio.h>

// Event Object APIs
cl_int
clWaitForEvents(cl_uint             num_events,
                const cl_event *    event_list)
{
    if (!num_events || !event_list)
        return CL_INVALID_VALUE;

    // Check if the events in the list have the same context
    cl_context global_ctx = 0;

    for (cl_uint i=0; i<num_events; ++i)
    {
        if (!event_list[i]->isA(Coal::Object::T_Event))
            return CL_INVALID_EVENT;

        if (event_list[i]->status() < 0)
            return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;

        cl_context evt_ctx;
        if (event_list[i]->type() == Coal::Event::User)
            evt_ctx = (cl_context)((Coal::UserEvent *)event_list[i])->context();
        else
            evt_ctx = (cl_context)event_list[i]->parent()->parent();

#if 0 // YUAN: no need to wait for queue to be flushed
        cl_command_queue evt_queue = (cl_command_queue)event_list[i]->parent();
        // Flush the queue
        evt_queue->flush();
#endif

        if (global_ctx == 0)
            global_ctx = evt_ctx;
        else if (global_ctx != evt_ctx)
            return CL_INVALID_CONTEXT;
    }

    // Wait for the events
    for (cl_uint i=0; i<num_events; ++i)
    {
        event_list[i]->waitForStatus(Coal::Event::Complete);
        // Per OpenCL spec, we need to return this error if any event
        // in the event_wait_list fails
        if (event_list[i]->status() < 0)
            return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
    }

    return CL_SUCCESS;
}

cl_int
clGetEventInfo(cl_event         event,
               cl_event_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret)
{
    if (!event->isA(Coal::Object::T_Event))
        return CL_INVALID_EVENT;

    return event->info(param_name, param_value_size, param_value,
                       param_value_size_ret);
}

cl_int
clSetEventCallback(cl_event     event,
                   cl_int       command_exec_callback_type,
                   void         (CL_CALLBACK *pfn_event_notify)(cl_event event,
                                                                cl_int exec_status,
                                                                void *user_data),
                   void *user_data)
{
    if (!event->isA(Coal::Object::T_Event))
        return CL_INVALID_EVENT;

    if (!pfn_event_notify || command_exec_callback_type != CL_COMPLETE)
        return CL_INVALID_VALUE;

    event->setCallback(command_exec_callback_type, pfn_event_notify, user_data);

    return CL_SUCCESS;
}

cl_int
clRetainEvent(cl_event event)
{
    if (!event->isA(Coal::Object::T_Event))
        return CL_INVALID_EVENT;

    event->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseEvent(cl_event event)
{
    if (!event->isA(Coal::Object::T_Event))
        return CL_INVALID_EVENT;

    if (event->dereference())
    {
        event->freeDeviceData();
        delete event;
    }

    return CL_SUCCESS;
}

cl_event
clCreateUserEvent(cl_context    context,
                  cl_int *      errcode_ret)
{
    cl_int dummy_errcode;

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    *errcode_ret = CL_SUCCESS;

    Coal::UserEvent *command = new Coal::UserEvent(
        (Coal::Context *)context, errcode_ret
    );

    if (*errcode_ret != CL_SUCCESS)
    {
        delete command;
        return 0;
    }

    return (cl_event)command;
}

cl_int
clSetUserEventStatus(cl_event   event,
                     cl_int     execution_status)
{
    Coal::Event *command = (Coal::Event *)event;

    if (!command->isA(Coal::Object::T_Event) ||
        command->type() != Coal::Event::User)
        return CL_INVALID_EVENT;

    if (execution_status != CL_COMPLETE)
        return CL_INVALID_VALUE;

    if (command->status() != CL_SUBMITTED)
        return CL_INVALID_OPERATION;

    command->setStatus((Coal::Event::Status)execution_status);

    return CL_SUCCESS;
}
