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
 * \file cpu/sampler.cpp
 * \brief OpenCL C image access functions
 *
 * It is recommended to compile this file using Clang as it supports the
 * \c __builtin_shufflevector() built-in function, providing SSE or
 * NEON-accelerated code.
 */

#include "../memobject.h"
#include "../sampler.h"
#include "kernel.h"
#include "buffer.h"
#include "builtins.h"

#include <cstdlib>
#include <cmath>
// ASW #include <immintrin.h>

using namespace Coal;

/*
 * Helper functions
 */

static int clamp(int a, int b, int c)
{
    return (a < b) ? b : ((a > c) ? c : a);
}

static int min(int a, int b)
{
    return (a < b ? a : b);
}

static int max(int a, int b)
{
    return (a > b ? a : b);
}

static float frac(float x)
{
    return x - std::floor(x);
}

static float round(float x)
{
    return (float)(int)x;
}

static bool handle_address_mode(Image2D *image, int &x, int &y, int &z,
                                uint32_t sampler)
{
    bool is_3d = (image->type() == MemObject::Image3D);
    int w = image->width(),
        h = image->height(),
        d = (is_3d ? ((Image3D *)image)->depth() : 1);

    if ((sampler & 0xf0) ==  CLK_ADDRESS_CLAMP_TO_EDGE)
    {
        x = clamp(x, 0, w - 1);
        y = clamp(y, 0, h - 1);
        if (is_3d) z = clamp(z, 0, d - 1);
    }
    else if ((sampler & 0xf0) == CLK_ADDRESS_CLAMP)
    {
        x = clamp(x, 0, w);
        y = clamp(y, 0, h);
        if (is_3d) z = clamp(z, 0, d);
    }

    return (x == w || y == h || z == d);
}

/*
 * Macros or functions used to accelerate the functions
 */
#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

static void slow_shuffle4(uint32_t *rs, uint32_t *a, uint32_t *b,
                          int x, int y, int z, int w)
{
    rs[0] = (x < 4 ? a[x] : b[x - 4]);
    rs[1] = (y < 4 ? a[y] : b[y - 4]);
    rs[2] = (z < 4 ? a[z] : b[z - 4]);
    rs[3] = (w < 4 ? a[w] : b[w - 4]);
}

static void convert_to_format(void *dest, float *data,
                                   cl_channel_type type, unsigned int channels)
{
    // Convert always the four components of source to target
    if (type == CL_FLOAT)
        std::memcpy(dest, data, channels * sizeof(float));

    for (unsigned int i=0; i<channels; ++i)
    {
        switch (type)
        {
            case CL_SNORM_INT8:
                ((int8_t *)dest)[i] = data[i] * 128.0f;
                break;
            case CL_SNORM_INT16:
                ((int16_t *)dest)[i] = data[i] * 32767.0f;
                break;
            case CL_UNORM_INT8:
                ((uint8_t *)dest)[i] = data[i] * 255.0f;
                break;
            case CL_UNORM_INT16:
                ((uint16_t *)dest)[i] = data[i] * 65535.0f;
                break;
        }
    }
}

static void convert_from_format(float *data, void *source,
                                     cl_channel_type type, unsigned int channels)
{
    // Convert always the four components of source to target
    if (type == CL_FLOAT)
        std::memcpy(data, source, channels * sizeof(float));

    for (unsigned int i=0; i<channels; ++i)
    {
        switch (type)
        {
            case CL_SNORM_INT8:
                data[i] = (float)((int8_t *)source)[i] / 127.0f;
                break;
            case CL_SNORM_INT16:
                data[i] = (float)((int16_t *)source)[i] / 32767.0f;
                break;
            case CL_UNORM_INT8:
                data[i] = (float)((uint8_t *)source)[i] / 127.0f;
                break;
            case CL_UNORM_INT16:
                data[i] = (float)((uint16_t *)source)[i] / 127.0f;
                break;
        }
    }
}

static void convert_to_format(void *dest, int *data,
                                   cl_channel_type type, unsigned int channels)
{
    // Convert always the four components of source to target
    if (type == CL_SIGNED_INT32)
        std::memcpy(dest, data, channels * sizeof(int32_t));

    for (unsigned int i=0; i<channels; ++i)
    {
        switch (type)
        {
            case CL_SIGNED_INT8:
                ((int8_t *)dest)[i] = data[i];
                break;
            case CL_SIGNED_INT16:
                ((int16_t *)dest)[i] = data[i];
                break;
        }
    }
}

