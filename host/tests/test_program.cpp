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

#include "test_program.h"
#include "CL/cl.h"

#include <cstdlib>
#include <cstring>

const char program_source[] =
    "#warning We need that line\n"
    "\n"
    "__kernel void test(__global float4 *a, __global float4 *b) {\n"
    "   int i = get_global_id(0);\n"
    "\n"
    "   a[i].xwyz = 3.1415926f * b[0].xyzw * b[0].wzyx;\n"
    "}\n";

START_TEST (test_create_program)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_program program;
    cl_int result;

    const char *src = program_source;
    size_t program_len = sizeof(program_source);

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

    program = clCreateProgramWithSource(ctx, 0, &src, 0, &result);
    fail_if(
        result != CL_INVALID_VALUE,
        "count cannot be 0"
    );

    program = clCreateProgramWithSource(ctx, 1, 0, 0, &result);
    fail_if(
        result != CL_INVALID_VALUE,
        "strings cannot be NULL"
    );

    program = clCreateProgramWithSource(ctx, 1, &src, &program_len,
                                        &result);
    fail_if(
        result != CL_SUCCESS,
        "cannot create a program from source with sane arguments"
    );

    clReleaseProgram(program); // Sorry

    program = clCreateProgramWithSource(ctx, 1, &src, 0, &result);
    fail_if(
        result != CL_SUCCESS,
        "lengths can be NULL and it must work"
    );

    result = clBuildProgram(program, 1, &device, "", 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot build a valid program"
    );

    clReleaseProgram(program);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_program_binary)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_program program;
    cl_int result, binary_status;

    const char *src = program_source;
    size_t program_len = sizeof(program_source);

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

    size_t binary_size = 0;
    unsigned char *binary_data = 0;

    result = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t),
                              (void *)&binary_size, 0);
    fail_if(
        result != CL_SUCCESS || binary_size == 0,
        "cannot get the binary size of the program"
    );

    binary_data = (unsigned char *)std::malloc(binary_size);

    result = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char *),
                              (void *)&binary_data, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot get the program's binary"
    );

    clReleaseProgram(program);

    program = clCreateProgramWithBinary(ctx, 1, &device, &binary_size,
                                        (const unsigned char **)&binary_data,
                                        &binary_status, &result);
    fail_if(
        result != CL_SUCCESS || binary_status != CL_SUCCESS,
        "cannot create a program from a previously-built binary"
    );

    clReleaseProgram(program);
    clReleaseContext(ctx);
}
END_TEST

START_TEST (test_program_build_info)
{
    cl_platform_id platform = 0;
    cl_device_id device;
    cl_context ctx;
    cl_program program;
    cl_int result;

    const char *src = program_source;

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

    program = clCreateProgramWithSource(ctx, 1, &src, 0, &result);
    fail_if(
        result != CL_SUCCESS,
        "lengths can be NULL and it must work"
    );

    result = clBuildProgram(program, 1, &device, "", 0, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot build a valid program"
    );

    char *log = 0;
    size_t log_len;

    result = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                   0, 0, &log_len);
    fail_if(
        result != CL_SUCCESS,
        "cannot get the build log len"
    );
    fail_if(
        log_len == 0,
        "we put a warning in the source, log cannot be of lentgth 0"
    );

    log = (char *)std::malloc(log_len);

    result = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                   log_len, log, 0);
    fail_if(
        result != CL_SUCCESS,
        "cannot get the build log"
    );
    fail_if(
        !std::strstr(log, "We need that line"),
        "the build log doesn't contain the warning found in the source"
    );

    clReleaseProgram(program);
    clReleaseContext(ctx);
}
END_TEST

TCase *cl_program_tcase_create(void)
{
    TCase *tc = NULL;
    tc = tcase_create("program");
    tcase_add_test(tc, test_create_program);
    tcase_add_test(tc, test_program_binary);
    tcase_add_test(tc, test_program_build_info);
    return tc;
}
