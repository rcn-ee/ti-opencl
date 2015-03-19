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
 * \file api_enqueue.cpp
 * \brief Events
 */

#include <CL/cl.h>

#include <core/events.h>
#include <core/memobject.h>

#include <cstdlib>

static inline cl_int queueEvent(Coal::CommandQueue *queue,
                                Coal::Event *command,
                                cl_event *event,
                                cl_bool blocking)
{
    cl_int rs;

    rs = queue->queueEvent(command);

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    if (event)
    {
        *event = (cl_event)command;
        command->reference();
    }

    if (blocking)
    {
        rs = clWaitForEvents(1, (cl_event *)&command);

        if (rs != CL_SUCCESS)
        {
            delete command;
            return rs;
        }
    }

    return CL_SUCCESS;
}

// Enqueued Commands APIs
cl_int
clEnqueueReadBuffer(cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              cb,
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::ReadBufferEvent *command = new Coal::ReadBufferEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)buffer,
        offset, cb, ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, blocking_read);
}

cl_int
clEnqueueWriteBuffer(cl_command_queue   command_queue,
                     cl_mem             buffer,
                     cl_bool            blocking_write,
                     size_t             offset,
                     size_t             cb,
                     const void *       ptr,
                     cl_uint            num_events_in_wait_list,
                     const cl_event *   event_wait_list,
                     cl_event *         event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::WriteBufferEvent *command = new Coal::WriteBufferEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)buffer,
        offset, cb, (void *)ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, blocking_write);
}

cl_int
clEnqueueReadBufferRect(cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_read,
                        const size_t *      buffer_origin,
                        const size_t *      host_origin,
                        const size_t *      region,
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        void *              ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::ReadBufferRectEvent *command = new Coal::ReadBufferRectEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)buffer,
        buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, blocking_read);
}

cl_int
clEnqueueWriteBufferRect(cl_command_queue    command_queue,
                        cl_mem              buffer,
                        cl_bool             blocking_write,
                        const size_t *      buffer_origin,
                        const size_t *      host_origin,
                        const size_t *      region,
                        size_t              buffer_row_pitch,
                        size_t              buffer_slice_pitch,
                        size_t              host_row_pitch,
                        size_t              host_slice_pitch,
                        const void *        ptr,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::WriteBufferRectEvent *command = new Coal::WriteBufferRectEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)buffer,
        buffer_origin, host_origin, region, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, (void *)ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, blocking_write);
}

cl_int
clEnqueueCopyBufferRect(cl_command_queue    command_queue,
                        cl_mem              src_buffer,
                        cl_mem              dst_buffer,
                        const size_t *      src_origin,
                        const size_t *      dst_origin,
                        const size_t *      region,
                        size_t              src_row_pitch,
                        size_t              src_slice_pitch,
                        size_t              dst_row_pitch,
                        size_t              dst_slice_pitch,
                        cl_uint             num_events_in_wait_list,
                        const cl_event *    event_wait_list,
                        cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::CopyBufferRectEvent *command = new Coal::CopyBufferRectEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)src_buffer,
        (Coal::MemObject *)dst_buffer,
        src_origin, dst_origin, region, src_row_pitch, src_slice_pitch,
        dst_row_pitch, dst_slice_pitch, 1,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueCopyBuffer(cl_command_queue    command_queue,
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer,
                    size_t              src_offset,
                    size_t              dst_offset,
                    size_t              cb,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::CopyBufferEvent *command = new Coal::CopyBufferEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)src_buffer,
        (Coal::MemObject *)dst_buffer,
        src_offset, dst_offset, cb,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueReadImage(cl_command_queue     command_queue,
                   cl_mem               image,
                   cl_bool              blocking_read,
                   const size_t *       origin,
                   const size_t *       region,
                   size_t               row_pitch,
                   size_t               slice_pitch,
                   void *               ptr,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    if (!image || (image->type() != Coal::MemObject::Image2D &&
        image->type() != Coal::MemObject::Image3D))
        return CL_INVALID_MEM_OBJECT;

    Coal::ReadImageEvent *command = new Coal::ReadImageEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Image2D *)image,
        origin, region, row_pitch, slice_pitch, (void *)ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, blocking_read);
}