static void convert_from_format(int32_t *data, void *source,
                                     cl_channel_type type, unsigned int channels)
{
    // Convert always the four components of source to target
    if (type == CL_SIGNED_INT32)
        std::memcpy(data, source, channels * sizeof(int32_t));

    for (unsigned int i=0; i<channels; ++i)
    {
        switch (type)
        {
            case CL_SIGNED_INT8:
                data[i] = ((int8_t *)source)[i];
                break;
            case CL_SIGNED_INT16:
                data[i] = ((int16_t *)source)[i];
                break;
        }
    }
}

static void convert_to_format(void *dest, uint32_t *data,
                                   cl_channel_type type, unsigned int channels)
{
    // Convert always the four components of source to target
    if (type == CL_UNSIGNED_INT32)
        std::memcpy(dest, data, channels * sizeof(uint32_t));

    for (unsigned int i=0; i<3; ++i)
    {
        switch (type)
        {
            case CL_UNSIGNED_INT8:
                ((uint8_t *)dest)[i] = data[i];
                break;
            case CL_UNSIGNED_INT16:
                ((uint16_t *)dest)[i] = data[i];
                break;
        }
    }
}

static void convert_from_format(uint32_t *data, void *source,
                                     cl_channel_type type, unsigned int channels)
{
    // Convert always the four components of source to target
    if (type == CL_UNSIGNED_INT32)
        std::memcpy(data, source, channels * sizeof(uint32_t));

    for (unsigned int i=0; i<channels; ++i)
    {
        switch (type)
        {
            case CL_UNSIGNED_INT8:
                data[i] = ((uint8_t *)source)[i];
                break;
            case CL_UNSIGNED_INT16:
                data[i] = ((uint16_t *)source)[i];
                break;
        }
    }
}

template<typename T>
static void vec4_scalar_mul(T *vec, float val)
{
    for (unsigned int i=0; i<4; ++i)
        vec[i] *= val;
}

template<typename T>
static void vec4_add(T *vec1, T *vec2)
{
    for (unsigned int i=0; i<4; ++i)
        vec1[i] += vec2[i];
}

template<typename T>
void CPUKernelWorkGroup::linear3D(T *result, float a, float b, float c,
              int i0, int j0, int k0, int i1, int j1, int k1,
              Image3D *image) const
{
    T accum[4];

    readImageImplI<T>(result, image, i0, j0, k0, 0);
    vec4_scalar_mul(result, (1.0f - a) * (1.0f - b) * (1.0f - c ));

    readImageImplI<T>(accum, image, i1, j0, k0, 0);
    vec4_scalar_mul(accum, a * (1.0f - b) * (1.0f - c ));
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i0, j1, k0, 0);
    vec4_scalar_mul(accum, (1.0f - a) * b * (1.0f - c ));
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i1, j1, k0, 0);
    vec4_scalar_mul(accum, a * b * (1.0f -c ));
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i0, j0, k1, 0);
    vec4_scalar_mul(accum, (1.0f - a) * (1.0f - b) * c);
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i1, j0, k1, 0);
    vec4_scalar_mul(accum, a * (1.0f - b) * c);
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i0, j1, k1, 0);
    vec4_scalar_mul(accum, (1.0f - a) * b * c);
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i1, j1, k1, 0);
    vec4_scalar_mul(accum, a * b * c);
    vec4_add(result, accum);
}

template<typename T>
void CPUKernelWorkGroup::linear2D(T *result, float a, float b, float c, int i0, int j0,
              int i1, int j1, Image2D *image) const
{
    T accum[4];

    readImageImplI<T>(result, image, i0, j0, 0, 0);
    vec4_scalar_mul(result, (1.0f - a) * (1.0f - b));

    readImageImplI<T>(accum, image, i1, j0, 0, 0);
    vec4_scalar_mul(accum, a * (1.0f - b));
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i0, j1, 0, 0);
    vec4_scalar_mul(accum, (1.0f - a) * b);
    vec4_add(result, accum);

    readImageImplI<T>(accum, image, i1, j1, 0, 0);
    vec4_scalar_mul(accum, a * b);
    vec4_add(result, accum);
}

