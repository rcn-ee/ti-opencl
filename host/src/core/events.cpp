/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * \file events.cpp
 * \brief Events inheriting \c Coal::Event
 */

#include "events.h"
#include "commandqueue.h"
#include "memobject.h"
#include "kernel.h"
#include "deviceinterface.h"
#include "context.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Coal;

/*
 * Read/Write buffers
 */

BufferEvent::BufferEvent(CommandQueue *parent,
                         MemObject *buffer,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_int *errcode_ret)
: Event(parent, Queued, num_events_in_wait_list, event_wait_list, errcode_ret),
  p_buffer(buffer)
{
    clRetainMemObject(desc(p_buffer));

    if (*errcode_ret != CL_SUCCESS) return;

    // Correct buffer
    if (!buffer)
    {
        *errcode_ret = CL_INVALID_MEM_OBJECT;
        return;
    }

    // Buffer's context must match the CommandQueue one
    cl_context d_ctx = 0;
    *errcode_ret = parent->info(CL_QUEUE_CONTEXT, sizeof(cl_context), &d_ctx, 0);

    if (*errcode_ret != CL_SUCCESS) return;

    if ((Context *)buffer->parent() != pobj(d_ctx))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return;
    }

    // Alignment of SubBuffers
    cl_device_id d_device = 0;
    *errcode_ret = parent->info(CL_QUEUE_DEVICE, sizeof(cl_device_id), &d_device, 0);

    if (*errcode_ret != CL_SUCCESS)
        return;

    auto device = pobj(d_device);
    if (!isSubBufferAligned(buffer, device))
    {
        *errcode_ret = CL_MISALIGNED_SUB_BUFFER_OFFSET;
        return;
    }

    // Allocate the buffer for the device
    if (!buffer->allocate(device))
    {
        *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        return;
    }
}

BufferEvent::~BufferEvent()
{
    clReleaseMemObject(desc(p_buffer));
}

MemObject *BufferEvent::buffer() const
{
    return p_buffer;
}

bool BufferEvent::isSubBufferAligned(const MemObject *buffer,
                                     const DeviceInterface *device)
{
    cl_uint align;
    cl_int rs;

    if (buffer == nullptr || buffer->type() != MemObject::SubBuffer)
        return true;

    rs = device->info(CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_int),
                      &align, 0);

    if (rs != CL_SUCCESS)
        return false;

    size_t mask = 0;
    if (align != 0) mask = (align >> 3) - 1;  // align in bits, offset in bytes

    if (((SubBuffer *)buffer)->offset() & mask)
        return false;

    return true;
}

ReadWriteBufferEvent::ReadWriteBufferEvent(CommandQueue *parent,
                                           MemObject *buffer,
                                           size_t offset,
                                           size_t cb,
                                           void *ptr,
                                           cl_uint num_events_in_wait_list,
                                           const cl_event *event_wait_list,
                                           cl_int *errcode_ret)