cl_int
clEnqueueWriteImage(cl_command_queue    command_queue,
                    cl_mem              image,
                    cl_bool             blocking_write,
                    const size_t *      origin,
                    const size_t *      region,
                    size_t              row_pitch,
                    size_t              slice_pitch,
                    const void *        ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::WriteImageEvent *command = new Coal::WriteImageEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Image2D *)image,
        origin, region, row_pitch, slice_pitch, (void *)ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, blocking_write);
}

cl_int
clEnqueueCopyImage(cl_command_queue     command_queue,
                   cl_mem               src_image,
                   cl_mem               dst_image,
                   const size_t *       src_origin,
                   const size_t *       dst_origin,
                   const size_t *       region,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::CopyImageEvent *command = new Coal::CopyImageEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Image2D *)src_image, (Coal::Image2D *)dst_image,
        src_origin, dst_origin, region,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                           cl_mem           src_image,
                           cl_mem           dst_buffer,
                           const size_t *   src_origin,
                           const size_t *   region,
                           size_t           dst_offset,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::CopyImageToBufferEvent *command = new Coal::CopyImageToBufferEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Image2D *)src_image, (Coal::MemObject *)dst_buffer,
        src_origin, region, dst_offset,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
                           cl_mem           src_buffer,
                           cl_mem           dst_image,
                           size_t           src_offset,
                           const size_t *   dst_origin,
                           const size_t *   region,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::CopyBufferToImageEvent *command = new Coal::CopyBufferToImageEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)src_buffer, (Coal::Image2D *)dst_image,
        src_offset, dst_origin, region,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

void *
clEnqueueMapBuffer(cl_command_queue command_queue,
                   cl_mem           buffer,
                   cl_bool          blocking_map,
                   cl_map_flags     map_flags,
                   size_t           offset,
                   size_t           cb,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event,
                   cl_int *         errcode_ret)
{
    cl_int dummy_errcode;

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    *errcode_ret = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
    {
        *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        return 0;
    }

    Coal::MapBufferEvent *command = new Coal::MapBufferEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)buffer,
        offset, cb, map_flags,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, errcode_ret
    );

    if (*errcode_ret != CL_SUCCESS)
    {
        delete command;
        return 0;
    }

    // We need command to be valid after queueEvent, so don't let the command
    // queue handle it like a fire-and-forget event. Fixes a crash when event
    // is NULL : the event gets deleted by clReleaseEvent called from
    // CPUDevice's worker() and we then try to read it in command->ptr();
    command->reference();

    *errcode_ret = queueEvent(command_queue, command, event, blocking_map);

    if (*errcode_ret != CL_SUCCESS)
        return 0;
    else
    {
        void *rs = command->ptr();

        clReleaseEvent((cl_event)command);

        return rs;
    }
}

void *
clEnqueueMapImage(cl_command_queue  command_queue,
                  cl_mem            image,
                  cl_bool           blocking_map,
                  cl_map_flags      map_flags,
                  const size_t *    origin,
                  const size_t *    region,
                  size_t *          image_row_pitch,
                  size_t *          image_slice_pitch,
                  cl_uint           num_events_in_wait_list,
                  const cl_event *  event_wait_list,
                  cl_event *        event,
                  cl_int *          errcode_ret)
{
    cl_int rs;

    if (!errcode_ret)
        errcode_ret = &rs;

    *errcode_ret = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
    {
        *errcode_ret = CL_INVALID_COMMAND_QUEUE;
        return 0;
    }

    Coal::MapImageEvent *command = new Coal::MapImageEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Image2D *)image,
        map_flags, origin, region,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, errcode_ret
    );

    if (*errcode_ret != CL_SUCCESS)
    {
        delete command;
        return 0;
    }

    if (!image_row_pitch ||
        (image->type() == Coal::MemObject::Image3D && !image_slice_pitch))
    {
        *errcode_ret = CL_INVALID_VALUE;
        delete command;
        return 0;
    }

    command->reference(); // See clEnqueueMapImage for explanation.
    *errcode_ret = queueEvent(command_queue, command, event, blocking_map);

    if (*errcode_ret != CL_SUCCESS)
    {
        delete command;
        return 0;
    }
    else
    {
        *image_row_pitch = command->row_pitch();

        if (image_slice_pitch)
            *image_slice_pitch = command->slice_pitch();

        void *rs = command->ptr();

        clReleaseEvent((cl_event)command);

        return rs;
    }
}