#if __has_builtin(__builtin_shufflevector)
    #define shuffle4(rs, a, b, x, y, z, w) \
        *(__v4sf *)rs = __builtin_shufflevector(*(__v4sf *)a, *(__v4sf *)b, \
                                                x, y, z, w)
#else
    #define shuffle4(rs, a, b, x, y, z, w) \
        slow_shuffle4(rs, a, b, x, y, z, w)
#endif

static void swizzle(uint32_t *target, uint32_t *source,
                    cl_channel_order order, bool reading, uint32_t t_max)
{
    uint32_t special[4] = {0, t_max, 0, 0 };

    if (reading)
    {
        switch (order)
        {
            case CL_R:
            case CL_Rx:
                // target = {source->x, 0, 0, t_max}
                shuffle4(target, source, special, 0, 4, 4, 5);
                break;
            case CL_A:
                // target = {0, 0, 0, source->x}
                shuffle4(target, source, special, 4, 4, 4, 0);
                break;
            case CL_INTENSITY:
                // target = {source->x, source->x, source->x, source->x}
                shuffle4(target, source, source, 0, 0, 0, 0);
                break;
            case CL_LUMINANCE:
                // target = {source->x, source->x, source->x, t_max}
                shuffle4(target, source, special, 0, 0, 0, 5);
                break;
            case CL_RG:
            case CL_RGx:
                // target = {source->x, source->y, 0, t_max}
                shuffle4(target, source, special, 0, 1, 4, 5);
                break;
            case CL_RA:
                // target = {source->x, 0, 0, source->y}
                shuffle4(target, source, special, 0, 4, 4, 1);
                break;
            case CL_RGB:
            case CL_RGBx:
            case CL_RGBA:
                // Nothing to do, already the good order
                std::memcpy(target, source, 16);
                break;
            case CL_ARGB:
                // target = {source->y, source->z, source->w, source->x}
                shuffle4(target, source, source, 1, 2, 3, 0);
                break;
            case CL_BGRA:
                // target = {source->z, source->y, source->x, source->w}
                shuffle4(target, source, source, 2, 1, 0, 3);
                break;
        }
    }
    else
    {
        switch (order)
        {
            case CL_A:
                // target = {source->w, undef, undef, undef}
                shuffle4(target, source, source, 3, 3, 3, 3);
                break;
            case CL_RA:
                // target = {source->x, source->w, undef, undef}
                shuffle4(target, source, source, 0, 3, 3, 3);
                break;
            case CL_ARGB:
                // target = {source->w, source->x, source->y, source->z}
                shuffle4(target, source, source, 3, 0, 1, 2);
                break;
            case CL_BGRA:
                // target = {source->z, source->y, source->x, source->w}
                shuffle4(target, source, source, 2, 1, 0, 3);
                break;
            default:
                std::memcpy(target, source, 16);
        }
    }
}

/*
 * Actual implementation of the built-ins
 */

void *CPUKernelWorkGroup::getImageData(Image2D *image, int x, int y, int z) const
{
    CPUBuffer *buffer =
        (CPUBuffer *)image->deviceBuffer((DeviceInterface *)p_kernel->device());

    return imageData((unsigned char *)buffer->data(),
                     x, y, z,
                     image->row_pitch(),
                     image->slice_pitch(),
                     image->pixel_size());
}

template<typename T>
void CPUKernelWorkGroup::writeImageImpl(Image2D *image, int x, int y, int z,
                                        T *color) const
{
    T converted[4];

    // Swizzle to the correct order (float, int and uint are 32-bit, so the
    // type has no importance
    swizzle((uint32_t *)converted, (uint32_t *)color,
            image->format().image_channel_order, false, 0);

    // Get a pointer in the image where to write the data
    void *target = getImageData(image, x, y, z);

    // Convert color to the correct format
    convert_to_format(target,
                      converted,
                      image->format().image_channel_data_type,
                      image->channels());
}

void CPUKernelWorkGroup::writeImage(Image2D *image, int x, int y, int z,
                                    float *color) const
{
    writeImageImpl<float>(image, x, y, z, color);
}

void CPUKernelWorkGroup::writeImage(Image2D *image, int x, int y, int z,
                                    int32_t *color) const
{
    writeImageImpl<int32_t>(image, x, y, z, color);
}

void CPUKernelWorkGroup::writeImage(Image2D *image, int x, int y, int z,
                                    uint32_t *color) const
{
    writeImageImpl<uint32_t>(image, x, y, z, color);
}

