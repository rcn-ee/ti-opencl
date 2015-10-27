/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2012-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#ifdef _SYS_BIOS
#include <xdc/std.h>
#endif
#include "CL/cl.h"
#include "CL/cl_ext.h"
#include <core/platform.h>
#include <core/config.h>
#include <cstring>

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
    if (platforms != 0) *platforms = (cl_platform_id) &the_platform::Instance();

    return CL_SUCCESS;
}

cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_id   platform,
                  cl_platform_info param_name,
                  size_t           param_value_size,
                  void *           param_value,
                  size_t *         param_value_size_ret)
{



    /*-------------------------------------------------------------------------
    * NULL or what is returned by clGetPlatformIDs, that's to say also NULL
    *------------------------------------------------------------------------*/
    if (platform != (cl_platform_id) &the_platform::Instance()) return CL_INVALID_PLATFORM;

    return platform->info(param_name, param_value_size, param_value,
                          param_value_size_ret);
}

/******************************************************************************
* Return a pointer to any supported extension functions
******************************************************************************/
void * clGetExtensionFunctionAddress(const char *funcname)
{
    if (strcmp(funcname, "clIcdGetPlatformIDsKHR") == 0)
        return (void*)clGetPlatformIDs;

    return NULL;
}

