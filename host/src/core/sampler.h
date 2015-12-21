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
 * \file sampler.h
 * \brief Sampler object
 */

#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include <CL/cl.h>
#include "object.h"
#include "icd.h"

// WARNING: Keep in sync with stdlib.h

#define CLK_NORMALIZED_COORDS_FALSE 0x00000000
#define CLK_NORMALIZED_COORDS_TRUE  0x00000001
#define CLK_ADDRESS_NONE            0x00000000
#define CLK_ADDRESS_MIRRORED_REPEAT 0x00000010
#define CLK_ADDRESS_REPEAT          0x00000020
#define CLK_ADDRESS_CLAMP_TO_EDGE   0x00000030
#define CLK_ADDRESS_CLAMP           0x00000040
#define CLK_FILTER_NEAREST          0x00000000
#define CLK_FILTER_LINEAR           0x00000100

#define CLK_NORMALIZED_COORDS_MASK  0x0000000f
#define CLK_ADDRESS_MODE_MASK       0x000000f0
#define CLK_FILTER_MASK             0x00000f00

namespace Coal
{
  class Sampler;
}
struct _cl_sampler: public Coal::descriptor<Coal::Sampler, _cl_sampler> {};

namespace Coal
{

class Context;

/**
 * \brief Sampler
 * 
 * This object doesn't do anything intersting, it only converts a set of
 * host OpenCL constants to constants that will be used by the kernels and
 * the image reading and writing built-in functions.
 */
class Sampler : public _cl_sampler, public Object
{
    public:
        /**
         * \brief Constructor
         * \param ctx parent \c Coal::Context
         * \param normalized_coords true if the coords given to the built-in
         *        image functions are normalized, false otherwise
         * \param addressing_mode addressing mode used to read images
         * \param filter_mode filter mode used to read images
         * \param errcode_ret return code (\c CL_SUCCESS if all is good)
         */
        Sampler(Context *ctx,
                cl_bool normalized_coords,
                cl_addressing_mode addressing_mode,
                cl_filter_mode filter_mode,
                cl_int *errcode_ret);

        /**
         * \brief Simpler constructor
         * \param ctx parent \c Coal::Context
         * \param bitfield bitfield already calculated
         */
        Sampler(Context *ctx,
                unsigned int bitfield);

        unsigned int bitfield() const; /*!< \brief Bitfield value usable by the kernels */

        /**
         * \brief Get information about the sampler
         * \copydetails Coal::DeviceInterface::info
         */
        cl_int info(cl_sampler_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret) const;

    private:
        unsigned int p_bitfield;

        cl_int checkImageAvailability() const;
};

}

#endif
