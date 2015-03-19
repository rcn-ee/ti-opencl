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
 * \file cpu/buffer.cpp
 * \brief CPU buffer
 */

#include "buffer.h"
#include "device.h"

#include "../memobject.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Coal;

CPUBuffer::CPUBuffer(CPUDevice *device, MemObject *buffer, cl_int *rs)
: DeviceBuffer(), p_device(device), p_buffer(buffer), p_data(0),
  p_data_malloced(false)
{
    if (buffer->type() == MemObject::SubBuffer)
    {
        // We need to create this CPUBuffer based on the CPUBuffer of the
        // parent buffer
        SubBuffer *subbuf = (SubBuffer *)buffer;
        MemObject *parent = subbuf->parent();
        CPUBuffer *parentcpubuf = (CPUBuffer *)parent->deviceBuffer(device);

        char *tmp_data = (char *)parentcpubuf->data();
        tmp_data += subbuf->offset();

        p_data = (void *)tmp_data;
    }
    else if (buffer->flags() & CL_MEM_USE_HOST_PTR)
    {
        // We use the host ptr, we are already allocated
        p_data = buffer->host_ptr();
    }

    // NOTE: This function can also reject Image buffers by setting a value
    // != CL_SUCCESS in rs.
}

CPUBuffer::~CPUBuffer()
{
    if (p_data_malloced)
    {
        std::free((void *)p_data);
    }
}

void *CPUBuffer::data() const
{
    return p_data;
}

void *CPUBuffer::nativeGlobalPointer() const
{
    return data();
}

bool CPUBuffer::allocate()
{
    size_t buf_size = p_buffer->size();

    if (buf_size == 0)
        // Something went wrong...
        return false;

    if (!p_data)
    {
        // We don't use a host ptr, we need to allocate a buffer
        p_data = std::malloc(buf_size);

        if (!p_data)
            return false;

        p_data_malloced = true;
    }

    if (p_buffer->type() != MemObject::SubBuffer &&
        p_buffer->flags() & CL_MEM_COPY_HOST_PTR)
    {
        std::memcpy(p_data, p_buffer->host_ptr(), buf_size);
    }

    // Say to the memobject that we are allocated
    p_buffer->deviceAllocated(this);

    return true;
}

DeviceInterface *CPUBuffer::device() const
{
    return p_device;
}

bool CPUBuffer::allocated() const
{
    return p_data != 0;
}
