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
 * \file api_platform.cpp
 * \brief Platform
 */

#include "CL/cl.h"
#include "api_platform.h"
#include <cstring>
#include <core/config.h>

static const char platform_profile[]    = "FULL_PROFILE";
static const char platform_version[]    = "OpenCL 1.1 TI " COAL_VERSION;
static const char platform_name[]       = "TI OpenCL for Advantech DSPC-8681 Ubuntu 10.04";
static const char platform_vendor[]     = "Texas Instruments, Inc.";
static const char platform_extensions[] = "cl_khr_fp64";


// Platform API

cl_int CL_API_CALL
clGetPlatformIDs(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms)
{
    if (num_platforms) *num_platforms = 1;
    else if (!platforms) return CL_INVALID_VALUE;

    if (!num_entries && platforms) return CL_INVALID_VALUE;

    /*-------------------------------------------------------------------------
    * Only one "default" platform
    *------------------------------------------------------------------------*/
    if (platforms != 0) *platforms = PLATFORM_ID;

    return CL_SUCCESS;
}

cl_int
clGetPlatformInfo(cl_platform_id   platform,
                  cl_platform_info param_name,
                  size_t           param_value_size,
                  void *           param_value,
                  size_t *         param_value_size_ret)
{
    const char *string = 0;
    unsigned long len = 0;

    /*-------------------------------------------------------------------------
    * NULL or what is returned by clGetPlatformIDs, that's to say also NULL
    *------------------------------------------------------------------------*/
    if (platform != PLATFORM_ID) return CL_INVALID_PLATFORM;

    switch (param_name) {
        case CL_PLATFORM_PROFILE:
            string = platform_profile;
            len = sizeof(platform_profile);
            break;

        case CL_PLATFORM_VERSION:
            string = platform_version;
            len = sizeof(platform_version);
            break;

        case CL_PLATFORM_NAME:
            string = platform_name;
            len = sizeof(platform_name);
            break;

        case CL_PLATFORM_VENDOR:
            string = platform_vendor;
            len = sizeof(platform_vendor);
            break;

        case CL_PLATFORM_EXTENSIONS:
            string = platform_extensions;
            len = sizeof(platform_extensions);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value_size < len && param_value != 0)
        return CL_INVALID_VALUE;

    if (param_value != 0)
        std::memcpy(param_value, string, len);

    if (param_value_size_ret)
        *param_value_size_ret = len;

    return CL_SUCCESS;
}
