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

#include <cstring>
#include <cstdlib>
#include <iostream>

#include "test_commandqueue.h"
#include "CL/cl.h"

#include <unistd.h>

START_TEST (test_create_command_queue)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(0, device, 0, &result);
    fail_if(
        result != CL_INVALID_CONTEXT,
        "context must be valid"
    );

    queue = clCreateCommandQueue(ctx, 0, 0, &result);
    fail_if(
        result != CL_INVALID_DEVICE,
        "device cannot be NULL"
    );

    // HACK : may crash if implementation changes, even if the test should pass.
    queue = clCreateCommandQueue(ctx, (cl_device_id)1337, 0, &result);
    fail_if(
        result != CL_INVALID_DEVICE,
        "1337 is not a device associated to the context"
    );

    queue = clCreateCommandQueue(ctx, device, 1337, &result);
    fail_if(
        result != CL_INVALID_VALUE,
        "1337 is not a valid value for properties"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_get_command_queue_info)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;

    union {
        cl_context ctx;
        cl_device_id device;
        cl_uint refcount;
        cl_command_queue_properties properties;
    } info;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    result = clGetCommandQueueInfo(queue, CL_QUEUE_CONTEXT, sizeof(cl_context),
                                   (void *)&info, 0);
    fail_if(
        result != CL_SUCCESS || info.ctx != ctx,
        "the queue doesn't retain its context"
    );

    result = clGetCommandQueueInfo(queue, CL_QUEUE_DEVICE, sizeof(cl_device_id),
                                   (void *)&info, 0);
    fail_if(
        result != CL_SUCCESS || info.device != device,
        "the queue doesn't retain its device"
    );

    result = clGetCommandQueueInfo(queue, CL_QUEUE_REFERENCE_COUNT, sizeof(cl_uint),
                                   (void *)&info, 0);
    fail_if(
        result != CL_SUCCESS || info.refcount != 1,
        "the queue must have a refcount of 1 when it's created"
    );

    result = clGetCommandQueueInfo(queue, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties),
                                   (void *)&info, 0);
    fail_if(
        result != CL_SUCCESS || info.properties != 0,
        "we gave no properties to the command queue"
    );

    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_object_tree)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;
    cl_uint refcount;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    result = clGetContextInfo(ctx, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint),
                              (void *)&refcount, 0);
    fail_if(
        result != CL_SUCCESS || refcount != 2,
        "the queue must increment the refcount of its context"
    );

    clReleaseCommandQueue(queue);

    result = clGetContextInfo(ctx, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint),
                              (void *)&refcount, 0);
    fail_if(
        result != CL_SUCCESS || refcount != 1,
        "the queue must decrement the refcount of its context when it's destroyed"
    );

    clReleaseContext(ctx);
}
END_TEST

static void event_notify(cl_event event, cl_int exec_status, void *user_data)
{
    unsigned char *good = (unsigned char *)user_data;

    *good = 1;
}

