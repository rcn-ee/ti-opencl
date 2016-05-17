/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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

#include "configuration.h"

#include <xdc/std.h>
#include "package/internal/OpenCL.xdc.h"

const char* ti_opencl_getComputeUnitList()
{
    return OpenCL_computeUnitList;
}

uint32_t ti_opencl_get_OCL_GLOBAL_base()
{
    return OpenCL_OCL_GLOBAL_base;
}

uint32_t ti_opencl_get_OCL_GLOBAL_len()
{
    return OpenCL_OCL_GLOBAL_len;
}

uint32_t ti_opencl_get_OCL_HOSTMEM_base()
{
    return OpenCL_OCL_HOSTPROG_base;
}

uint32_t ti_opencl_get_OCL_HOSTMEM_len()
{
    return OpenCL_OCL_HOSTPROG_len;
}

uint32_t ti_opencl_get_OCL_LOCAL_base()
{
    return OpenCL_OCL_LOCAL_base;
}

uint32_t ti_opencl_get_OCL_LOCAL_len()
{
    return OpenCL_OCL_LOCAL_len;
}

