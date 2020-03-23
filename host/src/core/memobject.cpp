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
 * \file memobject.cpp
 * \brief Memory objects
 */

#include "CL/TI/cl_ext.h"
#include "memobject.h"
#include "context.h"
#include "deviceinterface.h"
#include "propertylist.h"
#include "events.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Coal;

/*
 * MemObject
 */

MemObject::MemObject(Context *ctx, cl_mem_flags flags, void *host_ptr,
                     cl_int *errcode_ret)
: Object(Object::T_MemObject, ctx), p_num_devices(0), p_flags(flags),
  p_host_ptr(host_ptr), p_host_ptr_clMalloced(false),
  p_devicebuffers(0), p_dtor_callback_stack()
{
    // Check the flags value
    const cl_mem_flags all_flags = CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY |
                                   CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR |
                                   CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR
                                   |CL_MEM_USE_MSMC_TI | CL_MEM_HOST_NO_ACCESS;

    if ((flags & ~all_flags) != 0)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if ((flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    // Check other values
    if ((flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) != 0 && !host_ptr)
    {
        *errcode_ret = CL_INVALID_HOST_PTR;
        return;
    }

    if ((flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0 && host_ptr)
    {
        *errcode_ret = CL_INVALID_HOST_PTR;
        return;
    }
}

MemObject::~MemObject()
{
    while (!p_dtor_callback_stack.empty())
    {
        dtor_callback_t callback;
        if (p_dtor_callback_stack.pop(callback))
	    callback.first(desc(this), callback.second);
    }

    if (p_devicebuffers)
    {
        // Also delete our children in the device
        for (unsigned int i=0; i<p_num_devices; ++i)
            if (p_devicebuffers[i] != nullptr)  delete p_devicebuffers[i];

        std::free((void *)p_devicebuffers);
    }
}

cl_int MemObject::init()
{
    // Get the device list of the context
    cl_device_id *devices = 0;
    cl_int rs;

    rs = ((Context *)parent())->info(CL_CONTEXT_NUM_DEVICES,
                                     sizeof(unsigned int),
                                     &p_num_devices, 0);

    if (rs != CL_SUCCESS)
        return rs;

    p_devices_to_allocate = p_num_devices;
    devices = (cl_device_id *)std::malloc(p_num_devices * sizeof(cl_device_id));

    if (!devices)
        return CL_OUT_OF_HOST_MEMORY;

    rs = ((Context *)parent())->info(CL_CONTEXT_DEVICES,
                                     p_num_devices * sizeof(cl_device_id),
                                     devices, 0);

    if (rs != CL_SUCCESS)
    {
        std::free((void *)devices);
        return rs;
    }

    // Allocate a table of DeviceBuffers
    p_devicebuffers = (DeviceBuffer **)std::malloc(p_num_devices *
                                             sizeof(DeviceBuffer *));

    if (!p_devicebuffers)
    {
        std::free((void *)devices);
        return CL_OUT_OF_HOST_MEMORY;
    }
    std::memset(p_devicebuffers, 0, p_num_devices * sizeof(DeviceBuffer *));

    // If we have more than one device, the allocation on the devices is
    // defered to first use, so host_ptr can become invalid. So, copy it in
    // a RAM location and keep it. Also, set a flag telling CPU devices that
    // they don't need to reallocate and re-copy host_ptr
    // SubBuffer should simply reuse Buffer data
    if (p_num_devices > 1 && (p_flags & CL_MEM_COPY_HOST_PTR)
                          && type() != SubBuffer)
    {
        void *tmp_hostptr = std::malloc(size());

        if (!tmp_hostptr)
        {
            std::free((void *)devices);
            return CL_OUT_OF_HOST_MEMORY;
        }

        std::memcpy(tmp_hostptr, p_host_ptr, size());

        p_host_ptr = tmp_hostptr;
        // Now, the client application can safely std::free() its host_ptr
    }

    // Create a DeviceBuffer for each device
    unsigned int failed_devices = 0;

    for (unsigned int i=0; i<p_num_devices; ++i)
    {
        auto device = pobj(devices[i]);

        rs = CL_SUCCESS;
        DeviceBuffer *d_buf_allocated = deviceBuffer(device);
        if (d_buf_allocated == nullptr)
            p_devicebuffers[i] = device->createDeviceBuffer(this, &rs);
        // else: keep p_devicebuffers[i] as nullptr to prevent double freeing
        //       because the device buffer has already been allocated and
        //       owned by another device j.  Let device j free buffer later.
        // Alternatively, we might use std::shared_ptr<DeviceBuffer> in C++11

        if (rs != CL_SUCCESS)
        {
            p_devicebuffers[i] = 0;
            failed_devices++;
        }
    }

    if (failed_devices == p_num_devices)
    {
        // Each device found a reason to reject the buffer, so it's invalid
        std::free((void *)devices);
        return rs;
    }

    std::free((void *)devices);
    devices = 0;

    // If we have only one device, pre-allocate the buffer
    if (p_num_devices == 1)
    {
        if (!p_devicebuffers[0]->allocate())
            return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }

    return CL_SUCCESS;
}

bool MemObject::allocate(DeviceInterface *device)
{
    DeviceBuffer *buffer = deviceBuffer(device);

    if (!buffer->allocated())
    {
        return buffer->allocate();
    }

    return true;
}

cl_mem_flags MemObject::flags() const
{
    return p_flags;
}

void *MemObject::host_ptr() const
{
    if (type() != SubBuffer)
        return p_host_ptr;
    else
    {
        const class SubBuffer *subbuf = (const class SubBuffer *)this;
        char *tmp = (char *)subbuf->parent()->host_ptr();

        if (!tmp) return 0;

        tmp += subbuf->offset();

        return (void *)tmp;
    }
}

/*----------------------------------------------------------------------------
 * deviceBuffer(): if devices share the same shared memory handler,
 *                 then they share the same device buffer for the MemObject
 *                 System memory is NULL SHMHandler, used by CPU Device/Buffer
 *---------------------------------------------------------------------------*/
DeviceBuffer *MemObject::deviceBuffer(DeviceInterface *device) const
{
    return deviceBuffer(device->GetSHMHandler());
}

DeviceBuffer *MemObject::deviceBuffer(tiocl::SharedMemory *shm) const
{
    for (unsigned int i=0; i<p_num_devices; ++i)
    {
        if (p_devicebuffers[i] != nullptr && 
            p_devicebuffers[i]->GetSHMHandler() == shm)
            return p_devicebuffers[i];
    }

    return nullptr;
}

void MemObject::deviceAllocated(DeviceBuffer *buffer)
{
    (void) buffer;

    // Decrement the count of devices that must be allocated. If it becomes
    // 0, it means we don't need to keep a copied host_ptr and that we can
    // std::free() it.
    p_devices_to_allocate--;

    if (p_devices_to_allocate == 0 &&
        p_num_devices > 1 &&
        (p_flags & CL_MEM_COPY_HOST_PTR))
    {
        std::free(p_host_ptr);
        p_host_ptr = 0;
    }

}

void MemObject::setDestructorCallback(void (CL_CALLBACK *pfn_notify)
                                               (cl_mem memobj, void *user_data),
                                      void *user_data)
{
    p_dtor_callback_stack.push(dtor_callback_t(pfn_notify, user_data));
}

// HACK for the union
typedef void * void_p;

cl_int MemObject::info(cl_mem_info param_name,
                       size_t param_value_size,
                       void *param_value,
                       size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;
    class SubBuffer *subbuf = (class SubBuffer *)this;

    union {
        cl_mem_object_type cl_mem_object_type_var;
        cl_mem_flags cl_mem_flags_var;
        size_t size_t_var;
        void_p void_p_var;
        cl_uint cl_uint_var;
        cl_context cl_context_var;
        cl_mem cl_mem_var;
    };

    switch (param_name)
    {
        case CL_MEM_TYPE:
            switch (type())
            {
                case Buffer:
                case SubBuffer:
                    cl_mem_object_type_var = CL_MEM_OBJECT_BUFFER;
                    break;

                case Image2D:
                    cl_mem_object_type_var = CL_MEM_OBJECT_IMAGE2D;
                    break;

                case Image3D:
                    cl_mem_object_type_var = CL_MEM_OBJECT_IMAGE3D;
                    break;
            }
            value = (void *)&cl_mem_object_type_var;
            value_length = sizeof(cl_mem_object_type);
            break;

        case CL_MEM_FLAGS:
            SIMPLE_ASSIGN(cl_mem_flags, p_flags);
            break;

        case CL_MEM_SIZE:
            SIMPLE_ASSIGN(size_t, size());
            break;

        case CL_MEM_HOST_PTR:
            SIMPLE_ASSIGN(void_p, host_ptr());
            break;

        case CL_MEM_MAP_COUNT:
            SIMPLE_ASSIGN(cl_uint, 0); // TODO
            break;

        case CL_MEM_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_MEM_CONTEXT:
	        SIMPLE_ASSIGN(cl_context, desc((Context *)parent()));
            break;

        case CL_MEM_ASSOCIATED_MEMOBJECT:
            if (type() != SubBuffer)
                SIMPLE_ASSIGN(cl_mem, 0)
            else
                SIMPLE_ASSIGN(cl_mem, desc(subbuf->parent()));
            break;

        case CL_MEM_OFFSET:
            if (type() != SubBuffer)
                SIMPLE_ASSIGN(size_t, 0)
            else
                SIMPLE_ASSIGN(size_t, subbuf->offset());
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/*
 * Buffer
 */

Buffer::Buffer(Context *ctx, size_t size, void *host_ptr, cl_mem_flags flags,
               cl_int *errcode_ret)
: MemObject(ctx, flags, host_ptr, errcode_ret), p_size(size)
{
    if (size == 0)
    {
        *errcode_ret = CL_INVALID_BUFFER_SIZE;
        return;
    }

    // CL_MEM_READ_WRITE is default if not specified {READ,WRITE}_ONLY
    if (! (flags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)))
        p_flags |= CL_MEM_READ_WRITE;
}

size_t Buffer::size() const
{
    return p_size;
}

MemObject::Type Buffer::type() const
{
    return MemObject::Buffer;
}

/*----------------------------------------------------------------------------
 * mapped_event: MapBufferEvent when the Map is on a Buffer
 * RETURN: true if successful, false if fail
 *     Traverse currently mapped event list, check overlapping and if either is
 *     WRITE, insert into list in the increasing order of offset
 * TODO: do we need to lock the list for operation???
 *---------------------------------------------------------------------------*/
bool Buffer::addMapEvent(BufferEvent *mapped_event)
{
    MapBufferEvent *mbe = (MapBufferEvent *) mapped_event;
    size_t   mbe_offset = mbe->offset();
    if (mbe->buffer()->type() == SubBuffer)
        mbe_offset += ((class SubBuffer *) mbe->buffer())->offset();

    std::list<BufferEvent *>::iterator it, it_insert = p_mapped_events.end();
    for (it = p_mapped_events.begin(); it != p_mapped_events.end(); ++it)
    {
        MapBufferEvent *e = (MapBufferEvent *) (*it);
        size_t   e_offset = e->offset();
        if (e->buffer()->type() == SubBuffer)
            e_offset += ((class SubBuffer *) e->buffer())->offset();
        if (mbe_offset < e_offset) it_insert = it;

        if (   mbe_offset <= e_offset + e->cb() - 1
            &&   e_offset <= mbe_offset + mbe->cb() - 1)
            if ((mbe->flags() & CL_MAP_WRITE) ||
                  (e->flags() & CL_MAP_WRITE))
                return false;
    }

    p_mapped_events.insert(it_insert, mapped_event);
    return true;
}

/*----------------------------------------------------------------------------
 * mapped_ptr: mapped pointer from previous MapBuffer/MapImage Event
 * RETURN: first MappedBufferEvent with same mapped_ptr in the list
 * TODO: do we need to lock the list for operation???
 *---------------------------------------------------------------------------*/
BufferEvent* Buffer::removeMapEvent(void *mapped_ptr)
{
    std::list<BufferEvent *>::iterator it;
    for (it = p_mapped_events.begin(); it != p_mapped_events.end(); ++it)
    {
        MapBufferEvent *e = (MapBufferEvent *) (*it);
        if (e->ptr() != mapped_ptr)  continue;
        p_mapped_events.erase(it);
        return e;
    }
    return NULL;
}

/*
 * SubBuffer
 */

SubBuffer::SubBuffer(class Buffer *parent, size_t offset, size_t size,
                     cl_mem_flags flags, cl_int *errcode_ret)
: MemObject((Context *)parent->parent(), flags, 0, errcode_ret), p_offset(offset),
  p_size(size), p_parent(parent)
{
    clRetainMemObject(desc(p_parent));

    if (size == 0)
    {
        *errcode_ret = CL_INVALID_BUFFER_SIZE;
        return;
    }

    if (offset + size > parent->size())
    {
        *errcode_ret = CL_INVALID_BUFFER_SIZE;
        return;
    }

    // Check the compatibility of flags and parent->flags()
    const cl_mem_flags wrong_flags =
        CL_MEM_ALLOC_HOST_PTR |
        CL_MEM_USE_HOST_PTR |
        CL_MEM_COPY_HOST_PTR;

    if (flags & wrong_flags)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if ((parent->flags() & CL_MEM_WRITE_ONLY) &&
        (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY)))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if ((parent->flags() & CL_MEM_READ_ONLY) &&
        (flags & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY)))
    {
        *errcode_ret = CL_INVALID_VALUE;
        return;
    }

    if (parent->get_host_ptr_clMalloced())  set_host_ptr_clMalloced();

    // OpenCL 1.2: SubBuffer should inherit some of parent Buffer flags
    cl_mem_flags parent_rw_flags = parent->flags()
                 & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);
    cl_mem_flags my_rw_flags = p_flags
                 & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);
    // parent be READ_WRITE, subBuffer be READ_ONLY/WRITE_ONLY (Spec allows)
    if (! my_rw_flags)  p_flags |= parent_rw_flags;
    cl_mem_flags parent_hostptr_flags = parent->flags()
       & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
    if (parent_hostptr_flags) p_flags |= parent_hostptr_flags;
}

