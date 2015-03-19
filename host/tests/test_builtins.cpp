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
#include <cstdlib>

#include "test_builtins.h"
#include "CL/cl.h"

#include <stdint.h>

const char sampler_source[] =
    "__kernel void test_case(__global uint *rs, sampler_t sampler) {\n"
    "   sampler_t good_sampler = CLK_NORMALIZED_COORDS_TRUE |\n"
    "                            CLK_ADDRESS_MIRRORED_REPEAT |\n"
    "                            CLK_FILTER_NEAREST;\n"
    "\n"
    "   if (sampler != good_sampler) *rs = 1;\n"
    "}\n";

const char barrier_source[] =
    "__kernel void test_case(__global uint *rs) {\n"
    "   *rs = 0;\n"
    "   int i; for (i=0; i<3; i++) barrier(0);\n"
    "   *rs += 1;\n"
    "}\n";

const char image_source[] =
    "__kernel void test_case(__global uint *rs, __write_only image2d_t image1,\n"
    "                                           __write_only image2d_t image2,\n"
    "                                           __read_only image2d_t image3) {\n"
    "   float4 fcolor;\n"
    "   int4 scolor;\n"
    "   int2 coord;\n"
    "   sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |\n"
    "                       CLK_ADDRESS_MIRRORED_REPEAT |\n"
    "                       CLK_FILTER_NEAREST;\n"
    "\n"
    "   if (get_image_width(image1) != 4) *rs = 1;\n"
    "   if (get_image_height(image1) != 4) *rs = 2;\n"
    "   if (get_image_channel_data_type(image2) != CLK_SIGNED_INT16) *rs = 3;\n"
    "   if (get_image_channel_order(image2) != CLK_RGBA) *rs = 4;\n"
    "\n"
    "   if (*rs != 0) return;\n"
    "\n"
    "   fcolor.x = 1.0f;\n"
    "   fcolor.y = 0.5f;\n"
    "   fcolor.z = 0.0f;\n"
    "   fcolor.w = 1.0f;\n"
    "\n"
    "   scolor.x = -3057;\n"
    "   scolor.y = 65;\n"
    "   scolor.z = 0;\n"
    "   scolor.w = 32767;\n"
    "\n"
    "   coord.x = 3;\n"
    "   coord.y = 1;\n"
    "\n"
    "   write_imagef(image1, coord, fcolor);\n"
    "   write_imagei(image2, coord, scolor);\n"
    "\n"
    "   coord.x = 1;\n"
    "   coord.y = 1;\n"
    "   fcolor = read_imagef(image3, 0, coord);\n"
    "   if (fcolor.x < 0.99f || fcolor.y < 0.99f || fcolor.z > 0.01f ||\n"
    "       fcolor.w > 0.01f) { *rs = 5; return; }\n"
    "\n"
    "   float2 fcoords;\n"
    "   fcoords.x = 0.31f;\n"
    "   fcoords.y = 0.1415f;\n"
    "   fcolor = read_imagef(image3, sampler, fcoords);\n"
    "}\n";

const char builtins_source[] =
    "__kernel void test_case(__global uint *rs) {\n"
    "   float2 f2;\n"
    "   float2 f2b;\n"
    "\n"
    "   f2.x = 1.0f;\n"
    "   f2.y = 0.0f;\n"
    "   f2b.x = -0.5f;\n"
    "   f2b.y = (float)M_PI;\n"
    "\n"
    "   if (cos(f2).y != 1.0f) { *rs = 1; return; }\n"
    "   if (cos(0.0f) != 1.0f) { *rs = 2; return; }\n"
    "   if (copysign(1.0f, -0.5f) != -1.0f) { *rs = 3; return; }\n"
    "   if (copysign(f2, f2b).x != -1.0f) { *rs = 4; return; }\n"
    "   if (exp2(3.0f) != 8.0f) { *rs = 5; return; }\n"
    "}\n";

