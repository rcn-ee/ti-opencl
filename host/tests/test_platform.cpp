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

#include "test_platform.h"

#include "CL/cl.h"

START_TEST (test_get_platform_ids)
{
    cl_platform_id platform = 0;
    cl_uint num_platforms = 0;
    cl_int result = 0;

    result = clGetPlatformIDs(0, &platform, &num_platforms);
    fail_if(
        result != CL_INVALID_VALUE,
        "num_entries cannot be NULL when *platforms is provided"
    );

    result = clGetPlatformIDs(0, 0, 0);
    fail_if(
        result != CL_INVALID_VALUE,
        "Both num_platforms and *platforms cannot be NULL at the same time"
    );

    result = clGetPlatformIDs(1, 0, &num_platforms);
    fail_if(
        result != CL_SUCCESS || num_platforms == 0,
        "When *platforms is NULL, success and put the number of platforms in num_platforms"
    );

    result = clGetPlatformIDs(1, &platform, &num_platforms);
    fail_if(
        result != CL_SUCCESS,
        "It's bad to fail when the function is used in the most common sense"
    );
}
END_TEST

START_TEST (test_get_platform_info)
{
    cl_platform_id platform = 0;
    cl_uint num_platforms = 0;
    cl_int result = 0;
    char *buf[100];
    size_t buf_len = 0;

    result = clGetPlatformIDs(1, &platform, &num_platforms);
    fail_if(
        result != CL_SUCCESS,
        "It's bad to fail when the function is used in the most common sense"
    );

    result = clGetPlatformInfo((_cl_platform_id *) -1, CL_PLATFORM_PROFILE, sizeof(buf), buf, &buf_len);
    fail_if(
        result != CL_INVALID_PLATFORM,
        "-1 is not a valid platform"
    );

    result = clGetPlatformInfo(platform, 1337, sizeof(buf), buf, &buf_len);
    fail_if(
        result != CL_INVALID_VALUE,
        "1337 is not a valid param_name"
    );

    result = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, buf, &buf_len);
    fail_if(
        result != CL_INVALID_VALUE,
        "param_value_size cannot be NULL when a buffer is supplied"
    );

    result = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, 0, 0, &buf_len);
    fail_if(
        result != CL_SUCCESS || buf_len == 0,
        "We are allowed not to pass a buffer. The function must fill param_value_size_ret"
    );

    result = clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, sizeof(buf), buf, &buf_len);
    fail_if(
        result != CL_SUCCESS || buf_len == 0,
        "It's bad to fail when the function is used in the most common sense"
    );
}
END_TEST

TCase *cl_platform_tcase_create(void)
{
    TCase *tc = NULL;
    tc = tcase_create("platform");
    tcase_add_test(tc, test_get_platform_ids);
    tcase_add_test(tc, test_get_platform_info);
    return tc;
}