SubBuffer::~SubBuffer()
{
    clReleaseMemObject((cl_mem) p_parent);
}

size_t SubBuffer::size() const
{
    return p_size;
}

MemObject::Type SubBuffer::type() const
{
    return MemObject::SubBuffer;
}

bool SubBuffer::allocate(DeviceInterface *device)
{
    // SubBuffer always use Buffer's data
    return p_parent->allocate(device);
}

size_t SubBuffer::offset() const
{
    return p_offset;
}

Buffer *SubBuffer::parent() const
{
    return p_parent;
}

bool SubBuffer::addMapEvent(BufferEvent *mapped_event)
{
    return p_parent->addMapEvent(mapped_event);
}

BufferEvent* SubBuffer::removeMapEvent(void *mapped_ptr)
{
    return p_parent->removeMapEvent(mapped_ptr);
}

/*
 * Image2D
 */

Image2D::Image2D(Context *ctx, size_t width, size_t height, size_t row_pitch,
                 const cl_image_format *format, void *host_ptr,
                 cl_mem_flags flags, cl_int *errcode_ret)
: MemObject(ctx, flags, host_ptr, errcode_ret),
  p_width(width), p_height(height), p_row_pitch(row_pitch)
{
    if (!width || !height)
    {
        *errcode_ret = CL_INVALID_IMAGE_SIZE;
        return;
    }

    if (!format)
    {
        *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return;
    }

    p_format = *format;

    // Check format descriptor
    switch (p_format.image_channel_data_type)
    {
        case CL_UNORM_INT_101010:
        case CL_UNORM_SHORT_555:
        case CL_UNORM_SHORT_565:
            if (p_format.image_channel_order != CL_RGB ||
                p_format.image_channel_order != CL_RGBx)
            {
                *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
                return;
            }
    }

    switch (p_format.image_channel_order)
    {
        case CL_LUMINANCE:
        case CL_INTENSITY:
            switch (p_format.image_channel_data_type)
            {
                case CL_UNORM_INT8:
                case CL_UNORM_INT16:
                case CL_SNORM_INT8:
                case CL_SNORM_INT16:
                case CL_HALF_FLOAT:
                case CL_FLOAT:
                    break;
                default:
                    *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
                    return;
            }
            break;

        case CL_RGB:
        case CL_RGBx:
            switch (p_format.image_channel_data_type)
            {
                case CL_UNORM_SHORT_555:
                case CL_UNORM_SHORT_565:
                case CL_UNORM_INT_101010:
                    break;
                default:
                    *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
                    return;
            }
            break;

        case CL_ARGB:
        case CL_BGRA:
            switch (p_format.image_channel_data_type)
            {
                case CL_UNORM_INT8:
                case CL_SNORM_INT8:
                case CL_SIGNED_INT8:
                case CL_UNSIGNED_INT8:
                    break;
                default:
                    *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
                    return;
            }
            break;
    }

    // Row pitch
    p_row_pitch = width * pixel_size(p_format);

    if (row_pitch)
    {
        if (!host_ptr)
        {
            // row_pitch must be 0 if host_ptr is null
            *errcode_ret = CL_INVALID_IMAGE_SIZE;
            return;
        }
        if (row_pitch < p_row_pitch)
        {
            *errcode_ret = CL_INVALID_IMAGE_SIZE;
            return;
        }
        if (row_pitch % pixel_size(p_format) != 0)
        {
            *errcode_ret = CL_INVALID_IMAGE_SIZE;
            return;
        }

        p_row_pitch = row_pitch;
    }
}