: BufferEvent(parent, buffer, num_events_in_wait_list, event_wait_list, errcode_ret),
  p_offset(offset), p_cb(cb), p_ptr(ptr)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Check for out-of-bounds reads
    if (!ptr)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (offset + cb > buffer->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

size_t ReadWriteBufferEvent::offset() const
{
    return p_offset;
}

size_t ReadWriteBufferEvent::cb() const
{
    return p_cb;
}

void *ReadWriteBufferEvent::ptr() const
{
    return p_ptr;
}

ReadBufferEvent::ReadBufferEvent(CommandQueue *parent,
                                 MemObject *buffer,
                                 size_t offset,
                                 size_t cb,
                                 void *ptr,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 cl_int *errcode_ret)
: ReadWriteBufferEvent(parent, buffer, offset, cb, ptr, num_events_in_wait_list,
                       event_wait_list, errcode_ret)
{}

Event::Type ReadBufferEvent::type() const
{
    return Event::ReadBuffer;
}

WriteBufferEvent::WriteBufferEvent(CommandQueue *parent,
                                   MemObject *buffer,
                                   size_t offset,
                                   size_t cb,
                                   void *ptr,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   cl_int *errcode_ret)
: ReadWriteBufferEvent(parent, buffer, offset, cb, ptr, num_events_in_wait_list,
                       event_wait_list, errcode_ret)
{}

Event::Type WriteBufferEvent::type() const
{
    return Event::WriteBuffer;
}

FillBufferEvent::FillBufferEvent(CommandQueue *parent,
                                 MemObject *buffer,
                                 const void *pattern,
                                 size_t pattern_size,
                                 size_t offset,
                                 size_t cb,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 cl_int *errcode_ret)
: BufferEvent(parent, buffer, num_events_in_wait_list, event_wait_list,
              errcode_ret),
  p_pattern(nullptr), p_pattern_size(pattern_size), p_offset(offset), p_cb(cb)
{
    if (*errcode_ret != CL_SUCCESS) return;

    if (offset + cb > buffer->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (!pattern || pattern_size == 0 ||
        (pattern_size != 1 && pattern_size != 2 && pattern_size != 4 &&
         pattern_size != 8 && pattern_size != 16 && pattern_size != 32 &&
         pattern_size != 64 && pattern_size != 128))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (offset % pattern_size != 0 || cb % pattern_size != 0)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    p_pattern = std::malloc(pattern_size);
    if (p_pattern == nullptr)
    {
        *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return;
    }
    memcpy(p_pattern, pattern, pattern_size);
}

FillBufferEvent::~FillBufferEvent()
{
    if (p_pattern)  std::free(p_pattern);
}

Event::Type FillBufferEvent::type() const
{
    return Event::FillBuffer;
}

void* FillBufferEvent::pattern() const
{
    return p_pattern;
}

size_t FillBufferEvent::pattern_size() const
{
    return p_pattern_size;
}

size_t FillBufferEvent::offset() const
{
    return p_offset;
}

size_t FillBufferEvent::cb() const
{
    return p_cb;
}

MapBufferEvent::MapBufferEvent(CommandQueue *parent,
                               MemObject *buffer,
                               size_t offset,
                               size_t cb,
                               cl_map_flags map_flags,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_int *errcode_ret)
: BufferEvent(parent, buffer, num_events_in_wait_list, event_wait_list, errcode_ret),
  p_offset(offset), p_cb(cb), p_map_flags(map_flags)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Check flags
    if (map_flags & ~(CL_MAP_READ | CL_MAP_WRITE))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check for out-of-bounds values
    if (offset + cb > buffer->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // check conflict between map flags and buffer flags
    cl_mem_flags buf_flags = buffer->flags();
    if (   ((map_flags & CL_MAP_READ)
            && (buf_flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)))
        || ((map_flags & CL_MAP_WRITE)
            && (buf_flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) )
    {
        *errcode_ret = CL_INVALID_OPERATION;
        return;
    }
}

Event::Type MapBufferEvent::type() const
{
    return Event::MapBuffer;
}

size_t MapBufferEvent::offset() const
{
    return p_offset;
}

size_t MapBufferEvent::cb() const
{
    return p_cb;
}

cl_map_flags MapBufferEvent::flags() const
{
    return p_map_flags;
}

void *MapBufferEvent::ptr() const
{
    return p_ptr;
}

void MapBufferEvent::setPtr(void *ptr)
{
    p_ptr = ptr;
}

MapImageEvent::MapImageEvent(CommandQueue *parent,
                             Image2D *image,
                             cl_map_flags map_flags,
                             const size_t origin[3],
                             const size_t region[3],
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_int *errcode_ret)
: BufferEvent (parent, image, num_events_in_wait_list, event_wait_list, errcode_ret)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Check flags
    if (map_flags & ~(CL_MAP_READ | CL_MAP_WRITE))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Copy the vectors
    if (origin)
        std::memcpy(&p_origin, origin, 3 * sizeof(size_t));
    else
        std::memset(&p_origin, 0, 3 * sizeof(size_t));

    for (unsigned int i=0; i<3; ++i)
    {
        if (!region[i])
        {
            *errcode_ret = CL_INVALID_VALUE;
            return;
        }

        p_region[i] = region[i];
    }

    // Multiply the elements (for images)
    p_region[0] *= image->pixel_size();
    p_origin[0] *= image->pixel_size();

    // Check for overflow
    if (image->type() == MemObject::Image2D &&
        ((origin && origin[2] != 0) || region[2] != 1))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check for out-of-bounds
    // cross-row and cross-slice transfers on Image should be allowed,
    // as seen by OpenCV use cases, and as SPEC does not prohibit such cases
    if ((p_origin[0] + p_region[0]) > image->size() ||
        (p_origin[1] + p_region[1]) * image->row_pitch() > image->size() ||
        (p_origin[2] + p_region[2]) * image->slice_pitch() > image->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

Event::Type MapImageEvent::type() const
{
    return Event::MapImage;
}


cl_map_flags MapImageEvent::flags() const
{
    return p_map_flags;
}

size_t MapImageEvent::origin (unsigned int index) const
{
    return p_origin[index];
}

size_t MapImageEvent::region (unsigned int index) const
{
    return p_region[index];
}

size_t MapImageEvent::row_pitch() const
{
    return p_row_pitch;
}

size_t MapImageEvent::slice_pitch() const
{
    return p_slice_pitch;
}

void *MapImageEvent::ptr() const
{
    return p_ptr;
}

void MapImageEvent::setRowPitch (size_t row_pitch)
{
    p_row_pitch = row_pitch;
}

void MapImageEvent::setSlicePitch (size_t slice_pitch)
{
    p_slice_pitch = slice_pitch;
}

void MapImageEvent::setPtr (void *ptr)
{
    p_ptr = ptr;
}

UnmapBufferEvent::UnmapBufferEvent(CommandQueue *parent,
                                   MemObject *buffer,
                                   void *mapped_addr,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   cl_int *errcode_ret)
: BufferEvent(parent, buffer, num_events_in_wait_list, event_wait_list, errcode_ret),
  p_mapping(mapped_addr)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // TODO: Check that p_mapping is ok (will be done in the drivers)
    if (!mapped_addr)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

Event::Type UnmapBufferEvent::type() const
{
    return Event::UnmapMemObject;
}

void *UnmapBufferEvent::mapping() const
{
    return p_mapping;
}

CopyBufferEvent::CopyBufferEvent(CommandQueue *parent,
                                 MemObject *source,
                                 MemObject *destination,
                                 size_t src_offset,
                                 size_t dst_offset,
                                 size_t cb,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 cl_int *errcode_ret)
: BufferEvent(parent, source, num_events_in_wait_list, event_wait_list,
              errcode_ret), p_destination(destination), p_src_offset(src_offset),
  p_dst_offset(dst_offset), p_cb(cb)
{
    clRetainMemObject(desc(p_destination));

    if (*errcode_ret != CL_SUCCESS) return;

    if (!destination)
    {
        *errcode_ret = CL_INVALID_MEM_OBJECT;
        return;
    }

    // Check for out-of-bounds
    if (src_offset + cb > source->size() ||
        dst_offset + cb > destination->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check for overlap
    if (source == destination)
    {
        if ((src_offset < dst_offset && src_offset + cb > dst_offset) ||
            (dst_offset < src_offset && dst_offset + cb > src_offset))
        {
            *errcode_ret = CL_MEM_COPY_OVERLAP;
            return;
        }
    }

    // Check alignement of destination
    cl_device_id d_device = 0;
    *errcode_ret = parent->info(CL_QUEUE_DEVICE, sizeof(cl_device_id), &d_device, 0);
    if (*errcode_ret != CL_SUCCESS)
        return;

    auto device = pobj(d_device);
    if (!isSubBufferAligned(destination, device))
    {
        *errcode_ret = CL_MISALIGNED_SUB_BUFFER_OFFSET;
        return;
    }

    // Allocate the buffer for the device
    if (!destination->allocate(device))
    {
        *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        return;
    }
}

CopyBufferEvent::~CopyBufferEvent()
{
    clReleaseMemObject(desc(p_destination));
}

MemObject *CopyBufferEvent::source() const
{
    return buffer();
}

MemObject *CopyBufferEvent::destination() const
{
    return p_destination;
}

size_t CopyBufferEvent::src_offset() const
{
    return p_src_offset;
}

size_t CopyBufferEvent::dst_offset() const
{
    return p_dst_offset;
}

size_t CopyBufferEvent::cb() const
{
    return p_cb;
}

Event::Type CopyBufferEvent::type() const
{
    return Event::CopyBuffer;
}

/*
 * Native kernel
 */
NativeKernelEvent::NativeKernelEvent(CommandQueue *parent,
                                     void (*user_func)(void *),
                                     void *args,
                                     size_t cb_args,
                                     cl_uint num_mem_objects,
                                     const cl_mem *mem_list,
                                     const void **args_mem_loc,
                                     cl_uint num_events_in_wait_list,
                                     const cl_event *event_wait_list,
                                     cl_int *errcode_ret)
: Event (parent, Queued, num_events_in_wait_list, event_wait_list, errcode_ret),
  p_user_func((void *)user_func), p_args(0)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Parameters sanity
    if (!user_func)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (!args && (cb_args || num_mem_objects))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (args && !cb_args)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (num_mem_objects && (!mem_list || !args_mem_loc))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (!num_mem_objects && (mem_list || args_mem_loc))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check that the device can execute a native kernel
    cl_device_exec_capabilities caps;
    cl_device_id   d_device = 0;
    *errcode_ret = parent->info(CL_QUEUE_DEVICE, sizeof(cl_device_id), &d_device, 0);

    if (*errcode_ret != CL_SUCCESS)
        return;

    auto device = pobj(d_device);
    *errcode_ret = device->info(CL_DEVICE_EXECUTION_CAPABILITIES,
                                sizeof(cl_device_exec_capabilities), &caps, 0);

    if (*errcode_ret != CL_SUCCESS)
        return;

    if ((caps & CL_EXEC_NATIVE_KERNEL) == 0)
    {
        *errcode_ret = CL_INVALID_OPERATION;
        return;
    }

    // Copy the arguments in a new list
    if (cb_args)
    {
        p_args = std::malloc(cb_args);

        if (!p_args)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return;
        }

        std::memcpy((void *)p_args, (void *)args, cb_args);

        // Replace memory objects with global pointers
        for (cl_uint i=0; i<num_mem_objects; ++i)
        {
            const MemObject *buffer = pobj(mem_list[i]);
            const char *loc = (const char *)args_mem_loc[i];

            if (!buffer)
            {
                *errcode_ret = CL_INVALID_MEM_OBJECT;
                return;
            }

            // We need to do relocation : loc is in args, we need it in p_args
            size_t delta = (char *)p_args - (char *)args;
            loc += delta;

            *(void **)loc = buffer->deviceBuffer(device)->nativeGlobalPointer();
        }
    }
}

NativeKernelEvent::~NativeKernelEvent()
{
    if (p_args)
        std::free((void *)p_args);
}

Event::Type NativeKernelEvent::type() const
{
    return Event::NativeKernel;
}

void *NativeKernelEvent::function() const
{
    return p_user_func;
}

void *NativeKernelEvent::args() const
{
    return p_args;
}

/*
 * Kernel event
 */
KernelEvent::KernelEvent(CommandQueue *parent,
                         Kernel *kernel,
                         cl_uint work_dim,
                         const size_t *global_work_offset,
                         const size_t *global_work_size,
                         const size_t *local_work_size,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_int *errcode_ret)
: Event(parent, Queued, num_events_in_wait_list, event_wait_list, errcode_ret),
  p_work_dim(work_dim), p_kernel(kernel), p_timeout_ms(0)
{
    clRetainKernel(desc(p_kernel));

    if (*errcode_ret != CL_SUCCESS) return;

    *errcode_ret = CL_SUCCESS;

    // Sanity checks
    if (!kernel)
    {
        *errcode_ret = CL_INVALID_KERNEL;
        return;
    }

    // Check that the kernel was built for parent's device.
    cl_device_id d_device = 0;
    cl_context k_ctx, q_ctx;
    size_t max_work_group_size;
    cl_uint max_dims = 0;

    *errcode_ret = parent->info(CL_QUEUE_DEVICE, sizeof(cl_device_id), &d_device, 0);

    if (*errcode_ret != CL_SUCCESS)
        return;

    auto device = pobj(d_device);
    *errcode_ret = parent->info(CL_QUEUE_CONTEXT, sizeof(cl_context), &q_ctx, 0);
    *errcode_ret |= kernel->info(CL_KERNEL_CONTEXT, sizeof(cl_context), &k_ctx, 0);
    *errcode_ret |= device->info(CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t),
                                &max_work_group_size, 0);
    *errcode_ret |= device->info(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t),
                                &max_dims, 0);
    *errcode_ret |= device->info(CL_DEVICE_MAX_WORK_ITEM_SIZES,
                                max_dims * sizeof(size_t), p_max_work_item_sizes, 0);

    if (*errcode_ret != CL_SUCCESS)
        return;

    p_dev_kernel = kernel->deviceDependentKernel(device);

    if (!p_dev_kernel)
    {
        *errcode_ret = CL_INVALID_PROGRAM_EXECUTABLE;
        return;
    }

    // Check that contexts match
    if (k_ctx != q_ctx)
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return;
    }

    // Check args
    if (!kernel->argsSpecified())
    {
        *errcode_ret = CL_INVALID_KERNEL_ARGS;
        return;
    }

    // Check dimension
    if ((work_dim == 0 || work_dim > max_dims) ||
            (max_dims > MAX_WORK_DIMS))
    {
        *errcode_ret = CL_INVALID_WORK_DIMENSION;
        return;
    }

    // Populate work_offset, work_size and local_work_size
    size_t work_group_size = 1;
    cl_uint dims[3];
    kernel->reqdWorkGroupSize(kernel->deviceDependentModule(device), dims);

    uint32_t reqd_x = dims[0];
    uint32_t reqd_y = dims[1];
    uint32_t reqd_z = dims[2];
    bool reqd_any = reqd_x > 0 || reqd_y > 0 || reqd_z > 0;

    if (reqd_any)
    {
        // if __attribute__((reqd_work_group_size(X, Y, Z))) is set and local size not specified
        if (!local_work_size)
        {
            *errcode_ret = CL_INVALID_WORK_GROUP_SIZE;
            return;
        }

        // if __attribute__((reqd_work_group_size(X, Y, Z))) doesn't match
        else 
        {
            if ((                local_work_size[0] != reqd_x) ||
                (work_dim > 1 && local_work_size[1] != reqd_y) ||
                (work_dim > 2 && local_work_size[2] != reqd_z))
            {
                *errcode_ret = CL_INVALID_WORK_GROUP_SIZE;
                return;
            }
        }
    }

    cl_uint i;
    for (i=0; i<work_dim; ++i)
    {
        if (global_work_offset)
        {
            p_global_work_offset[i] = global_work_offset[i];
        }
        else
        {
            p_global_work_offset[i] = 0;
        }

        if (!global_work_size || !global_work_size[i])
        {
            *errcode_ret = CL_INVALID_GLOBAL_WORK_SIZE;
            return;
        }
        p_global_work_size[i] = global_work_size[i];

        if (!local_work_size)
        {
            // Guess the best value according to the device
            p_local_work_size[i] =
                p_dev_kernel->guessWorkGroupSize(work_dim, i, global_work_size[i]);
        }
        else
        {
            // Check divisibility
            if ((global_work_size[i] % local_work_size[i]) != 0)
            {
                *errcode_ret = CL_INVALID_WORK_GROUP_SIZE;
                return;
            }

            // Not too big ?
            if (local_work_size[i] > p_max_work_item_sizes[i])
            {
                *errcode_ret = CL_INVALID_WORK_ITEM_SIZE;
                return;
            }

            p_local_work_size[i] = local_work_size[i];
            work_group_size *= local_work_size[i];
        }
    }
    // initialize missing dimensions
    for (; i < max_dims; i++)
    {
        p_global_work_offset[i]  = 0;
        p_global_work_size[i]    = 1;
        p_local_work_size[i]     = 1;
    }

    // Check we don't ask too much to the device
    if (work_group_size > max_work_group_size)
    {
        *errcode_ret = CL_INVALID_WORK_GROUP_SIZE;
        return;
    }

    // Check arguments (buffer alignment, image size, ...)
    for (unsigned int i=0; i<kernel->numArgs(); ++i)
    {
        const Kernel::Arg &a = kernel->arg(i);

        if (a.kind() == Kernel::Arg::Buffer && a.file() != Kernel::Arg::Local)
        {
            MemObject *buffer = *(MemObject **)(a.value(0));

            if (!BufferEvent::isSubBufferAligned(buffer, device))
            {
                *errcode_ret = CL_MISALIGNED_SUB_BUFFER_OFFSET;
                return;
            }

            clRetainMemObject(desc(buffer));
            p_mem_objects.push_back((MemObject *) buffer);
        }
        else if (a.kind() == Kernel::Arg::Image2D)
        {
            Image2D *image = *(Image2D **)(a.value(0));
            size_t maxWidth, maxHeight;

            *errcode_ret = device->info(CL_DEVICE_IMAGE2D_MAX_WIDTH,
                                        sizeof(size_t), &maxWidth, 0);
            *errcode_ret |= device->info(CL_DEVICE_IMAGE2D_MAX_HEIGHT,
                                         sizeof(size_t), &maxHeight, 0);

            if (*errcode_ret != CL_SUCCESS)
                return;

            if (image->width() > maxWidth || image->height() > maxHeight)
            {
                *errcode_ret = CL_INVALID_IMAGE_SIZE;
                return;
            }

            clRetainMemObject(desc(image));
            p_mem_objects.push_back((MemObject *) image);
        }
        else if (a.kind() == Kernel::Arg::Image3D)
        {
            Image3D *image = *(Image3D **)a.value(0);
            size_t maxWidth, maxHeight, maxDepth;

            *errcode_ret = device->info(CL_DEVICE_IMAGE3D_MAX_WIDTH,
                                        sizeof(size_t), &maxWidth, 0);
            *errcode_ret |= device->info(CL_DEVICE_IMAGE3D_MAX_HEIGHT,
                                         sizeof(size_t), &maxHeight, 0);
            *errcode_ret |= device->info(CL_DEVICE_IMAGE3D_MAX_DEPTH,
                                         sizeof(size_t), &maxDepth, 0);

            if (*errcode_ret != CL_SUCCESS)
                return;

            if (image->width() > maxWidth || image->height() > maxHeight ||
                image->depth() > maxDepth)
            {
                *errcode_ret = CL_INVALID_IMAGE_SIZE;
                return;
            }

            clRetainMemObject(desc(image));
            p_mem_objects.push_back((MemObject *) image);
        }
    }

    // Check if kernel has timeout specified and CommandQueue allows it
    if (kernel->getTimeout() > 0)
    {
        cl_command_queue_properties queue_props;
        *errcode_ret = parent->info(CL_QUEUE_PROPERTIES,
                                    sizeof(cl_command_queue_properties),
                                    &queue_props, 0);
        if (*errcode_ret != CL_SUCCESS)  return;
        if ((queue_props & CL_QUEUE_KERNEL_TIMEOUT_COMPUTE_UNIT_TI) != 0)
            p_timeout_ms = kernel->getTimeout();
    }
}

