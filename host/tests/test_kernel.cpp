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

#include <iostream>

#include "test_kernel.h"
#include "CL/cl.h"

static const char source[] =
    "float simple_function(float a) {\n"
    "    return a * 2.5f;\n"
    "}\n"
    "\n"
    "__kernel void kernel1(__global float *a, __global float *b, float f) {\n"
    "    size_t i = get_global_id(0);\n"
    "\n"
    "    a[i] = simple_function(f) * b[i];\n"
    "}\n"
    "\n"
    "__kernel void kernel2(__global unsigned int *buf) {\n"
    "    size_t i = get_global_id(0);\n"
    "\n"
    "    buf[i % 256] = 2 * (i % 256);\n"
    "}\n";

static void native_kernel(void *args)
{
    struct ags
    {
        size_t buffer_size;
        char *buffer;
    };

    struct ags *data = (struct ags *)args;

    // Not
    for (size_t i=0; i<data->buffer_size; ++i)
    {
        data->buffer[i] = ~data->buffer[i];
    }
}

START_TEST (test_compiled_kernel)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_program program;
    cl_int result;
    cl_kernel kernels[2];
    cl_uint num_kernels;
    cl_mem buf;

    const char *src = source;
    size_t program_len = sizeof(source);

    unsigned int buffer[256];

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
        result != CL_SUCCESS,
        "unable to create a command queue"
    );

    program = clCreateProgramWithSource(ctx, 1, &src, &program_len, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create a program from source with sane arguments"
    );

    result = clBuildProgram(program, 1, &device, "", 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot build a valid program"
    );

    kernels[0] = clCreateKernel(program, "simple_function", &result);
    fail_if(
        result != CL_INVALID_KERNEL_NAME,
        "simple_function is not a kernel"
    );

    kernels[0] = clCreateKernel(program, "kernel1", &result);
    fail_if(
        result != CL_SUCCESS,
        "unable to create a valid kernel"
    );

    clReleaseKernel(kernels[0]);    // Just born and already killed...

    result = clCreateKernelsInProgram(program, 0, 0, &num_kernels);
    fail_if(
        result != CL_SUCCESS || num_kernels != 2,
        "unable to get the number of kernels"
    );

    result = clCreateKernelsInProgram(program, 2, kernels, 0);
    fail_if(
        result != CL_SUCCESS,
        "unable to get the two kernels of the program"
    );

    // Try to run kernel2
    buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                         sizeof(buffer), buffer, &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create a valid CL_MEM_COPY_HOST_PTR read-write buffer"
    );

    result = clSetKernelArg(kernels[1], 0, sizeof(cl_mem), &buf);
    fail_if(
        result != CL_SUCCESS,
        "cannot set kernel argument"
    );

    size_t local_size = sizeof(buffer) / sizeof(buffer[0]);
    size_t global_size = 100000 * local_size;
    cl_event event;
    bool ok;

    result = clEnqueueNDRangeKernel(queue, kernels[1], 1, 0, &global_size, 0, 0, 0, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to queue a NDRange kernel with local work size guessed"
    );

    result = clWaitForEvents(1, &event);
    fail_if(
        result != CL_SUCCESS,
        "unable to wait for event"
    );

    ok = true;
    for (size_t i=0; i<local_size; ++i)
    {
        if (buffer[i] !=  2 * i)
        {
            ok = false;
            break;
        }
    }

    fail_if(
        ok == false,
        "the kernel hasn't done its job, the buffer is wrong"
    );

    clReleaseKernel(kernels[0]);
    clReleaseKernel(kernels[1]);
    clReleaseProgram(program);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_native_kernel)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_int result;
    cl_event events[2];
    cl_mem buf1, buf2;

    char s1[] = "Lorem ipsum dolor sit amet";
    char s2[] = "I want to tell you that you rock";

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

    queue = clCreateCommandQueue(ctx, device,
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &result);
    fail_if(
        result != CL_SUCCESS || queue == 0,
        "cannot create a command queue"
    );

    buf1 = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                          sizeof(s1), (void *)&s1, &result);
    fail_if(
        result != CL_SUCCESS || buf1 == 0,
        "cannot create a buffer"
    );

    buf2 = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                          sizeof(s2), (void *)&s2, &result);
    fail_if(
        result != CL_SUCCESS || buf2 == 0,
        "cannot create a buffer"
    );

    struct
    {
        size_t buffer_size;
        char *buffer;
    } args;

    args.buffer_size = sizeof(s1);
    args.buffer = 0;

    const void *mem_loc = (const void *)&args.buffer;

    result = clEnqueueNativeKernel(queue, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    fail_if(
        result != CL_INVALID_VALUE,
        "user_func cannot be NULL"
    );

    result = clEnqueueNativeKernel(queue, &native_kernel, 0, sizeof(args),
                                   1, &buf1, &mem_loc, 0, 0,
                                   &events[0]);
    fail_if(
        result != CL_INVALID_VALUE,
        "args cannot be NULL when cb_args != 0"
    );

    result = clEnqueueNativeKernel(queue, &native_kernel, &args, sizeof(args),
                                   1, 0, &mem_loc, 0, 0,
                                   &events[0]);
    fail_if(
        result != CL_INVALID_VALUE,
        "mem_list cannot be NULL when num_mem_objects != 0"
    );

    result = clEnqueueNativeKernel(queue, &native_kernel, &args, sizeof(args),
                                   1, &buf1, 0, 0, 0, &events[0]);
    fail_if(
        result != CL_INVALID_VALUE,
        "args_mem_loc cannot be NULL when num_mem_objects != 0"
    );

    result = clEnqueueNativeKernel(queue, &native_kernel, &args, sizeof(args),
                                   1, &buf1, &mem_loc, 0, 0,
                                   &events[0]);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue native kernel nr 1"
    );

    args.buffer_size = sizeof(s2);

    result = clEnqueueNativeKernel(queue, &native_kernel, &args, sizeof(args),
                                   1, &buf2, &mem_loc, 0, 0,
                                   &events[1]);
    fail_if(
        result != CL_SUCCESS,
        "unable to enqueue native kernel nr 2"
    );

    // Wait for events
    result = clWaitForEvents(2, events);
    fail_if(
        result != CL_SUCCESS,
        "unable to wait for events"
    );

    fail_if(
        s1[0] != ~'L' || s2[0] != ~'I',
        "the native kernel hasn't done its job"
    );

    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
END_TEST

TCase *cl_kernel_tcase_create(void)
{
    TCase *tc = NULL;
    tc = tcase_create("kernel");
    tcase_add_test(tc, test_native_kernel);
    tcase_add_test(tc, test_compiled_kernel);
    return tc;
}