template<typename T>
uint32_t type_max_value()
{
    return 0;
}

template<>
uint32_t type_max_value<float>()
{
    return 1065353216; // 1.0f in decimal form
}

template<>
uint32_t type_max_value<int32_t>()
{
    return 0x7fffffff;
}

template<>
uint32_t type_max_value<uint32_t>()
{
    return 0xffffffff;
}

template<typename T>
void CPUKernelWorkGroup::readImageImplI(T *result, Image2D *image, int x, int y,
                                        int z, uint32_t sampler) const
{
    // Handle the addressing mode of the sampler
    if (handle_address_mode(image, x, y, z, sampler))
    {
        // Border color
        result[0] = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;

        switch (image->format().image_channel_order)
        {
            case CL_R:
            case CL_RG:
            case CL_RGB:
            case CL_LUMINANCE:
                result[3] = 1.0f;
                break;
            default:
                result[3] = 0.0f;
        }

        return;
    }

    // Load the data from the image, converting it
    void *source = getImageData(image, x, y, z);
    T converted[4];

    convert_from_format(converted,
                        source,
                        image->format().image_channel_data_type,
                        image->channels());

    // Swizzle the pixel just read and place it in result
    swizzle((uint32_t *)result, (uint32_t *)converted,
            image->format().image_channel_order, true, type_max_value<T>());
}

void CPUKernelWorkGroup::readImage(float *result, Image2D *image, int x, int y,
                                   int z, uint32_t sampler) const
{
    readImageImplI<float>(result, image, x, y, z, sampler);
}

void CPUKernelWorkGroup::readImage(int32_t *result, Image2D *image, int x, int y,
                                   int z, uint32_t sampler) const
{
    readImageImplI<int32_t>(result, image, x, y, z, sampler);
}

void CPUKernelWorkGroup::readImage(uint32_t *result, Image2D *image, int x, int y,
                                   int z, uint32_t sampler) const
{
    readImageImplI<uint32_t>(result, image, x, y, z, sampler);
}

