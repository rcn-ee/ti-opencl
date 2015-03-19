/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "buffer.h"
#include "device.h"
#include "driver.h"

#include "CL/cl_ext.h"
#include "../memobject.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Coal;

DSPBuffer::DSPBuffer(DSPDevice *device, MemObject *buffer, cl_int *rs)
     : DeviceBuffer(), p_device(device), p_buffer(buffer), p_data(0), 
       p_data_malloced(false), p_buffer_idx(0)
{
    if (buffer->type() != MemObject::SubBuffer &&
        buffer->flags() & CL_MEM_USE_HOST_PTR)
    {
        /*---------------------------------------------------------------------
        * We use the host ptr, we are already allocated
        *--------------------------------------------------------------------*/
        if (device->clMallocQuery(buffer->host_ptr(), &p_data, NULL))
            buffer->set_host_ptr_clMalloced();
        else
            p_data = (DSPDevicePtr64) buffer->host_ptr();
    }
}

DSPBuffer::~DSPBuffer()
{
    if (p_data_malloced)
    {
        if (p_buffer->flags() & CL_MEM_USE_MSMC_TI)
             p_device->free_msmc  (p_data);
        else p_device->free_global(p_data);
    }
}

DSPDevicePtr64 DSPBuffer::data() const
{
    if (!p_data && p_buffer->type() == MemObject::SubBuffer)
    {
        /*---------------------------------------------------------------------
        * Data is based on the DSPBuffer of the parent buffer
        *--------------------------------------------------------------------*/
        SubBuffer *subbuf        = (SubBuffer *)p_buffer;
        MemObject *parent        = subbuf->parent();
        DSPBuffer *parent_dspbuf = (DSPBuffer *)parent->deviceBuffer(p_device);

        if (!parent_dspbuf->data()) parent_dspbuf->allocate();
        if (!parent_dspbuf->data()) { return 0; } //ERROR() 

        return parent_dspbuf->data() + subbuf->offset();
    }
    else if (!p_data) ; // ERROR();

    return p_data;
}

void *DSPBuffer::nativeGlobalPointer() const
{
    // this is correct only when USE_HOST_PTR!!!
    if (p_buffer->flags() & CL_MEM_USE_HOST_PTR)  return p_buffer->host_ptr();
    else                                          return (void*) data();
}

bool DSPBuffer::allocate()
{
    size_t buf_size = p_buffer->size();

    /*-------------------------------------------------------------------------
    * Something went wrong...
    *------------------------------------------------------------------------*/
    if (buf_size == 0) return false;

    if (!p_data && p_buffer->type() == MemObject::SubBuffer)
    {
        /*---------------------------------------------------------------------
        * Data is based on the DSPBuffer of the parent buffer
        *--------------------------------------------------------------------*/
        SubBuffer *subbuf        = (SubBuffer *)p_buffer;
        MemObject *parent        = subbuf->parent();
        DSPBuffer *parent_dspbuf = (DSPBuffer *)parent->deviceBuffer(p_device);

        if (!parent_dspbuf->data()) parent_dspbuf->allocate();
        if (!parent_dspbuf->data()) return false;

        p_data =  parent_dspbuf->data() + subbuf->offset();
        return true;
    }

    /*-------------------------------------------------------------------------
    * We not using a host ptr, allocate a buffer
    *------------------------------------------------------------------------*/
    if (!p_data)
    {
        if (p_buffer->flags() & CL_MEM_USE_MSMC_TI)
             p_data = (DSPDevicePtr64) p_device->malloc_msmc(buf_size);
        else p_data = (DSPDevicePtr64) p_device->malloc_global(buf_size, false);

        if (!p_data) return false;

        p_data_malloced = true;
    }

    if (p_buffer->type() != MemObject::SubBuffer &&
        p_buffer->flags() & CL_MEM_COPY_HOST_PTR)
        Driver::instance()->write(p_device->dspID(), p_data, 
                                (uint8_t*)p_buffer->host_ptr(), buf_size);

    // Say to the memobject that we are allocated
    p_buffer->deviceAllocated(this);

    return true;
}

DeviceInterface *DSPBuffer::device() const
{
    return p_device;
}

bool DSPBuffer::allocated() const
{
    return p_data != 0;
}
