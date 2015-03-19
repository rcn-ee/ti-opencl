#include "buffer.h"
#include "device.h"

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
        // ASW TODO ???
        /*---------------------------------------------------------------------
        * We use the host ptr, we are already allocated
        *--------------------------------------------------------------------*/
        p_data = buffer->host_ptr();
    }
}

DSPBuffer::~DSPBuffer()
{
    if (p_data_malloced)
        p_device->free_ddr(p_data);
}

void *DSPBuffer::data() const
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

        char *tmp_data = (char*)parent_dspbuf->data();
        tmp_data += subbuf->offset();

        return (void*)tmp_data;
    }
    else if (!p_data) ; // ERROR();

    return p_data;
}

void *DSPBuffer::nativeGlobalPointer() const
{
    return data();
}

bool DSPBuffer::allocate()
{
    size_t buf_size = p_buffer->size();

    /*-------------------------------------------------------------------------
    * Something went wrong...
    *------------------------------------------------------------------------*/
    if (buf_size == 0) return false;

    /*-------------------------------------------------------------------------
    * We not using a host ptr, allocate a buffer
    *------------------------------------------------------------------------*/
    if (!p_data)
    {
        p_data = p_device->malloc_ddr(buf_size);

        if (!p_data) return false;

        p_data_malloced = true;
    }

    //ASW TODO ???
    if (p_buffer->type() != MemObject::SubBuffer &&
        p_buffer->flags() & CL_MEM_COPY_HOST_PTR)
        std::memcpy(p_data, p_buffer->host_ptr(), buf_size);

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