KernelEvent::~KernelEvent()
{
    for (MemObject *mem_object : p_mem_objects)
        clReleaseMemObject(desc(mem_object));

    clReleaseKernel(desc(p_kernel));
}

cl_uint KernelEvent::work_dim() const
{
    return p_work_dim;
}

size_t KernelEvent::global_work_offset(cl_uint dim) const
{
    return p_global_work_offset[dim];
}

size_t KernelEvent::global_work_size(cl_uint dim) const
{
    return p_global_work_size[dim];
}

size_t KernelEvent::local_work_size(cl_uint dim) const
{
    return p_local_work_size[dim];
}

Kernel *KernelEvent::kernel() const
{
    return p_kernel;
}

DeviceKernel *KernelEvent::deviceKernel() const
{
    return p_dev_kernel;
}

Event::Type KernelEvent::type() const
{
    return Event::NDRangeKernel;
}

static size_t one = 1;

TaskEvent::TaskEvent(CommandQueue *parent,
                     Kernel *kernel,
                     cl_uint num_events_in_wait_list,
                     const cl_event *event_wait_list,
                     cl_int *errcode_ret)
: KernelEvent(parent, kernel, 1, 0, &one, &one, num_events_in_wait_list,
              event_wait_list, errcode_ret)
{
    // TODO: CL_INVALID_WORK_GROUP_SIZE if
    // __attribute__((reqd_work_group_size(X, Y, Z))) != (1, 1, 1)
}