enum TestCaseKind
{
    NormalKind,
    SamplerKind,
    BarrierKind,
    ImageKind
};

/*
 * To ease testing, each kernel will be a Task kernel taking a pointer to an
 * integer and running built-in functions. If an error is encountered, the
 * integer pointed to by the arg will be set accordingly. If the kernel succeeds,
 * this integer is set to 0.
 */
static uint32_t run_kernel(const char *source, TestCaseKind kind)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_command_queue queue;
    cl_program program;
    cl_int result;
    cl_kernel kernel;
    cl_event event;
    cl_mem rs_buf;

    cl_sampler sampler;
    cl_mem mem1, mem2, mem3;
    cl_image_format fmt;

    unsigned char image2d_data[3*3*4] = {
        255, 0, 0, 0,       0, 255, 0, 0,       128, 128, 128, 0,
        0, 0, 255, 0,       255, 255, 0, 0,     0, 128, 0, 0,
        255, 128, 0, 0,     128, 0, 255, 0,     0, 0, 0, 0
    };

    uint32_t rs = 0;

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, 0);
    if (result != CL_SUCCESS) return 65536;

    ctx = clCreateContext(0, 1, &device, 0, 0, &result);
    if (result != CL_SUCCESS) return 65537;

    queue = clCreateCommandQueue(ctx, device, 0, &result);
    if (result != CL_SUCCESS) return 65538;

    program = clCreateProgramWithSource(ctx, 1, &source, 0, &result);
    if (result != CL_SUCCESS) return 65539;

    result = clBuildProgram(program, 1, &device, "", 0, 0);
    if (result != CL_SUCCESS)
    {
        // Print log
        char *log = 0;
        size_t len = 0;

        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, 0, &len);
        log = (char *)std::malloc(len);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len, log, 0);

        std::cout << log << std::endl;
        std::free(log);

        return 65540;
    }

    kernel = clCreateKernel(program, "test_case", &result);
    if (result != CL_SUCCESS) return 65541;

    // Create the result buffer
    rs_buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                            sizeof(rs), &rs, &result);
    if (result != CL_SUCCESS) return 65542;

    result = clSetKernelArg(kernel, 0, sizeof(cl_mem), &rs_buf);
    if (result != CL_SUCCESS) return 65543;

    // Kind
    switch (kind)
    {
        case NormalKind:
            break;

        case SamplerKind:
            sampler = clCreateSampler(ctx, 1, CL_ADDRESS_MIRRORED_REPEAT, CL_FILTER_NEAREST, &result);
            if (result != CL_SUCCESS) return 65546;

            result = clSetKernelArg(kernel, 1, sizeof(cl_sampler), &sampler);
            if (result != CL_SUCCESS) return 65547;
            break;

        case ImageKind:
            fmt.image_channel_data_type = CL_UNORM_INT8;
            fmt.image_channel_order = CL_RGBA;

            mem1 = clCreateImage2D(ctx, CL_MEM_WRITE_ONLY, &fmt, 4, 4, 0, 0, &result);
            if (result != CL_SUCCESS) return 65548;

            mem3 = clCreateImage2D(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   &fmt, 3, 3, 0, image2d_data, &result);
            if (result != CL_SUCCESS) return 65548;

            fmt.image_channel_data_type = CL_SIGNED_INT16;

            mem2 = clCreateImage2D(ctx, CL_MEM_WRITE_ONLY, &fmt, 4, 4, 0, 0, &result);
            if (result != CL_SUCCESS) return 65548;

            result = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem1);
            if (result != CL_SUCCESS) return 65549;

            result = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem2);
            if (result != CL_SUCCESS) return 65549;

            result = clSetKernelArg(kernel, 3, sizeof(cl_mem), &mem3);
            if (result != CL_SUCCESS) return 65549;
            break;

        default:
            break;
    }

    if (kind == BarrierKind)
    {
        size_t local_size = 64;
        size_t global_size = 64;

        result = clEnqueueNDRangeKernel(queue, kernel, 1, 0, &global_size,
                                        &local_size, 0, 0, &event);
        if (result != CL_SUCCESS) return 65544;
    }
    else
    {
        result = clEnqueueTask(queue, kernel, 0, 0, &event);
        if (result != CL_SUCCESS) return 65544;
    }

    result = clWaitForEvents(1, &event);
    if (result != CL_SUCCESS) return 65545;

    if (kind == SamplerKind) clReleaseSampler(sampler);
    if (kind == ImageKind)
    {
        clReleaseMemObject(mem1);
        clReleaseMemObject(mem2);
        clReleaseMemObject(mem3);
    }
    clReleaseEvent(event);
    clReleaseMemObject(rs_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);

    return rs;
}

