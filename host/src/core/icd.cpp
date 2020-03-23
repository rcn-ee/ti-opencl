/******************************************************************************
 * Copyright (c) 2011-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "CL/TI/cl.h"
#include "platform.h"
#include "icd.h"

// Note: This must match the OCL 1.2 disptach table in the file icd/icd_dispatch.h
// which is part of the ICD loader source from the Khronos website.
// Most recently: https://www.khronos.org/registry/cl/specs/opencl-icd-1.2.11.0.tgz

void * dispatch_table[] = 
{
    (void*)   clGetPlatformIDs,
    (void*)   clGetPlatformInfo,
    (void*)   clGetDeviceIDs,
    (void*)   clGetDeviceInfo,
    (void*)   clCreateContext,
    (void*)   clCreateContextFromType,
    (void*)   clRetainContext,
    (void*)   clReleaseContext,
    (void*)   clGetContextInfo,
    (void*)   clCreateCommandQueue,
    (void*)   clRetainCommandQueue,
    (void*)   clReleaseCommandQueue,
    (void*)   clGetCommandQueueInfo,
    (void*)   0, //clSetCommandQueueProperty,
    (void*)   clCreateBuffer,
    (void*)   clCreateImage2D,
    (void*)   clCreateImage3D,
    (void*)   clRetainMemObject,
    (void*)   clReleaseMemObject,
    (void*)   clGetSupportedImageFormats,
    (void*)   clGetMemObjectInfo,
    (void*)   clGetImageInfo,
    (void*)   clCreateSampler,
    (void*)   clRetainSampler,
    (void*)   clReleaseSampler,
    (void*)   clGetSamplerInfo,
    (void*)   clCreateProgramWithSource,
    (void*)   clCreateProgramWithBinary,
    (void*)   clRetainProgram,
    (void*)   clReleaseProgram,
    (void*)   clBuildProgram,
    (void*)   clUnloadCompiler,
    (void*)   clGetProgramInfo,
    (void*)   clGetProgramBuildInfo,
    (void*)   clCreateKernel,
    (void*)   clCreateKernelsInProgram,
    (void*)   clRetainKernel,
    (void*)   clReleaseKernel,
    (void*)   clSetKernelArg,
    (void*)   clGetKernelInfo,
    (void*)   clGetKernelWorkGroupInfo,
    (void*)   clWaitForEvents,
    (void*)   clGetEventInfo,
    (void*)   clRetainEvent,
    (void*)   clReleaseEvent,
    (void*)   clGetEventProfilingInfo,
    (void*)   clFlush,
    (void*)   clFinish,
    (void*)   clEnqueueReadBuffer,
    (void*)   clEnqueueWriteBuffer,
    (void*)   clEnqueueCopyBuffer,
    (void*)   clEnqueueReadImage,
    (void*)   clEnqueueWriteImage,
    (void*)   clEnqueueCopyImage,
    (void*)   clEnqueueCopyImageToBuffer,
    (void*)   clEnqueueCopyBufferToImage,
    (void*)   clEnqueueMapBuffer,
    (void*)   clEnqueueMapImage,
    (void*)   clEnqueueUnmapMemObject,
    (void*)   clEnqueueNDRangeKernel,
    (void*)   clEnqueueTask,
    (void*)   clEnqueueNativeKernel,
    (void*)   clEnqueueMarker,
    (void*)   clEnqueueWaitForEvents,
    (void*)   clEnqueueBarrier,
    (void*)   clGetExtensionFunctionAddress,
    (void*)   0, //clCreateFromGLBuffer,
    (void*)   0, //clCreateFromGLTexture2D,
    (void*)   0, //clCreateFromGLTexture3D,
    (void*)   0, //clCreateFromGLRenderbuffer,
    (void*)   0, //clGetGLObjectInfo,
    (void*)   0, //clGetGLTextureInfo,
    (void*)   0, //clEnqueueAcquireGLObjects,
    (void*)   0, //clEnqueueReleaseGLObjects,
    (void*)   0, //clGetGLContextInfoKHR,

    (void*)   0, //clGetDeviceIDsFromD3D10KHR,
    (void*)   0, //clCreateFromD3D10BufferKHR,
    (void*)   0, //clCreateFromD3D10Texture2DKHR,
    (void*)   0, //clCreateFromD3D10Texture3DKHR,
    (void*)   0, //clEnqueueAcquireD3D10ObjectsKHR,
    (void*)   0, //clEnqueueReleaseD3D10ObjectsKHR,

    (void*)   clSetEventCallback,
    (void*)   clCreateSubBuffer,
    (void*)   clSetMemObjectDestructorCallback,
    (void*)   clCreateUserEvent,
    (void*)   clSetUserEventStatus,
    (void*)   clEnqueueReadBufferRect,
    (void*)   clEnqueueWriteBufferRect,
    (void*)   clEnqueueCopyBufferRect,

    (void*)   0, //clCreateSubDevicesEXT;
    (void*)   0, //clRetainDeviceEXT;
    (void*)   0, //clReleaseDeviceEXT;

    (void *)  0, //clCreateEventFromGLsyncKHR;

#if 0 // OpenCL 1.2
    (void *)  0, //clCreateSubDevices,
    (void *)  0, //clRetainDevice,
    (void *)  0, //clReleaseDevice,
    (void *)  0, //clCreateImage,
    (void *)  0, // clCreateProgramWithBuiltInKernels;
    (void *)  0, //clCompileProgram,
    (void *)  0, //clLinkProgram,
    (void *)  0, //clUnloadPlatformCompiler,
    (void *)  0, //clGetKernelArgInfo,
    (void *)  0, //clEnqueueFillBuffer,
    (void *)  0, // clEnqueueFillImage;
    (void *)  0, //clEnqueueMigrateMemObjects,
    (void *)  0, //clEnqueueMarkerWithWaitList,
    (void *)  0, //clEnqueueBarrierWithWaitList,
    (void *)  0, //clGetExtensionFunctionAddressForPlatform,
    (void *)  0, // clCreateFromGLTexture;

    (void *)  0, // clGetDeviceIDsFromD3D11KHR;
    (void *)  0, // clCreateFromD3D11BufferKHR;
    (void *)  0, // clCreateFromD3D11Texture2DKHR;
    (void *)  0, // clCreateFromD3D11Texture3DKHR;
    (void *)  0, // clCreateFromDX9MediaSurfaceKHR;
    (void *)  0, // clEnqueueAcquireD3D11ObjectsKHR;
    (void *)  0, // clEnqueueReleaseD3D11ObjectsKHR;

    (void *)  0, // clGetDeviceIDsFromDX9MediaAdapterKHR;
    (void *)  0, // clEnqueueAcquireDX9MediaSurfacesKHR;
    (void *)  0, // clEnqueueReleaseDX9MediaSurfacesKHR;
#endif
};


cl_int CL_API_CALL
clIcdGetPlatformIDsKHR(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms)
{
    if (num_platforms) *num_platforms = 1;
    else if (!platforms) return CL_INVALID_VALUE;

    if (!num_entries && platforms) return CL_INVALID_VALUE;

    /*-------------------------------------------------------------------------
    * Only one "default" platform
    *------------------------------------------------------------------------*/
    if (platforms != 0) *platforms = (cl_platform_id) &the_platform::Instance();

    return CL_SUCCESS;
}