Event::Type TaskEvent::type() const
{
    return Event::TaskKernel;
}

/*
 * User event
 */
UserEvent::UserEvent(Context *context, cl_int *errcode_ret)
: Event(0, Submitted, 0, 0, errcode_ret), p_context(context)
{}

Event::Type UserEvent::type() const
{
    return Event::User;
}

Context *UserEvent::context() const
{
    return p_context;
}

/*
 * ReadWriteBufferRectEvent
 */
ReadWriteCopyBufferRectEvent::ReadWriteCopyBufferRectEvent(CommandQueue *parent,
                                                           MemObject *source,
                                                           const size_t src_origin[3],
                                                           const size_t dst_origin[3],
                                                           const size_t region[3],
                                                           size_t src_row_pitch,
                                                           size_t src_slice_pitch,
                                                           size_t dst_row_pitch,
                                                           size_t dst_slice_pitch,
                                                           unsigned int bytes_per_element,
                                                           cl_uint num_events_in_wait_list,
                                                           const cl_event *event_wait_list,
                                                           cl_int *errcode_ret)
: BufferEvent (parent, source, num_events_in_wait_list, event_wait_list,
               errcode_ret)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Copy the vectors
    if (src_origin)
        std::memcpy(&p_src_origin, src_origin, 3 * sizeof(size_t));
    else
        std::memset(&p_src_origin, 0, 3 * sizeof(size_t));

    if (dst_origin)
        std::memcpy(&p_dst_origin, dst_origin, 3 * sizeof(size_t));
    else
        std::memset(&p_dst_origin, 0, 3 * sizeof(size_t));

    for (unsigned int i=0; i<3; ++i)
    {
        if (!region[i])
        {
            *errcode_ret = CL_INVALID_VALUE;
            return;
        }

        p_region[i] = region[i];
    }

    // Multiply the elements (for images)
    p_region[0] *= bytes_per_element;
    p_src_origin[0] *= bytes_per_element;
    p_dst_origin[0] *= bytes_per_element;

    // Compute the pitches
    p_src_row_pitch = p_region[0];

    if (src_row_pitch)
    {
        if (src_row_pitch < p_src_row_pitch)
        {
            *errcode_ret = CL_INVALID_VALUE;
            return;
        }

        p_src_row_pitch = src_row_pitch;
    }

    p_src_slice_pitch = p_region[1] * p_src_row_pitch;

    if (src_slice_pitch)
    {
        if (src_slice_pitch < p_src_slice_pitch)
        {
            *errcode_ret = CL_INVALID_VALUE;
            return;
        }

        p_src_slice_pitch = src_slice_pitch;
    }

    p_dst_row_pitch = p_region[0];

    if (dst_row_pitch)
    {
        if (dst_row_pitch < p_dst_row_pitch)
        {
            *errcode_ret = CL_INVALID_VALUE;
            return;
        }

        p_dst_row_pitch = dst_row_pitch;
    }

    p_dst_slice_pitch = p_region[1] * p_dst_row_pitch;

    if (dst_slice_pitch)
    {
        if (dst_slice_pitch < p_dst_slice_pitch)
        {
            *errcode_ret = CL_INVALID_VALUE;
            return;
        }

        p_dst_slice_pitch = dst_slice_pitch;
    }
}

