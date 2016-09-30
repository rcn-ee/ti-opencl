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
 * \file api_memory.cpp
 * \brief Memory objects
 */

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include <core/memobject.h>
#include <core/context.h>
#include <core/platform.h>
#include <core/dsp/device.h>

#include <cstring>

// Memory Object APIs
cl_mem
clCreateBuffer(cl_context   d_context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret)
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

    *errcode_ret = CL_SUCCESS;

    Coal::Buffer *buf = new Coal::Buffer(context, size, host_ptr, flags,
                                         errcode_ret);

    if (*errcode_ret != CL_SUCCESS || (*errcode_ret = buf->init()) != CL_SUCCESS)
    {
        delete buf;
        return 0;
    }

    return desc(buf);
}

cl_mem
clCreateSubBuffer(cl_mem                d_buffer,
                  cl_mem_flags          flags,
                  cl_buffer_create_type buffer_create_type,
                  const void *          buffer_create_info,
                  cl_int *              errcode_ret)
{
    cl_int dummy_errcode;
    // code below seems to be expecting a Coal::Buffer *, so convert to such:
    Coal::Buffer * buffer = (Coal::Buffer *)pobj(d_buffer);

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!buffer->isA(Coal::Object::T_MemObject))
    {
        *errcode_ret = CL_INVALID_MEM_OBJECT;
        return 0;
    }

    Coal::MemObject *memobject = (Coal::MemObject *)buffer;
    cl_buffer_region *region = (cl_buffer_region *)buffer_create_info;

    // NOTE: Is it right ? Couldn't we create SubBuffers of images ?
    if (memobject->type() != Coal::MemObject::Buffer)
    {
        *errcode_ret = CL_INVALID_MEM_OBJECT;
        return 0;
    }

    if (buffer_create_type != CL_BUFFER_CREATE_TYPE_REGION)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    if (!buffer_create_info)
    {
        *errcode_ret = CL_INVALID_VALUE;
        return 0;
    }

    *errcode_ret = CL_SUCCESS;

    Coal::SubBuffer *buf = new Coal::SubBuffer(buffer,
                                               region->origin, region->size,
                                               flags, errcode_ret);

    if (*errcode_ret != CL_SUCCESS || (*errcode_ret = buf->init()) != CL_SUCCESS)
    {
        delete buf;
        return 0;
    }

    return desc(buf);
}

cl_mem
clCreateImage2D(cl_context              d_context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret)
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

    *errcode_ret = CL_SUCCESS;

    Coal::Image2D *image = new Coal::Image2D(context, image_width, image_height,
                                             image_row_pitch, image_format,
                                             host_ptr, flags, errcode_ret);

    if (*errcode_ret != CL_SUCCESS || (*errcode_ret = image->init()) != CL_SUCCESS)
    {
        delete image;
        return 0;
    }

    return desc(image);
}

cl_mem
clCreateImage3D(cl_context              d_context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_depth,
                size_t                  image_row_pitch,
                size_t                  image_slice_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret)
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

    *errcode_ret = CL_SUCCESS;

    Coal::Image3D *image = new Coal::Image3D(context, image_width, image_height,
                                             image_depth, image_row_pitch,
                                             image_slice_pitch, image_format,
                                             host_ptr, flags, errcode_ret);

    if (*errcode_ret != CL_SUCCESS || (*errcode_ret = image->init()) != CL_SUCCESS)
    {
        delete image;
        return 0;
    }

    return desc(image);
}

