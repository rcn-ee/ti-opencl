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
#include "CL/cl.h"
#include "platform.h"
#include "icd.h"

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
    (void*)   0, //clCreateSubDevicesEXT,
    (void*)   0, //clRetainDeviceEXT,
    (void*)   0, //clReleaseDeviceEXT
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
    if (platforms != 0) *platforms = &the_platform::Instance();

    return CL_SUCCESS;
}