cl_int
clEnqueueUnmapMemObject(cl_command_queue command_queue,
                        cl_mem           memobj,
                        void *           mapped_ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event *  event_wait_list,
                        cl_event *        event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    Coal::UnmapBufferEvent *command = new Coal::UnmapBufferEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::MemObject *)memobj,
        mapped_ptr,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    Coal::KernelEvent *command = new Coal::KernelEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Kernel *)kernel,
        work_dim, global_work_offset, global_work_size, local_work_size,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueTask(cl_command_queue  command_queue,
              cl_kernel         kernel,
              cl_uint           num_events_in_wait_list,
              const cl_event *  event_wait_list,
              cl_event *        event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
    {
        return CL_INVALID_COMMAND_QUEUE;
    }

    Coal::TaskEvent *command = new Coal::TaskEvent(
        (Coal::CommandQueue *)command_queue,
        (Coal::Kernel *)kernel,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueNativeKernel(cl_command_queue  command_queue,
                      void (*user_func)(void *),
                      void *            args,
                      size_t            cb_args,
                      cl_uint           num_mem_objects,
                      const cl_mem *    mem_list,
                      const void **     args_mem_loc,
                      cl_uint           num_events_in_wait_list,
                      const cl_event *  event_wait_list,
                      cl_event *        event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::NativeKernelEvent *command = new Coal::NativeKernelEvent(
        (Coal::CommandQueue *)command_queue,
        user_func, args, cb_args, num_mem_objects,
        (const Coal::MemObject **)mem_list, args_mem_loc,
        num_events_in_wait_list, (const Coal::Event **)event_wait_list, &rs
    );

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueMarker(cl_command_queue    command_queue,
                cl_event *          event)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    if (!event)
        return CL_INVALID_VALUE;

    // Get the events in command_queue
    unsigned int count;
    Coal::Event **events = command_queue->events(count);

    Coal::MarkerEvent *command = new Coal::MarkerEvent(
        (Coal::CommandQueue *)command_queue,
        count, (const Coal::Event **)events, &rs);

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    // Free events, they were memcpyed by Coal::Event
    for (unsigned int i=0; i<count; ++i)
    {
        events[i]->dereference();
    }

    std::free(events);

    return queueEvent(command_queue, command, event, false);
}

cl_int
clEnqueueWaitForEvents(cl_command_queue command_queue,
                       cl_uint          num_events,
                       const cl_event * event_list)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::WaitForEventsEvent *command = new Coal::WaitForEventsEvent(
        (Coal::CommandQueue *)command_queue,
        num_events, (const Coal::Event **)event_list, &rs);

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, 0, false);
}

cl_int
clEnqueueBarrier(cl_command_queue command_queue)
{
    cl_int rs = CL_SUCCESS;

    if (!command_queue->isA(Coal::Object::T_CommandQueue))
        return CL_INVALID_COMMAND_QUEUE;

    Coal::BarrierEvent *command = new Coal::BarrierEvent(
        (Coal::CommandQueue *)command_queue, &rs);

    if (rs != CL_SUCCESS)
    {
        delete command;
        return rs;
    }

    return queueEvent(command_queue, command, 0, false);
}