template<typename T>
void CPUKernelWorkGroup::readImageImplF(T *result, Image2D *image, float x,
                                        float y, float z, uint32_t sampler) const
{
    bool is_3d = (image->type() == MemObject::Image3D);
    Image3D *image3d = (Image3D *)image;

    int w = image->width(),
        h = image->height(),
        d = (is_3d ? image3d->depth() : 1);

    switch (sampler & 0xf0)
    {
        case CLK_ADDRESS_NONE:
        case CLK_ADDRESS_CLAMP:
        case CLK_ADDRESS_CLAMP_TO_EDGE:
            /* De-normalize coordinates */
            if ((sampler & 0xf) == CLK_NORMALIZED_COORDS_TRUE)
            {
                x *= (float)w;
                y *= (float)h;
                if (is_3d) z *= (float)d;
            }

            switch (sampler & 0xf00)
            {
                case CLK_FILTER_NEAREST:
                {
                    readImageImplI<T>(result, image, std::floor(x),
                                      std::floor(y), std::floor(z), sampler);
                }
                case CLK_FILTER_LINEAR:
                {
                    float a, b, c;

                    a = frac(x - 0.5f);
                    b = frac(y - 0.5f);
                    c = frac(z - 0.5f);

                    if (is_3d)
                    {
                        linear3D<T>(result, a, b, c,
                                    std::floor(x - 0.5f),
                                    std::floor(y - 0.5f),
                                    std::floor(z - 0.5f),
                                    std::floor(x - 0.5f) + 1,
                                    std::floor(y - 0.5f) + 1,
                                    std::floor(z - 0.5f) + 1,
                                    image3d);
                    }
                    else
                    {
                        linear2D<T>(result, a, b, c,
                                    std::floor(x - 0.5f),
                                    std::floor(y - 0.5f),
                                    std::floor(x - 0.5f) + 1,
                                    std::floor(y - 0.5f) + 1,
                                    image);
                    }
                }
            }
            break;
        case CLK_ADDRESS_REPEAT:
            switch (sampler & 0xf00)
            {
                case CLK_FILTER_NEAREST:
                {
                    int i, j, k = 0;

                    x = (x - std::floor(x)) * (float)w;
                    i = std::floor(x);
                    if (i > w - 1)
                        i = i - w;

                    y = (y - std::floor(y)) * (float)h;
                    j = std::floor(y);
                    if (j > h - 1)
                        j = j - h;

                    if (is_3d)
                    {
                        z = (z - std::floor(z)) * (float)d;
                        k = std::floor(z);
                        if (k > d - 1)
                            k = k - d;
                    }

                    readImageImplI<T>(result, image, i, j, k, sampler);
                }
                case CLK_FILTER_LINEAR:
                {
                    float a, b, c;
                    int i0, i1, j0, j1, k0, k1;

                    x = (x - std::floor(x)) * (float)w;
                    i0 = std::floor(x - 0.5f);
                    i1 = i0 + 1;
                    if (i0 < 0)
                        i0 = w + i0;
                    if (i1 > w - 1)
                        i1 = i1 - w;

                    y = (y - std::floor(y)) * (float)h;
                    j0 = std::floor(y - 0.5f);
                    j1 = j0 + 1;
                    if (j0 < 0)
                        j0 = h + j0;
                    if (j1 > h - 1)
                        j1 = j1 - h;

                    if (is_3d)
                    {
                        z = (z - std::floor(z)) * (float)d;
                        k0 = std::floor(z - 0.5f);
                        k1 = k0 + 1;
                        if (k0 < 0)
                            k0 = d + k0;
                        if (k1 > d - 1)
                            k1 = k1 - d;
                    }

                    a = frac(x - 0.5f);
                    b = frac(y - 0.5f);
                    c = frac(z - 0.5f);

                    if (is_3d)
                    {
                        linear3D<T>(result, a, b, c, i0, j0, k0, i1, j1, k1,
                                    image3d);
                    }
                    else
                    {
                        linear2D<T>(result, a, b, c, i0, j0, i1, j1, image);
                    }
                }
            }
            break;
        case CLK_ADDRESS_MIRRORED_REPEAT:
            switch (sampler & 0xf00)
            {
                case CLK_FILTER_NEAREST:
                {
                    x = std::fabs(x - 2.0f * round(0.5f * x)) * (float)w;
                    y = std::fabs(y - 2.0f * round(0.5f * y)) * (float)h;
                    if (is_3d)
                        z = std::fabs(z - 2.0f * round(0.5f * z)) * (float)d;

                    readImageImplI<T>(result, image,
                                      min(std::floor(x), w - 1),
                                      min(std::floor(y), h - 1),
                                      min(std::floor(z), d - 1),
                                      sampler);
                }
                case CLK_FILTER_LINEAR:
                {
                    float a, b, c;
                    int i0, i1, j0, j1, k0, k1;

                    x = std::fabs(x - 2.0f * round(0.5f * x)) * (float)w;
                    i0 = std::floor(x - 0.5f);
                    i1 = i0 + 1;
                    i0 = max(i0, 0);
                    i1 = min(i1, w - 1);

                    y = std::fabs(y - 2.0f * round(0.5f * y)) * (float)h;
                    j0 = std::floor(y - 0.5f);
                    j1 = j0 + 1;
                    j0 = max(j0, 0);
                    j1 = min(j1, h - 1);

                    if (is_3d)
                    {
                        z = std::fabs(z - 2.0f * round(0.5f * z)) * (float)d;
                        k0 = std::floor(z - 0.5f);
                        k1 = k0 + 1;
                        k0 = max(k0, 0);
                        k1 = min(k1, d - 1);
                    }

                    a = frac(x - 0.5f);
                    b = frac(y - 0.5f);
                    c = frac(z - 0.5f);

                    if (is_3d)
                    {
                        linear3D<T>(result, a, b, c, i0, j0, k0, i1, j1, k1,
                                    image3d);
                    }
                    else
                    {
                        linear2D<T>(result, a, b, c, i0, j0, i1, j1, image);
                    }
                }
            }
            break;
    }
}

void CPUKernelWorkGroup::readImage(float *result, Image2D *image, float x,
                                   float y, float z, uint32_t sampler) const
{
    readImageImplF<float>(result, image, x, y, z, sampler);
}

void CPUKernelWorkGroup::readImage(int32_t *result, Image2D *image, float x,
                                   float y, float z, uint32_t sampler) const
{
    readImageImplF<int32_t>(result, image, x, y, z, sampler);
}

void CPUKernelWorkGroup::readImage(uint32_t *result, Image2D *image, float x,
                                   float y, float z, uint32_t sampler) const
{
    readImageImplF<uint32_t>(result, image, x, y, z, sampler);
}
