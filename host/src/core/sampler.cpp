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
 * \file core/sampler.cpp
 * \brief Sampler
 */

#include "sampler.h"
#include "context.h"
#include "deviceinterface.h"
#include "propertylist.h"

#include <cstring>
#include <cstdlib>

using namespace Coal;

Sampler::Sampler(Context *ctx,
                 cl_bool normalized_coords,
                 cl_addressing_mode addressing_mode,
                 cl_filter_mode filter_mode,
                 cl_int *errcode_ret)
: Object(Object::T_Sampler, ctx), p_bitfield(0)
{
    if (normalized_coords)
        p_bitfield |= CLK_NORMALIZED_COORDS_TRUE;
    else
        p_bitfield |= CLK_NORMALIZED_COORDS_FALSE;

    switch (addressing_mode)
    {
        case CL_ADDRESS_NONE:
            p_bitfield |= CLK_ADDRESS_NONE;
            break;

        case CL_ADDRESS_MIRRORED_REPEAT:
            p_bitfield |= CLK_ADDRESS_MIRRORED_REPEAT;
            break;

        case CL_ADDRESS_REPEAT:
            p_bitfield |= CLK_ADDRESS_REPEAT;
            break;

        case CL_ADDRESS_CLAMP_TO_EDGE:
            p_bitfield |= CLK_ADDRESS_CLAMP_TO_EDGE;
            break;

        case CL_ADDRESS_CLAMP:
            p_bitfield |= CLK_ADDRESS_CLAMP;
            break;

        default:
            *errcode_ret = CL_INVALID_VALUE;
            return;
    }

    switch (filter_mode)
    {
        case CL_FILTER_NEAREST:
            p_bitfield |= CLK_FILTER_NEAREST;
            break;

        case CL_FILTER_LINEAR:
            p_bitfield |= CLK_FILTER_LINEAR;
            break;

        default:
            *errcode_ret = CL_INVALID_VALUE;
            return;
    }

    // Check that images are available on all the devices
    *errcode_ret = checkImageAvailability();
}

Sampler::Sampler(Context *ctx, unsigned int bitfield)
: Object(Object::T_Sampler, ctx), p_bitfield(bitfield)
{
    checkImageAvailability();
}

cl_int Sampler::checkImageAvailability() const
{
    cl_uint num_devices;
    cl_device_id *d_devices;
    cl_int rs;

    rs = ((Context *)parent())->info(CL_CONTEXT_NUM_DEVICES,
                                     sizeof(unsigned int),
                                     &num_devices, 0);

    if (rs != CL_SUCCESS)
        return rs;

    d_devices = (cl_device_id*)std::malloc(num_devices * sizeof(cl_device_id));

    if (!d_devices)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    rs = ((Context *)parent())->info(CL_CONTEXT_DEVICES,
                                     num_devices * sizeof(cl_device_id),
                                     d_devices, 0);

    if (rs != CL_SUCCESS)
    {
        std::free((void *)d_devices);
        return rs;
    }

    for (unsigned int i=0; i<num_devices; ++i)
    {
        cl_bool image_support;

        rs = (pobj(d_devices[i]))->info(CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool),
                              &image_support, 0);

        if (rs != CL_SUCCESS)
        {
            std::free((void *)d_devices);
            return rs;
        }

        if (!image_support)
        {
            std::free((void *)d_devices);
            return CL_INVALID_OPERATION;
        }
    }

    std::free((void *)d_devices);

    return CL_SUCCESS;
}

unsigned int Sampler::bitfield() const
{
    return p_bitfield;
}

cl_int Sampler::info(cl_sampler_info param_name,
                     size_t param_value_size,
                     void *param_value,
                     size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union {
        cl_uint cl_uint_var;
        cl_context cl_context_var;
        cl_bool cl_bool_var;
        cl_addressing_mode cl_addressing_mode_var;
        cl_filter_mode cl_filter_mode_var;
    };

    switch (param_name)
    {
        case CL_SAMPLER_REFERENCE_COUNT:
            SIMPLE_ASSIGN(cl_uint, references());
            break;

        case CL_SAMPLER_CONTEXT:
            SIMPLE_ASSIGN(cl_context, desc((Context *)parent()));
            break;

        case CL_SAMPLER_NORMALIZED_COORDS:
            if (p_bitfield & CLK_NORMALIZED_COORDS_MASK)
                SIMPLE_ASSIGN(cl_bool, true)
            else
                SIMPLE_ASSIGN(cl_bool, false);
            break;

        case CL_SAMPLER_ADDRESSING_MODE:
            switch (p_bitfield & CLK_ADDRESS_MODE_MASK)
            {
                case CLK_ADDRESS_CLAMP:
                    SIMPLE_ASSIGN(cl_addressing_mode, CL_ADDRESS_CLAMP);
                    break;
                case CLK_ADDRESS_CLAMP_TO_EDGE:
                    SIMPLE_ASSIGN(cl_addressing_mode, CL_ADDRESS_CLAMP_TO_EDGE);
                    break;
                case CLK_ADDRESS_MIRRORED_REPEAT:
                    SIMPLE_ASSIGN(cl_addressing_mode, CL_ADDRESS_MIRRORED_REPEAT);
                    break;
                case CLK_ADDRESS_REPEAT:
                    SIMPLE_ASSIGN(cl_addressing_mode, CL_ADDRESS_REPEAT);
                    break;
                case CLK_ADDRESS_NONE:
                    SIMPLE_ASSIGN(cl_addressing_mode, CL_ADDRESS_NONE);
                    break;
            }
            break;

        case CL_SAMPLER_FILTER_MODE:
            switch (p_bitfield & CLK_FILTER_MASK)
            {
                case CLK_FILTER_LINEAR:
                    SIMPLE_ASSIGN(cl_filter_mode, CL_FILTER_LINEAR);
                    break;
                case CLK_FILTER_NEAREST:
                    SIMPLE_ASSIGN(cl_filter_mode, CL_FILTER_NEAREST);
                    break;
            }

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}