START_TEST (test_events)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;
    cl_event user_event, write_event;
    cl_mem buf;

    char s[] = "Original content";
    unsigned char good = 0;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, CL_QUEUE_PROFILING_ENABLE, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    user_event = clCreateUserEvent(0, &result);
    fail_if(
        result != CL_INVALID_CONTEXT,
        "0 is not a valid context"
    );

    user_event = clCreateUserEvent(ctx, &result);
    fail_if(
        result != CL_SUCCESS || user_event == 0,
        "cannot create an user event"
    );

    buf = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                         sizeof(s), s, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create a valid CL_MEM_USE_HOST_PTR read-write buffer"
    );

    // Queue a write buffer
    result = clEnqueueWriteBuffer(queue, buf, 0, 0, 8, "Modified", 1,
                                  &user_event, &write_event);
    fail_if(
        result != CL_SUCCESS,
        "cannot enqueue an asynchronous write buffer command"
    );

    result = clSetEventCallback(write_event, CL_SUBMITTED, &event_notify, &good);
    fail_if(
        result != CL_INVALID_VALUE,
        "callback_type must be CL_COMPLETE in OpenCL 1.1"
    );

    result = clSetEventCallback(write_event, CL_COMPLETE, &event_notify, &good);
    fail_if(
        result != CL_COMPLETE,
        "cannot register an event callback"
    );

    sleep(1); // Let the worker threads a chance to do faulty things

    fail_if(
        good != 0 || strncmp(s, "Original content", sizeof(s)),
        "at this time, nothing can have happened, the user event isn't complete"
    );

    // Now we can execute everything
    result = clSetUserEventStatus(write_event, CL_COMPLETE);
    fail_if(
        result != CL_INVALID_EVENT,
        "write_event is not an user event"
    );

    result = clSetUserEventStatus(user_event, CL_SUBMITTED);
    fail_if(
        result != CL_INVALID_VALUE,
        "the execution status must be CL_COMPLETE"
    );

    result = clSetUserEventStatus(user_event, CL_COMPLETE);
    fail_if(
        result != CL_SUCCESS,
        "cannot set the user event as completed"
    );

    // And wait (TODO: More careful checks of this function)
    result = clWaitForEvents(1, &write_event);
    fail_if(
        result != CL_SUCCESS,
        "cannot wait for events"
    );

    // Checks that all went good
    fail_if(
        good != 1,
        "the callback function must be called when an event is completed"
    );
    fail_if(
        strncmp(s, "Modified content", sizeof(s)),
        "the buffer must contain \"Modified content\""
    );

    result = clSetUserEventStatus(user_event, CL_COMPLETE);
    fail_if(
        result != CL_INVALID_OPERATION,
        "we cannot call clSetUserEventStatus two times for an event"
    );

    // Queue a map buffer
    char *data;

    data = (char *) clEnqueueMapBuffer(queue, buf, 1, CL_MAP_READ, 0, sizeof(s),
                                       0, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || !data || strncmp(data, s, sizeof(s)),
        "unable to map a buffer containing what the buffer contains"
    );

    result = clEnqueueUnmapMemObject(queue, buf, data, 0, 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to unmap a mapped buffer"
    );

    // Get timing information about the event
    cl_ulong timing_queued, timing_submit, timing_start, timing_end;

    result = clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_QUEUED,
                                     sizeof(cl_ulong), &timing_queued, 0);
    result |= clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_SUBMIT,
                                     sizeof(cl_ulong), &timing_submit, 0);
    result |= clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_START,
                                     sizeof(cl_ulong), &timing_start, 0);
    result |= clGetEventProfilingInfo(write_event, CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong), &timing_end, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get timing information about a profiling-enabled event"
    );
    fail_if(
        !(timing_queued <= timing_submit &&
          timing_submit <= timing_start &&
          timing_start <= timing_end),
        "something went wrong with the timings : they are unordered"
    );

    clReleaseEvent(write_event);
    clReleaseEvent(user_event);
    clReleaseMemObject(buf);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_read_write_rect)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;
    cl_mem buf, buf_part;

    // Grid xyz = (5 x 7 x 2)
    unsigned char grid[70] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        1, 2, 2, 2, 1,
        1, 2, 3, 2, 1,
        1, 2, 2, 2, 1,
        0, 1, 1, 1, 0,
        0, 0, 0, 0, 0,

        0, 0, 1, 0, 0,
        0, 0, 2, 0, 0,
        0, 1, 3, 1, 0,
        0, 2, 3, 2, 0,
        1, 3, 3, 3, 1,
        2, 3, 3, 3, 2,
        3, 3, 3, 3, 3
    };

    // Middle of the "image" : 3 x 3 x 2 centered at (3, 3)
    unsigned char part[18] = {
        2, 2, 2,
        2, 3, 2,
        2, 2, 2,

        1, 3, 1,
        2, 3, 2,
        3, 3, 3
    };

    unsigned char buffer[70], buffer_part[18];
    size_t host_origin[3] = {0, 0, 0};
    size_t buf_origin[3] = {0, 0, 0};
    size_t region[3] = {5, 7, 2};

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    buf = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                         sizeof(buffer), buffer, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create a valid CL_MEM_USE_HOST_PTR read-write buffer"
    );

    buf_part = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                              sizeof(buffer_part), buffer_part, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create a buffer for the part that will be read"
    );

    // Write grid into buffer
    result = clEnqueueWriteBufferRect(queue, buf, 1, buf_origin, host_origin,
                                      region, 0, 0, 0, 0, grid, 0, 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot enqueue a blocking write buffer rect event with pitches guessed"
    );
    fail_if(
        std::memcmp(buffer, grid, sizeof(buffer)) != 0,
        "buffer doesn't contain the data"
    );

    // Read it back into a temporary region
    buf_origin[0] = 1;
    buf_origin[1] = 2;
    buf_origin[2] = 0;
    // host_origin remains (0, 0, 0)
    region[0] = 3;
    region[1] = 3;
    region[2] = 2;

    result = clEnqueueReadBufferRect(queue, buf, 1, buf_origin, host_origin,
                                     region, 5, 5*7, 0, 0, buffer_part, 0, 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to queue a blocking write buffer rect event with host pitches guessed"
    );
    fail_if(
        std::memcmp(buffer_part, part, sizeof(part)) != 0,
        "the part of the buffer was not correctly read"
    );

    // Clear the temporary region and re-read into it using buf_part
    std::memset(buffer_part, 0, sizeof(buffer_part));
    cl_event event;

    result = clEnqueueCopyBufferRect(queue, buf, buf_part, buf_origin,
                                     host_origin, region, 5, 5*7, 0, 0, 0, 0, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to queue a copy buffer rect event"
    );

    result = clWaitForEvents(1, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to wait for the event"
    );

    fail_if(
        std::memcmp(buffer_part, part, sizeof(part)) != 0,
        "the part of the buffer was not correctly read using a buffer"
    );

    clReleaseEvent(event);
    clReleaseMemObject(buf_part);
    clReleaseMemObject(buf);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_copy_buffer)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;
    cl_mem src_buf, dst_buf;
    cl_event event;

    char src[] = "This is the data.";
    char dst[] = "Overwrite this...";

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    src_buf = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                             sizeof(src), src, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create the source buffer"
    );

    dst_buf = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                             sizeof(dst), dst, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create the destination buffer"
    );

    result = clEnqueueCopyBuffer(queue, src_buf, dst_buf, 0, 0, sizeof(src),
                                 0, 0, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to queue a copy buffer event"
    );

    result = clWaitForEvents(1, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to wait for the event"
    );

    fail_if(
        std::memcmp(src, dst, sizeof(src)) != 0,
        "the buffer wasn't copied"
    );

    clReleaseEvent(event);
    clReleaseMemObject(src_buf);
    clReleaseMemObject(dst_buf);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_read_write_image)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_mem image2d, part2d;
    cl_int result;

    unsigned char image2d_data_24bpp[3*3*4] = {
        255, 0, 0, 0,       0, 255, 0, 0,       128, 128, 128, 0,
        0, 0, 255, 0,       255, 255, 0, 0,     0, 128, 0, 0,
        255, 128, 0, 0,     128, 0, 255, 0,     0, 0, 0, 0
    };

    unsigned char image2d_part_24bpp[2*2*4] = {
        255, 0, 0, 0,       0, 255, 0, 0,
        0, 0, 255, 0,       255, 255, 0, 0
    };

    unsigned char image2d_buffer[3*3*4];
    unsigned char image2d_part[2*2*4];

    cl_image_format fmt;

    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RGBA;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {3, 3, 1};

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    image2d = clCreateImage2D(ctx, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &fmt,
                              3, 3, 0, image2d_buffer, &result);
    fail_if(
        result != CL_SUCCESS || image2d == 0,
        "cannot create a valid 3x3 image2D"
    );

    part2d = clCreateImage2D(ctx, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &fmt,
                             2, 2, 0, image2d_part, &result);
    fail_if(
        result != CL_SUCCESS || image2d == 0,
        "cannot create a valid 2x2 image2D"
    );

    // Write data in buffer
    result = clEnqueueWriteImage(queue, image2d, 1, origin, region, 0, 0,
                                 image2d_data_24bpp, 0, 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot enqueue a blocking write image event"
    );

    // Read it back
    region[0] = 2;
    region[1] = 2;

    result = clEnqueueReadImage(queue, image2d, 1, origin, region, 0, 0,
                                image2d_part, 0, 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot enqueue a blocking read image event"
    );

    // Compare
    fail_if(
        std::memcmp(image2d_part, image2d_part_24bpp, sizeof(image2d_part)) != 0,
        "reading and writing images doesn't produce the correct result"
    );

    // Read it back using a buffer
    cl_event event;
    std::memset(image2d_part, 0, sizeof(image2d_part));

    result = clEnqueueCopyImage(queue, image2d, part2d, origin, origin,
                                region, 0, 0, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue a copy image event"
    );

    result = clWaitForEvents(1, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to wait for events"
    );

    // Compare
    fail_if(
        std::memcmp(image2d_part, image2d_part_24bpp, sizeof(image2d_part)) != 0,
        "copying images doesn't produce the correct result"
    );

    clReleaseEvent(event);
    clReleaseMemObject(part2d);
    clReleaseMemObject(image2d);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_copy_image_buffer)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_mem image, buffer;
    cl_int result;
    cl_event event;

    unsigned char image_buffer[3*3*4] = {
        255, 0, 0, 0,       0, 255, 0, 0,       0, 0, 255, 0,
        128, 0, 0, 0,       0, 128, 0, 0,       0, 0, 128, 0,
        64, 0, 0, 0,        0, 64, 0, 0,        0, 0, 64, 0
    };

    // Square that will be put in image_buffer at (1, 0)
    unsigned char buffer_buffer[2*2*4+1] = {
        33, // Oh, a padding !
        255, 255, 255, 0,   255, 0, 255, 0,
        0, 255, 255, 0,     255, 255, 0, 0
    };

    // What we must get once re-reading 2x2 rect at (1, 1)
    unsigned char correct_data[2*2*4] = {
        0, 255, 255, 0,     255, 255, 0, 0,
        0, 64, 0, 0,        0, 0, 64, 0
    };

    cl_image_format fmt;

    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RGBA;

    size_t origin[3] = {1, 0, 0};
    size_t region[3] = {2, 2, 1};

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    image = clCreateImage2D(ctx, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &fmt,
                            3, 3, 0, image_buffer, &result);
    fail_if(
        result != CL_SUCCESS,
        "unable to create a 3x3 bgra image"
    );

    buffer = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                            sizeof(buffer_buffer), buffer_buffer, &result);
    fail_if(
        result != CL_SUCCESS,
        "unable to create a buffer object"
    );

    // Write buffer in image
    result = clEnqueueCopyBufferToImage(queue, buffer, image, 1, origin, region,
                                        0, 0, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to queue a copy buffer to image event, buffer offset 1, image 2x2 @ (1, 0)"
    );

    result = clWaitForEvents(1, &event);
    fail_if(
        result != CL_SUCCESS,
        "cannot wait for event"
    );

    clReleaseEvent(event);

    // Read it back into buffer, again with an offset
    origin[1] = 1;
    result = clEnqueueCopyImageToBuffer(queue, image, buffer, origin, region, 1,
                                        0, 0, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to queue a copy image to buffer event, buffer offset 1, image 2x2 @ (1, 1)"
    );

    result = clWaitForEvents(1, &event);
    fail_if(
        result != CL_SUCCESS,
        "cannot wait for event"
    );

    fail_if(
        std::memcmp(buffer_buffer + 1, correct_data, sizeof(correct_data)) != 0,
        "copying data around isn't working the expected way"
    );

    // Map the image and check pointers
    unsigned char *mapped;
    size_t row_pitch;

    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    mapped = (unsigned char *)clEnqueueMapImage(queue, image, 1, CL_MAP_READ,
                                                origin, region, &row_pitch, 0, 0,
                                                0, 0, &result);
    fail_if(
        result != CL_SUCCESS,
        "unable to map an image"
    );
    fail_if(
        mapped != image_buffer,
        "mapped aread doesn't match host ptr"
    );

    clReleaseEvent(event);
    clReleaseMemObject(image);
    clReleaseMemObject(buffer);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_misc_events)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;
    cl_event uevent1, uevent2, marker1, marker2;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the default device"
    );

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    fail_if(
        result != CL_SUCCESS || ctx == 0,
        "unable to create a valid context"
    );

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    /*
     * This test will build a command queue blocked by an user event. The events
     * will be in this order :
     *
     * -: UserEvent1
     * 0: WaitForEvents1 (wait=UserEvent1)
     * 1: Marker1
     * -: UserEvent2
     * 2: WaitForEvents2 (wait=UserEvent2)
     * 3: Barrier
     * 4: Marker2 (to check the barrier worked)
     *
     * When the command queue is built, we :
     *  - Check that Marker1 is Queued (WaitForEvents waits)
     *  - Set UserEvent1 to Complete
     *  - Check that Marker1 is Complete (WaitForEvents stopped to wait)
     *  - Check that Marker2 is Queued (Barrier is there)
     *  - Set UserEvent2 to Complete
     *  - Check that Marker2 is Complete (no more barrier)
     */
    uevent1 = clCreateUserEvent(ctx, &result);
    fail_if(
        result != CL_SUCCESS,
        "unable to create UserEvent1"
    );

    uevent2 = clCreateUserEvent(ctx, &result);
    fail_if(
        result != CL_SUCCESS,
        "unable to create UserEvent2"
    );

    result = clEnqueueWaitForEvents(queue, 1, &uevent1);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue WaitForEvents(UserEvent1)"
    );

    result = clEnqueueMarker(queue, &marker1);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue Marker1"
    );

    result = clEnqueueWaitForEvents(queue, 1, &uevent2);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue WaitForEvents(UserEvent2)"
    );

    result = clEnqueueBarrier(queue);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue Barrier"
    );

    result = clEnqueueMarker(queue, &marker2);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue Marker2"
    );

    // Now the checks
    cl_int status;

    result = clGetEventInfo(marker1, CL_EVENT_COMMAND_EXECUTION_STATUS,
                            sizeof(cl_int), &status, 0);
    fail_if(
        result != CL_SUCCESS || status != CL_QUEUED,
        "Marker1 must be Queued"
    );

    result = clSetUserEventStatus(uevent1, CL_COMPLETE);
    fail_if(
        result != CL_SUCCESS,
        "unable to set UserEvent1 to Complete"
    );

    result = clGetEventInfo(marker1, CL_EVENT_COMMAND_EXECUTION_STATUS,
                            sizeof(cl_int), &status, 0);
    fail_if(
        result != CL_SUCCESS || status != CL_COMPLETE,
        "Marker1 must be Complete"
    );

    result = clGetEventInfo(marker2, CL_EVENT_COMMAND_EXECUTION_STATUS,
                            sizeof(cl_int), &status, 0);
    fail_if(
        result != CL_SUCCESS || status != CL_QUEUED,
        "Marker2 must be Queued"
    );

    result = clSetUserEventStatus(uevent2, CL_COMPLETE);
    fail_if(
        result != CL_SUCCESS,
        "unable to set UserEvent2 to Complete"
    );

    result = clGetEventInfo(marker2, CL_EVENT_COMMAND_EXECUTION_STATUS,
                            sizeof(cl_int), &status, 0);
    fail_if(
        result != CL_SUCCESS || status != CL_COMPLETE,
        "Marker2 must be Complete"
    );

    clFinish(queue);

    clReleaseEvent(uevent1);
    clReleaseEvent(uevent2);
    clReleaseEvent(marker1);
    clReleaseEvent(marker2);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

TCase *cl_commandqueue_tcase_create(void)
{
    TCase *tc = NULL;
    tc = tcase_create("commandqueue");
    tcase_add_test(tc, test_create_command_queue);
    tcase_add_test(tc, test_get_command_queue_info);
    tcase_add_test(tc, test_object_tree);
    tcase_add_test(tc, test_events);
    tcase_add_test(tc, test_read_write_rect);
    tcase_add_test(tc, test_copy_buffer);
    tcase_add_test(tc, test_read_write_image);
    tcase_add_test(tc, test_copy_image_buffer);
    tcase_add_test(tc, test_misc_events);
    return tc;
}
