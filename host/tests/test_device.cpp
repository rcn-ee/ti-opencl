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

#include "test_device.h"
#include "CL/cl.h"

START_TEST (test_get_device_ids)
{
    cl_device_id device;
    cl_uint num_devices;
    cl_int result;

    cl_platform_id platform      = 0;
    cl_uint        num_platforms = 0;
    clGetPlatformIDs(1, &platform, &num_platforms);

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, &device, &num_devices);
    fail_if(
        result != CL_INVALID_VALUE,
        "num_entries cannot be NULL when devices is not null"
    );

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, 0, 0);
    fail_if(
        result != CL_INVALID_VALUE,
        "num_devices and devices cannot be NULL at the same time"
    );

    result = clGetDeviceIDs((cl_platform_id)1337, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, &num_devices);
    fail_if(
        result != CL_INVALID_PLATFORM,
        "1337 is not a valid platform"
    );

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);
    fail_if(
        result != CL_DEVICE_NOT_FOUND,
        "there are no GPU devices currently available"
    );

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, 0, &num_devices);
    fail_if(
        result != CL_SUCCESS || num_devices != 1,
        "we must succeed and say that we have one ACCELERATOR device"
    );

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, &num_devices);
    fail_if(
        result != CL_SUCCESS || num_devices != 1 || device == 0,
        "we must succeed and have one ACCELERATOR device"
    );
}
END_TEST

START_TEST (test_get_device_info)
{
    cl_device_id device;
    cl_uint num_devices;
    cl_int result;

    cl_platform_id platform      = 0;
    cl_uint        num_platforms = 0;
    clGetPlatformIDs(1, &platform, &num_platforms);

    size_t size_ret;
    char value[500];

    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, &num_devices);
    fail_if(
        result != CL_SUCCESS,
        "unable to get a ACCELERATOR device"
    );

    result = clGetDeviceInfo(0, CL_DEVICE_TYPE, 500, value, &size_ret);
    fail_if(
        result != CL_INVALID_DEVICE,
        "0 is not a valid device"
    );

    result = clGetDeviceInfo(device, 13376334, 500, value, &size_ret);
    fail_if(
        result != CL_INVALID_VALUE,
        "13376334 is not a valid param_name"
    );

    result = clGetDeviceInfo(device, CL_DEVICE_TYPE, 1, value, &size_ret);
    fail_if(
        result != CL_INVALID_VALUE,
        "1 is too small to contain a cl_device_type"
    );

    result = clGetDeviceInfo(device, CL_DEVICE_TYPE, 0, 0, &size_ret);
    fail_if(
        result != CL_SUCCESS || size_ret != sizeof(cl_device_type),
        "we have to succeed and to say that the result is a cl_device_type"
    );

    result = clGetDeviceInfo(device, CL_DEVICE_TYPE, 500, value, &size_ret);
    fail_if(
        result != CL_SUCCESS || *(cl_device_type*)(value) != CL_DEVICE_TYPE_ACCELERATOR,
        "we have to say the device is a ACCELERATOR"
    );

    result = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 500, value, &size_ret);
    fail_if(
        result != CL_SUCCESS,
        "we must succeed"
    );
    fail_if(
        strncmp(value, "Texas Instruments, Inc.", size_ret) != 0,
        "the device vendor must be \"Texas Instruments, Inc.\""
    );
}
END_TEST

TCase *cl_device_tcase_create(void)
{
    TCase *tc = NULL;
    tc = tcase_create("device");
    tcase_add_test(tc, test_get_device_ids);
    tcase_add_test(tc, test_get_device_info);
    return tc;
}