size_t ReadWriteCopyBufferRectEvent::src_origin(unsigned int index) const
{
    return p_src_origin[index];
}

size_t ReadWriteCopyBufferRectEvent::dst_origin(unsigned int index) const
{
    return p_dst_origin[index];
}

size_t ReadWriteCopyBufferRectEvent::region(unsigned int index) const
{
    return p_region[index];
}

size_t ReadWriteCopyBufferRectEvent::src_row_pitch() const
{
    return p_src_row_pitch;
}

size_t ReadWriteCopyBufferRectEvent::src_slice_pitch() const
{
    return p_src_slice_pitch;
}

size_t ReadWriteCopyBufferRectEvent::dst_row_pitch() const
{
    return p_dst_row_pitch;
}

size_t ReadWriteCopyBufferRectEvent::dst_slice_pitch() const
{
    return p_dst_slice_pitch;
}

MemObject *ReadWriteCopyBufferRectEvent::source() const
{
    return buffer();
}

CopyBufferRectEvent::CopyBufferRectEvent(CommandQueue *parent,
                                         MemObject *source,
                                         MemObject *destination,
                                         const size_t src_origin[3],
                                         const size_t dst_origin[3],
                                         const size_t region[3],
                                         size_t src_row_pitch,
                                         size_t src_slice_pitch,
                                         size_t dst_row_pitch,
                                         size_t dst_slice_pitch,
                                         unsigned int bytes_per_element,
                                         cl_uint num_events_in_wait_list,
                                         const cl_event *event_wait_list,
                                         cl_int *errcode_ret)