static const char *default_error(uint32_t errcode)
{
    switch (errcode)
    {
        case 0:
            return 0;
        case 65536:
            return "Cannot get a device ID";
        case 65537:
            return "Cannot create a context";
        case 65538:
            return "Cannot create a command queue";
        case 65539:
            return "Cannot create a program with given source";
        case 65540:
            return "Cannot build the program";
        case 65541:
            return "Cannot create the test_case kernel";
        case 65542:
            return "Cannot create a buffer holding a uint32_t";
        case 65543:
            return "Cannot set kernel argument";
        case 65544:
            return "Cannot enqueue the kernel";
        case 65545:
            return "Cannot wait for the event";
        case 65546:
            return "Cannot create a sampler";
        case 65547:
            return "Cannot set a sampler kernel argument";
        case 65548:
            return "Cannot create an Image2D object";
        case 65549:
            return "Cannot set image kernel argument";

        default:
            return "Unknown error code";
    }
}

START_TEST (test_sampler)
{
    uint32_t rs = run_kernel(sampler_source, SamplerKind);
    const char *errstr = 0;

    switch (rs)
    {
        case 1:
            errstr = "Sampler bitfield invalid";
            break;
        default:
            errstr = default_error(rs);
    }

    fail_if(
        errstr != 0,
        errstr
    );
}
END_TEST

START_TEST (test_barrier)
{
    uint32_t rs = run_kernel(barrier_source, BarrierKind);

    fail_if(
        rs != 0x40,
        default_error(rs)
    );
}
END_TEST

START_TEST (test_image)
{
    uint32_t rs = run_kernel(image_source, ImageKind);
    const char *errstr = 0;

    switch (rs)
    {
        case 1:
            errstr = "Image1 must have width of 4";
            break;
        case 2:
            errstr = "Image1 must have width of 4";
            break;
        case 3:
            errstr = "Image2 must have type SIGNED_FLOAT16";
            break;
        case 4:
            errstr = "Image2 must have channel order RGBA";
            break;
        case 5:
            errstr = "The value read from the image is not good";
            break;
        default:
            errstr = default_error(rs);
    }

    fail_if(
        errstr != 0,
        errstr
    );
}
END_TEST

START_TEST (test_builtins)
{
    uint32_t rs = run_kernel(builtins_source, NormalKind);
    const char *errstr = 0;

    switch (rs)
    {
        case 1:
            errstr = "float2 cos(float2) doesn't behave correctly";
            break;
        case 2:
            errstr = "float cos(float) doesn't behave correctly";
            break;
        case 3:
            errstr = "float copysign(float) doesn't behave correctly";
            break;
        case 4:
            errstr = "float2 copysign(float2) doesn't behave correctly";
            break;
        case 5:
            errstr = "exp2() doesn't behave correctly";
            break;
        default:
            errstr = default_error(rs);
    }

    fail_if(
        errstr != 0,
        errstr
    );
}
END_TEST

TCase *cl_builtins_tcase_create(void)
{
    TCase *tc = NULL;
    tc = tcase_create("builtins");
    tcase_add_test(tc, test_sampler);
    tcase_add_test(tc, test_barrier);
    tcase_add_test(tc, test_image);
    tcase_add_test(tc, test_builtins);
    return tc;
}
