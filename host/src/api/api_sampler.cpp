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
 * \file api_sampler.cpp
 * \brief Samplers
 */
#ifdef _SYS_BIOS
#include <xdc/std.h>
#endif
#include "CL/cl.h"

#include "core/sampler.h"
#include "core/context.h"

// Sampler APIs
cl_sampler
clCreateSampler(cl_context          context,
                cl_bool             normalized_coords,
                cl_addressing_mode  addressing_mode,
                cl_filter_mode      filter_mode,
                cl_int *            errcode_ret)
{
    cl_int dummy_errcode;

    if (!errcode_ret)
        errcode_ret = &dummy_errcode;

    if (!context->isA(Coal::Object::T_Context))
    {
        *errcode_ret = CL_INVALID_CONTEXT;
        return 0;
    }

    *errcode_ret = CL_SUCCESS;

    Coal::Sampler *sampler = new Coal::Sampler((Coal::Context *)context,
                                               normalized_coords,
                                               addressing_mode,
                                               filter_mode,
                                               errcode_ret);

    if (*errcode_ret != CL_SUCCESS)
    {
        delete sampler;
        return 0;
    }

    return (cl_sampler)sampler;
}

cl_int
clRetainSampler(cl_sampler sampler)
{
    if (!sampler->isA(Coal::Object::T_Sampler))
        return CL_INVALID_SAMPLER;

    sampler->reference();

    return CL_SUCCESS;
}

cl_int
clReleaseSampler(cl_sampler sampler)
{
    if (!sampler->isA(Coal::Object::T_Sampler))
        return CL_INVALID_SAMPLER;

    if (sampler->dereference())
        delete sampler;

    return CL_SUCCESS;
}

cl_int
clGetSamplerInfo(cl_sampler         sampler,
                 cl_sampler_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret)
{
    if (!sampler->isA(Coal::Object::T_Sampler))
        return CL_INVALID_SAMPLER;

    return sampler->info(param_name, param_value_size, param_value,
                         param_value_size_ret);
}