: ReadWriteCopyBufferRectEvent(parent, source, src_origin, dst_origin, region,
                               src_row_pitch, src_slice_pitch, dst_row_pitch,
                               dst_slice_pitch, bytes_per_element,
                               num_events_in_wait_list, event_wait_list, errcode_ret),
  p_destination(destination)
{
    if (*errcode_ret != CL_SUCCESS) return;

    if (!destination)
    {
        *errcode_ret = CL_INVALID_MEM_OBJECT;
        return;
    }

    // Check for out-of-bounds
    // cross-row and cross-slice transfers on BufferRect should be allowed,
    // as seen by OpenCV use cases, and as SPEC does not prohibit such cases
    if ((p_src_origin[0] + p_region[0]) > source->size() ||
        (p_src_origin[1] + p_region[1]) * p_src_row_pitch > source->size() ||
        (p_src_origin[2] + p_region[2]) * p_src_slice_pitch > source->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if ((p_dst_origin[0] + p_region[0]) > destination->size() ||
        (p_dst_origin[1] + p_region[1]) * p_dst_row_pitch > destination->size() ||
        (p_dst_origin[2] + p_region[2]) * p_dst_slice_pitch > destination->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check for overlapping
    if (source == destination)
    {
        unsigned char overlapping_dimensions = 0;

        for (unsigned int i=0; i<3; ++i)
        {
            if ((p_dst_origin[i] < p_src_origin[i] && p_dst_origin[i] + p_region[i] > p_src_origin[i]) ||
                (p_src_origin[i] < p_dst_origin[i] && p_src_origin[i] + p_region[i] > p_dst_origin[i]))
                overlapping_dimensions++;
        }

        if (overlapping_dimensions == 3)
        {
            // If all the dimensions are overlapping, the region is overlapping
            *errcode_ret = CL_MEM_COPY_OVERLAP;
            return;
        }
    }

    // Check alignment of destination (source already checked by BufferEvent)
    cl_device_id d_device = 0;
    *errcode_ret = parent->info(CL_QUEUE_DEVICE, sizeof(cl_device_id), &d_device, 0);

    if (*errcode_ret != CL_SUCCESS)
        return;

    auto device = pobj(d_device);
    if (!isSubBufferAligned(destination, device))
    {
        *errcode_ret = CL_MISALIGNED_SUB_BUFFER_OFFSET;
        return;
    }

    // Allocate the buffer for the device
    if (!destination->allocate(device))
    {
        *errcode_ret = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        return;
    }
}

Event::Type CopyBufferRectEvent::type() const
{
    return Event::CopyBufferRect;
}

MemObject *CopyBufferRectEvent::destination() const
{
    return p_destination;
}

ReadWriteBufferRectEvent::ReadWriteBufferRectEvent(CommandQueue *parent,
                                                   MemObject *buffer,
                                                   const size_t buffer_origin[3],
                                                   const size_t host_origin[3],
                                                   const size_t region[3],
                                                   size_t buffer_row_pitch,
                                                   size_t buffer_slice_pitch,
                                                   size_t host_row_pitch,
                                                   size_t host_slice_pitch,
                                                   void *ptr,
                                                   unsigned int bytes_per_element,
                                                   cl_uint num_events_in_wait_list,
                                                   const cl_event *event_wait_list,
                                                   cl_int *errcode_ret)
: ReadWriteCopyBufferRectEvent(parent, buffer, buffer_origin, host_origin, region,
                               buffer_row_pitch, buffer_slice_pitch,
                               host_row_pitch, host_slice_pitch, bytes_per_element,
                               num_events_in_wait_list, event_wait_list, errcode_ret),
  p_ptr(ptr)
{
    if (*errcode_ret != CL_SUCCESS) return;

    if (!ptr)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check for out-of-bounds
    // cross-row and cross-slice transfers on BufferRect should be allowed,
    // as seen by OpenCV use cases, and as SPEC does not prohibit such cases
    if ((p_src_origin[0] + p_region[0]) > buffer->size() ||
        (p_src_origin[1] + p_region[1]) * p_src_row_pitch > buffer->size() ||
        (p_src_origin[2] + p_region[2]) * p_src_slice_pitch > buffer->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

void *ReadWriteBufferRectEvent::ptr() const
{
    return p_ptr;
}

ReadBufferRectEvent::ReadBufferRectEvent (CommandQueue *parent,
                                          MemObject *buffer,
                                          const size_t buffer_origin[3],
                                          const size_t host_origin[3],
                                          const size_t region[3],
                                          size_t buffer_row_pitch,
                                          size_t buffer_slice_pitch,
                                          size_t host_row_pitch,
                                          size_t host_slice_pitch,
                                          void *ptr,
                                          cl_uint num_events_in_wait_list,
                                          const cl_event *event_wait_list,
                                          cl_int *errcode_ret)
: ReadWriteBufferRectEvent(parent, buffer, buffer_origin, host_origin, region,
                           buffer_row_pitch, buffer_slice_pitch, host_row_pitch,
                           host_slice_pitch, ptr, 1, num_events_in_wait_list,
                           event_wait_list, errcode_ret)
{
}

Event::Type ReadBufferRectEvent::type() const
{
    return ReadBufferRect;
}

WriteBufferRectEvent::WriteBufferRectEvent (CommandQueue *parent,
                                            MemObject *buffer,
                                            const size_t buffer_origin[3],
                                            const size_t host_origin[3],
                                            const size_t region[3],
                                            size_t buffer_row_pitch,
                                            size_t buffer_slice_pitch,
                                            size_t host_row_pitch,
                                            size_t host_slice_pitch,
                                            void *ptr,
                                            cl_uint num_events_in_wait_list,
                                            const cl_event *event_wait_list,
                                            cl_int *errcode_ret)
: ReadWriteBufferRectEvent (parent, buffer, buffer_origin, host_origin, region,
                            buffer_row_pitch, buffer_slice_pitch, host_row_pitch,
                            host_slice_pitch, ptr, 1, num_events_in_wait_list,
                            event_wait_list, errcode_ret)
{
}

Event::Type WriteBufferRectEvent::type() const
{
    return WriteBufferRect;
}

ReadWriteImageEvent::ReadWriteImageEvent (CommandQueue *parent,
                                          Image2D *image,
                                          const size_t origin[3],
                                          const size_t region[3],
                                          size_t row_pitch,
                                          size_t slice_pitch,
                                          void *ptr,
                                          cl_uint num_events_in_wait_list,
                                          const cl_event *event_wait_list,
                                          cl_int *errcode_ret)
: ReadWriteBufferRectEvent(parent, image, origin, 0, region, image->row_pitch(),
                           image->slice_pitch(), row_pitch, slice_pitch, ptr,
                           image->pixel_size(), num_events_in_wait_list,
                           event_wait_list, errcode_ret)
{
    if (*errcode_ret != CL_SUCCESS) return;

    if (image->type() == MemObject::Image2D &&
        (origin[2] != 0 || region[2] != 1))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

ReadImageEvent::ReadImageEvent(CommandQueue *parent,
                               Image2D *image,
                               const size_t origin[3],
                               const size_t region[3],
                               size_t row_pitch,
                               size_t slice_pitch,
                               void *ptr,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_int *errcode_ret)
: ReadWriteImageEvent(parent, image, origin, region, row_pitch, slice_pitch, ptr,
                      num_events_in_wait_list, event_wait_list, errcode_ret)
{}

Event::Type ReadImageEvent::type() const
{
    return Event::ReadImage;
}

WriteImageEvent::WriteImageEvent(CommandQueue *parent,
                                 Image2D *image,
                                 const size_t origin[3],
                                 const size_t region[3],
                                 size_t row_pitch,
                                 size_t slice_pitch,
                                 void *ptr,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 cl_int *errcode_ret)
: ReadWriteImageEvent (parent, image, origin, region, row_pitch, slice_pitch, ptr,
                       num_events_in_wait_list, event_wait_list, errcode_ret)
{}

Event::Type WriteImageEvent::type() const
{
    return Event::WriteImage;
}

static bool operator!=(const cl_image_format &a, const cl_image_format &b)
{
    return (a.image_channel_data_type != b.image_channel_data_type) ||
           (a.image_channel_order != b.image_channel_order);
}

CopyImageEvent::CopyImageEvent(CommandQueue *parent,
                               Image2D *source,
                               Image2D *destination,
                               const size_t src_origin[3],
                               const size_t dst_origin[3],
                               const size_t region[3],
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_int *errcode_ret)
: CopyBufferRectEvent (parent, source, destination, src_origin, dst_origin,
                       region, source->row_pitch(), source->slice_pitch(),
                       destination->row_pitch(), destination->slice_pitch(),
                       source->pixel_size(), num_events_in_wait_list,
                       event_wait_list, errcode_ret)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Check bounds
    if (source->type() == MemObject::Image2D &&
        (src_origin[2] != 0 || region[2] != 1))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (destination->type() == MemObject::Image2D &&
        (dst_origin[2] != 0 || region[2] != 1))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Formats must match
    if (source->format() != destination->format())
    {
        *errcode_ret = CL_IMAGE_FORMAT_MISMATCH;
        return;
    }
}

Event::Type CopyImageEvent::type() const
{
    return Event::CopyImage;
}

CopyImageToBufferEvent::CopyImageToBufferEvent(CommandQueue *parent,
                                               Image2D *source,
                                               MemObject *destination,
                                               const size_t src_origin[3],
                                               const size_t region[3],
                                               size_t dst_offset,
                                               cl_uint num_events_in_wait_list,
                                               const cl_event *event_wait_list,
                                               cl_int *errcode_ret)
: CopyBufferRectEvent(parent, source, destination, src_origin, 0, region,
                      source->row_pitch(), source->slice_pitch(), 0, 0,
                      source->pixel_size(), num_events_in_wait_list,
                      event_wait_list, errcode_ret),
  p_offset(dst_offset)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Check for buffer overflow
    size_t dst_cb = region[2] * region[1] * region[0] * source->pixel_size();

    if (dst_offset + dst_cb > destination->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check validity
    if (source->type() == MemObject::Image2D &&
        (src_origin[2] != 0 || region[2] != 1))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

size_t CopyImageToBufferEvent::offset() const
{
    return p_offset;
}

Event::Type CopyImageToBufferEvent::type() const
{
    return Event::CopyImageToBuffer;
}

CopyBufferToImageEvent::CopyBufferToImageEvent(CommandQueue *parent,
                                               MemObject *source,
                                               Image2D *destination,
                                               size_t src_offset,
                                               const size_t dst_origin[3],
                                               const size_t region[3],
                                               cl_uint num_events_in_wait_list,
                                               const cl_event *event_wait_list,
                                               cl_int *errcode_ret)
: CopyBufferRectEvent(parent, source, destination, 0, dst_origin, region, 0, 0,
                      destination->row_pitch(), destination->slice_pitch(),
                      destination->pixel_size(), num_events_in_wait_list,
                      event_wait_list, errcode_ret),
  p_offset(src_offset)
{
    if (*errcode_ret != CL_SUCCESS) return;

    // Check for buffer overflow
    size_t src_cb = region[2] * region[1] * region[0] * destination->pixel_size();

    if (src_offset + src_cb > source->size())
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check validity
    if (destination->type() == MemObject::Image2D &&
        (dst_origin[2] != 0 || region[2] != 1))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }
}

size_t CopyBufferToImageEvent::offset() const
{
    return p_offset;
}

Event::Type CopyBufferToImageEvent::type() const
{
    return Event::CopyBufferToImage;
}

/*
 * Barrier
 */

BarrierEvent::BarrierEvent(CommandQueue *parent, cl_int *errcode_ret)
: Event(parent, Queued, 0, 0, errcode_ret)
{}

Event::Type BarrierEvent::type() const
{
    return Event::Barrier;
}

/*
 * WaitForEvents
 */

WaitForEventsEvent::WaitForEventsEvent(CommandQueue *parent,
                                       cl_uint num_events_in_wait_list,
                                       const cl_event *event_wait_list,
                                       cl_int *errcode_ret)
: Event(parent, Queued, num_events_in_wait_list, event_wait_list, errcode_ret)
{}

Event::Type WaitForEventsEvent::type() const
{
    return Event::WaitForEvents;
}

/*
 * Marker
 */
MarkerEvent::MarkerEvent(CommandQueue *parent,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_int *errcode_ret)
: WaitForEventsEvent(parent, num_events_in_wait_list, event_wait_list, errcode_ret)
{}

Event::Type MarkerEvent::type() const
{
    return Event::Marker;
}