size_t Image2D::size() const
{
    return height() * row_pitch();
}

MemObject::Type Image2D::type() const
{
    return MemObject::Image2D;
}

size_t Image2D::width() const
{
    return p_width;
}

size_t Image2D::height() const
{
    return p_height;
}

size_t Image2D::row_pitch() const
{
    return p_row_pitch;
}

size_t Image2D::slice_pitch() const
{
    // An Image2D is made of only one slice
    return size();
}

const cl_image_format &Image2D::format() const
{
    return p_format;
}

cl_int Image2D::imageInfo(cl_image_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;
    class Image3D *image3D = (class Image3D *)this;

    union {
        cl_image_format cl_image_format_var;
        size_t size_t_var;
    };

    switch (param_name)
    {
        case CL_IMAGE_FORMAT:
            SIMPLE_ASSIGN(cl_image_format, format());
            break;

        case CL_IMAGE_ELEMENT_SIZE:
            SIMPLE_ASSIGN(size_t, element_size(p_format));
            break;

        case CL_IMAGE_ROW_PITCH:
            // TODO: What was given when the image was created or width*size ?
            SIMPLE_ASSIGN(size_t, row_pitch());
            break;

        case CL_IMAGE_SLICE_PITCH:
            if (type() == Image3D)
                SIMPLE_ASSIGN(size_t, image3D->slice_pitch())
            else
                SIMPLE_ASSIGN(size_t, 0);
            break;

        case CL_IMAGE_WIDTH:
            SIMPLE_ASSIGN(size_t, width());
            break;

        case CL_IMAGE_HEIGHT:
            SIMPLE_ASSIGN(size_t, height());
            break;

        case CL_IMAGE_DEPTH:
            if (type() == Image3D)
                SIMPLE_ASSIGN(size_t, image3D->depth())
            else
                SIMPLE_ASSIGN(size_t, 0);
            break;
        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

size_t Image2D::element_size(const cl_image_format &format)
{
    switch (format.image_channel_data_type)
    {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            return 1;
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
            return 2;
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
            return 4;
        case CL_FLOAT:
            return sizeof(float);
        case CL_HALF_FLOAT:
            return 2;
        case CL_UNORM_SHORT_565:
        case CL_UNORM_SHORT_555:
            return 2;
        case CL_UNORM_INT_101010:
            return 4;
        default:
            return 0;
    }
}

unsigned int Image2D::channels(const cl_image_format &format)
{
    switch (format.image_channel_order)
    {
        case CL_R:
        case CL_Rx:
        case CL_A:
        case CL_INTENSITY:
        case CL_LUMINANCE:
            return 1;
            break;

        case CL_RG:
        case CL_RGx:
        case CL_RA:
            return 2;
            break;

        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
            return 4;
            break;

        case CL_RGBx:
        case CL_RGB:
            return 1; // Only special data types allowed (565, 555, etc)
            break;

        default:
            return 0;
    }
}

size_t Image2D::pixel_size(const cl_image_format &format)
{
    switch (format.image_channel_data_type)
    {
        case CL_UNORM_SHORT_565:
        case CL_UNORM_SHORT_555:
            return 2;
        case CL_UNORM_INT_101010:
            return 4;
        default:
            return channels(format) * element_size(format);
    }
}

size_t Image2D::element_size() const
{
    return element_size(p_format);
}

size_t Image2D::pixel_size() const
{
    return pixel_size(p_format);
}

unsigned int Image2D::channels() const
{
    return channels(p_format);
}

/*
 * Image3D
 */

Image3D::Image3D(Context *ctx, size_t width, size_t height, size_t depth,
                 size_t row_pitch, size_t slice_pitch,
                 const cl_image_format *format, void *host_ptr,
                 cl_mem_flags flags, cl_int *errcode_ret)
: Image2D(ctx, width, height, row_pitch, format, host_ptr, flags, errcode_ret),
  p_depth(depth)
{
    if (depth <= 1)
    {
        *errcode_ret = CL_INVALID_IMAGE_SIZE;
        return;
    }

    // Slice pitch
    p_slice_pitch = height * this->row_pitch();

    if (slice_pitch)
    {
        if (!host_ptr)
        {
            // slice_pitch must be 0 if host_ptr is null
            *errcode_ret = CL_INVALID_IMAGE_SIZE;
            return;
        }
        if (slice_pitch < p_slice_pitch)
        {
            *errcode_ret = CL_INVALID_IMAGE_SIZE;
            return;
        }
        if (slice_pitch % this->row_pitch() != 0)
        {
            *errcode_ret = CL_INVALID_IMAGE_SIZE;
            return;
        }

        p_slice_pitch = slice_pitch;
    }
}

size_t Image3D::size() const
{
    return depth() * slice_pitch();
}

MemObject::Type Image3D::type() const
{
    return MemObject::Image3D;
}

size_t Image3D::depth() const
{
    return p_depth;
}

size_t Image3D::slice_pitch() const
{
    return p_slice_pitch;
}
