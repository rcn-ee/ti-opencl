/******************************************************************************
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>
#include <cassert>

#define NUM_EVE_DEVICES 4

using namespace cl;
using namespace std;

int main(int argc, char *argv[])
{
    cl_int errcode;

    // clGetPlatformIDs
    cl_platform_id platforms[1];
    cl_uint num_platforms;
    cout << "clGetPlatformIDs()\n";
    cout << "\tCL_SUCCESS(all correct args)..";
    errcode = clGetPlatformIDs(1, platforms, &num_platforms);
    assert(errcode == CL_SUCCESS);
    cout << num_platforms <<"..PASS\n";

    // clGetDeviceIDs
    cl_device_id devices[NUM_EVE_DEVICES];
    cl_uint num_devices;
    cout << "clGetDeviceIDs()\n";
    cout << "\tCL_SUCCESS(all correct args)..";
    errcode = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CUSTOM, NUM_EVE_DEVICES, devices, &num_devices);
    assert(errcode == CL_SUCCESS);
    cout << devices[0] << ", "
    	 << devices[1] << ", "
    	 << devices[2] << ", "
    	 << devices[3] << ".."
	 << num_devices << "..PASS\n";

    // clGetDeviceInfo
    char s[100];
    cl_uint value;
    cl_uint  param_value_size_ret;
    cout << "clGetDeviceInfo()\n";
    cout << "\tCL_DEVICE_TYPE(CL_DEVICE_TYPE_CUSTOM)..";
    errcode = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, 100, &value, &param_value_size_ret);
    assert(errcode == CL_SUCCESS);
    assert(value == CL_DEVICE_TYPE_CUSTOM);
    cout << "..PASS\n";
    cout << "\tCL_DEVICE_BUILT_IN_KERNELS..";
    errcode = clGetDeviceInfo(devices[0], CL_DEVICE_BUILT_IN_KERNELS, 100, s, &param_value_size_ret);
    assert(errcode == CL_SUCCESS);
    assert(strcmp(s, "tiocl_bik_memcpy_test;tiocl_bik_vecadd") == 0);
    cout << std::string(s) << "..PASS\n";

    // clCreateContext
    cout << "clCreateContext()\n";
    cout << "\tCL_SUCCESS(all correct args)..";
    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 0};
    cl_context context = clCreateContext(properties, NUM_EVE_DEVICES, devices, NULL, NULL, &errcode);
    assert(errcode == CL_SUCCESS);
    cout << "..PASS\n";

    // clCreateProgramWithBuiltInKernels
    cout << "clCreateProgramWithBuiltInKernels()\n";
    cl_program program;
    cout << "\tCL_INVALID_CONTEXT";
    program = clCreateProgramWithBuiltInKernels((cl_context)0, 1, devices, "tiocl_bik_vecadd", &errcode);
    assert(errcode == CL_INVALID_CONTEXT);
    cout << "..PASS\n";
    cout << "\tCL_INVALID_VALUE(devices=NULL)";
    program = clCreateProgramWithBuiltInKernels(context, 1, NULL, "tiocl_bik_vecadd", &errcode);
    assert(errcode == CL_INVALID_VALUE);
    cout << "..PASS\n";
    cout << "\tCL_INVALID_VALUE(num_devices=0)";
    program = clCreateProgramWithBuiltInKernels(context, 0, devices, "tiocl_bik_vecadd", &errcode);
    assert(errcode == CL_INVALID_VALUE);
    cout << "..PASS\n";
    cout << "\tCL_INVALID_VALUE(kernel_names=NULL)";
    program = clCreateProgramWithBuiltInKernels(context, 1, devices, NULL, &errcode);
    assert(errcode == CL_INVALID_VALUE);
    cout << "..PASS\n";
    cout << "\tCL_INVALID_VALUE(kernel_names=bad)";
    program = clCreateProgramWithBuiltInKernels(context, 1, devices, "bad", &errcode);
    assert(errcode == CL_INVALID_VALUE);
    cout << "..PASS\n";

    cout << "\tCL_SUCCESS(all correct args)..";
    program = clCreateProgramWithBuiltInKernels(context, 4, devices, "tiocl_bik_vecadd", &errcode);
    assert(errcode == CL_SUCCESS);
    cout << "..PASS\n";

    // clCreateKernel
    cout << "clCreateKernel()\n";
    cl_kernel kernel;
    cout << "\tCL_INVALID_PROGRAM(cl_program=0)..";
    kernel = clCreateKernel((cl_program)0, "tiocl_bik_vecadd", &errcode);
    assert(errcode == CL_INVALID_PROGRAM);
    cout << "..PASS\n";
    cout << "\tCL_INVALID_KERNEL_NAME(kernel_name=bad)..";
    kernel = clCreateKernel(program, "bad", &errcode);
    assert(errcode == CL_INVALID_KERNEL_NAME);
    cout << "..PASS\n";
    cout << "\tCL_INVALID_VALUE(kernel_name=NULL)..";
    kernel = clCreateKernel(program, NULL, &errcode);
    assert(errcode == CL_INVALID_VALUE);
    cout << "..PASS\n";

    cout << "\tCL_SUCCESS(all correct args)..";
    kernel = clCreateKernel(program, "tiocl_bik_vecadd", &errcode);
    assert(errcode == CL_SUCCESS);
    cout << "..PASS\n";

    cout << "All tests passed." << endl;

    return 0;
}