cl_int
clRetainMemObject(cl_mem d_memobj)
{
    auto memobj = pobj(d_memobj);

    if (!memobj->isA(Coal::Object::T_MemObject))
        return CL_INVALID_MEM_OBJECT;

    memobj->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseMemObject(cl_mem d_memobj)
{
    auto memobj = pobj(d_memobj);

    if (!memobj->isA(Coal::Object::T_MemObject))
        return CL_INVALID_MEM_OBJECT;

    if (memobj->dereference())
        delete memobj;

    return CL_SUCCESS;
}

static cl_image_format supported_formats[] = {
    { CL_RGBA, CL_UNORM_INT8 },
    { CL_RGBA, CL_UNORM_INT16 },
    { CL_RGBA, CL_SNORM_INT8 },
    { CL_RGBA, CL_SNORM_INT16 },
    { CL_RGBA, CL_SIGNED_INT8 },
    { CL_RGBA, CL_SIGNED_INT16 },
    { CL_RGBA, CL_SIGNED_INT32 },
    { CL_RGBA, CL_UNSIGNED_INT8 },
    { CL_RGBA, CL_UNSIGNED_INT16 },
    { CL_RGBA, CL_UNSIGNED_INT32 },
    { CL_RGBA, CL_FLOAT },

    { CL_ARGB, CL_UNORM_INT8 },
    { CL_ARGB, CL_SNORM_INT8 },
    { CL_ARGB, CL_SIGNED_INT8 },
    { CL_ARGB, CL_UNSIGNED_INT8 },

    { CL_BGRA, CL_UNORM_INT8 },
    { CL_BGRA, CL_SNORM_INT8 },
    { CL_BGRA, CL_SIGNED_INT8 },
    { CL_BGRA, CL_UNSIGNED_INT8 },

    { CL_RGB, CL_UNORM_SHORT_565 },
    { CL_RGB, CL_UNORM_SHORT_555 },
    { CL_RGB, CL_UNORM_INT_101010 },

    { CL_RGBx, CL_UNORM_SHORT_565 },
    { CL_RGBx, CL_UNORM_SHORT_555 },
    { CL_RGBx, CL_UNORM_INT_101010 },

    { CL_RG, CL_UNORM_INT8 },
    { CL_RG, CL_UNORM_INT16 },
    { CL_RG, CL_SNORM_INT8 },
    { CL_RG, CL_SNORM_INT16 },
    { CL_RG, CL_SIGNED_INT8 },
    { CL_RG, CL_SIGNED_INT16 },
    { CL_RG, CL_SIGNED_INT32 },
    { CL_RG, CL_UNSIGNED_INT8 },
    { CL_RG, CL_UNSIGNED_INT16 },
    { CL_RG, CL_UNSIGNED_INT32 },
    { CL_RG, CL_FLOAT },

    { CL_RGx, CL_UNORM_INT8 },
    { CL_RGx, CL_UNORM_INT16 },
    { CL_RGx, CL_SNORM_INT8 },
    { CL_RGx, CL_SNORM_INT16 },
    { CL_RGx, CL_SIGNED_INT8 },
    { CL_RGx, CL_SIGNED_INT16 },
    { CL_RGx, CL_SIGNED_INT32 },
    { CL_RGx, CL_UNSIGNED_INT8 },
    { CL_RGx, CL_UNSIGNED_INT16 },
    { CL_RGx, CL_UNSIGNED_INT32 },
    { CL_RGx, CL_FLOAT },

    { CL_RA, CL_UNORM_INT8 },
    { CL_RA, CL_UNORM_INT16 },
    { CL_RA, CL_SNORM_INT8 },
    { CL_RA, CL_SNORM_INT16 },
    { CL_RA, CL_SIGNED_INT8 },
    { CL_RA, CL_SIGNED_INT16 },
    { CL_RA, CL_SIGNED_INT32 },
    { CL_RA, CL_UNSIGNED_INT8 },
    { CL_RA, CL_UNSIGNED_INT16 },
    { CL_RA, CL_UNSIGNED_INT32 },
    { CL_RA, CL_FLOAT },

    { CL_R, CL_UNORM_INT8 },
    { CL_R, CL_UNORM_INT16 },
    { CL_R, CL_SNORM_INT8 },
    { CL_R, CL_SNORM_INT16 },
    { CL_R, CL_SIGNED_INT8 },
    { CL_R, CL_SIGNED_INT16 },
    { CL_R, CL_SIGNED_INT32 },
    { CL_R, CL_UNSIGNED_INT8 },
    { CL_R, CL_UNSIGNED_INT16 },
    { CL_R, CL_UNSIGNED_INT32 },
    { CL_R, CL_FLOAT },

    { CL_Rx, CL_UNORM_INT8 },
    { CL_Rx, CL_UNORM_INT16 },
    { CL_Rx, CL_SNORM_INT8 },
    { CL_Rx, CL_SNORM_INT16 },
    { CL_Rx, CL_SIGNED_INT8 },
    { CL_Rx, CL_SIGNED_INT16 },
    { CL_Rx, CL_SIGNED_INT32 },
    { CL_Rx, CL_UNSIGNED_INT8 },
    { CL_Rx, CL_UNSIGNED_INT16 },
    { CL_Rx, CL_UNSIGNED_INT32 },
    { CL_Rx, CL_FLOAT },

    { CL_A, CL_UNORM_INT8 },
    { CL_A, CL_UNORM_INT16 },
    { CL_A, CL_SNORM_INT8 },
    { CL_A, CL_SNORM_INT16 },
    { CL_A, CL_SIGNED_INT8 },
    { CL_A, CL_SIGNED_INT16 },
    { CL_A, CL_SIGNED_INT32 },
    { CL_A, CL_UNSIGNED_INT8 },
    { CL_A, CL_UNSIGNED_INT16 },
    { CL_A, CL_UNSIGNED_INT32 },
    { CL_A, CL_FLOAT },

    { CL_LUMINANCE, CL_UNORM_INT8 },
    { CL_LUMINANCE, CL_UNORM_INT16 },
    { CL_LUMINANCE, CL_SNORM_INT8 },
    { CL_LUMINANCE, CL_SNORM_INT16 },
    { CL_LUMINANCE, CL_FLOAT },

    { CL_INTENSITY, CL_UNORM_INT8 },
    { CL_INTENSITY, CL_UNORM_INT16 },
    { CL_INTENSITY, CL_SNORM_INT8 },
    { CL_INTENSITY, CL_SNORM_INT16 },
    { CL_INTENSITY, CL_FLOAT }
};

#ifdef MIN
#undef MIN
#endif
#define MIN(a, b) ((a) < (b) ? (a) : (b))

cl_int
clGetSupportedImageFormats(cl_context           d_context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type,
                           cl_uint              num_entries,
                           cl_image_format *    image_formats,
                           cl_uint *            num_image_formats)
{
    auto context = pobj(d_context);

    if (!context->isA(Coal::Object::T_Context))
        return CL_INVALID_CONTEXT;

    (void) flags;
    (void) image_type;

    if (!num_entries && image_formats)
        return CL_INVALID_VALUE;

    if (image_formats)
    {
        std::memcpy(image_formats, supported_formats,
                    MIN(num_entries * sizeof(cl_image_format),
                        sizeof(supported_formats)));
    }

    if (num_image_formats)
        *num_image_formats = sizeof(supported_formats) / sizeof(cl_image_format);

    return CL_SUCCESS;
}

cl_int
clGetMemObjectInfo(cl_mem           d_memobj,
                   cl_mem_info      param_name,
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret)
{
    auto memobj = pobj(d_memobj);

    if (!memobj->isA(Coal::Object::T_MemObject))
        return CL_INVALID_MEM_OBJECT;

    return memobj->info(param_name, param_value_size, param_value,
                        param_value_size_ret);
}

cl_int
clGetImageInfo(cl_mem           d_image,
               cl_image_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret)
{
    auto image = pobj(d_image);
    if (!image->isA(Coal::Object::T_MemObject) ||
            (image->type() != Coal::MemObject::Image2D &&
             image->type() != Coal::MemObject::Image3D))
        return CL_INVALID_MEM_OBJECT;

    Coal::Image2D *image2d = (Coal::Image2D *)image;

    return image2d->imageInfo(param_name, param_value_size, param_value,
                              param_value_size_ret);
}

cl_int
clSetMemObjectDestructorCallback(cl_mem d_memobj,
                                 void   (CL_CALLBACK *pfn_notify)(cl_mem memobj,
                                                                  void *user_data),
                                 void * user_data)
{
    auto memobj = pobj(d_memobj);

    if (!memobj->isA(Coal::Object::T_MemObject))
        return CL_INVALID_MEM_OBJECT;

    memobj->setDestructorCallback(pfn_notify, user_data);

    return CL_SUCCESS;
}

extern "C"
{

Coal::DSPDevice *
getDspDevice()
{
    static Coal::DSPDevice *dspdevice = NULL;

    if (dspdevice == NULL)
    {
        cl_device_id* devices;
        cl_uint       num_devices = 0;
        cl_int        errcode;

        errcode = clGetDeviceIDs((cl_platform_id) &the_platform::Instance(), CL_DEVICE_TYPE_ACCELERATOR,
                                 0, NULL, &num_devices);
        if (!num_devices)  return NULL;

        devices = (cl_device_id*) malloc(num_devices * sizeof(cl_device_id));
        if (!devices)  return NULL;

        errcode = clGetDeviceIDs((cl_platform_id) &the_platform::Instance(), CL_DEVICE_TYPE_ACCELERATOR,
                                 num_devices, devices, 0);
        if (errcode != CL_SUCCESS) { free (devices); return NULL; }

        dspdevice = (Coal::DSPDevice *)pobj(devices[0]);
        free(devices);
    }

    return dspdevice;
}

// context is ignored for now.  TODO: get device from context if not NULL
static void*
clMalloc(size_t size, cl_mem_flags flags, cl_context d_context)
{
    Coal::DSPDevice *dspdevice = getDspDevice();
    if (dspdevice != NULL)
    {
        tiocl::MemoryRange::Location l = dspdevice->ClFlagToLocation(flags);
        return dspdevice->GetSHMHandler()->clMalloc(size, l);
    }
    return NULL;
}

static void
clFree(void *p, cl_context d_context)
{
    Coal::DSPDevice *dspdevice = getDspDevice();
    if (dspdevice != NULL)  dspdevice->GetSHMHandler()->clFree(p);
}

void *
__malloc_ddr(size_t size)
{
    return clMalloc(size, 0, NULL);
}

void
__free_ddr(void *p)
{
    clFree(p, NULL);
}

void *
__malloc_msmc(size_t size)
{
    return clMalloc(size, CL_MEM_USE_MSMC_TI, NULL);
}

void
__free_msmc(void *p)
{
    clFree(p, NULL);
}

int
__is_in_malloced_region(void *p)
{
    Coal::DSPDevice *dspdevice = getDspDevice();
    return dspdevice->isInClMallocedRegion(p);
}

uint64_t __device_malloc(int32_t dsp, size_t size)
{
    Coal::DSPDevice  *device = getDspDevice();
    if (device == NULL)
        return 0;

    uint64_t t = device->GetSHMHandler()->AllocateGlobal(size, true);

    return t;
}

void __device_free(int32_t dsp, uint64_t addr)
{
    Coal::DSPDevice  *device = getDspDevice();
    if (device == NULL)
        return;

    device->GetSHMHandler()->FreeGlobal(addr);
}


}  // End: extern "C"

