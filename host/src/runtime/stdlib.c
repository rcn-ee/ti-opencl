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

#include "stdlib.h"

int debug(const char *format, ...);

/* WARNING: Due to some device-specific things in stdlib.h, the bitcode stdlib
 * must only be used by CPUDevice, as it's targeted to the host CPU at Clover's
 * compilation! */

/*
 * Image functions
 */

int __cpu_get_image_width(void *image);
int __cpu_get_image_height(void *image);
int __cpu_get_image_depth(void *image);
int __cpu_get_image_channel_data_type(void *image);
int __cpu_get_image_channel_order(void *image);
int __cpu_is_image_3d(void *image);
void *__cpu_image_data(void *image, int x, int y, int z, int *order, int *type);

void __cpu_write_imagef(void *image, int x, int y, int z, float4 *color);
void __cpu_write_imagei(void *image, int x, int y, int z, int4 *color);
void __cpu_write_imageui(void *image, int x, int y, int z, uint4 *color);

void __cpu_read_imagefi(float4 *result, void *image, int x, int y, int z,
                        sampler_t sampler);
void __cpu_read_imageii(int4 *result, void *image, int x, int y, int z,
                        sampler_t sampler);
void __cpu_read_imageuii(uint4 *result, void *image, int x, int y, int z,
                        sampler_t sampler);
void __cpu_read_imageff(float4 *result, void *image, float x, float y, float z,
                        sampler_t sampler);
void __cpu_read_imageif(int4 *result, void *image, float x, float y, float z,
                        sampler_t sampler);
void __cpu_read_imageuif(uint4 *result, void *image, float x, float y, float z,
                        sampler_t sampler);

float4 OVERLOAD read_imagef(image2d_t image, sampler_t sampler, int2 coord)
{
    float4 rs;

    __cpu_read_imagefi(&rs, image, coord.x, coord.y, 0, sampler);

    return rs;
}

float4 OVERLOAD read_imagef(image3d_t image, sampler_t sampler, int4 coord)
{
    float4 rs;

    __cpu_read_imagefi(&rs, image, coord.x, coord.y, coord.z, sampler);

    return rs;
}

float4 OVERLOAD read_imagef(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 rs;

    __cpu_read_imageff(&rs, image, coord.x, coord.y, 0.0f, sampler);

    return rs;
}

float4 OVERLOAD read_imagef(image3d_t image, sampler_t sampler, float4 coord)
{
    float4 rs;

    __cpu_read_imageff(&rs, image, coord.x, coord.y, coord.z, sampler);

    return rs;
}

int4 OVERLOAD read_imagei(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 rs;

    __cpu_read_imageii(&rs, image, coord.x, coord.y, 0, sampler);

    return rs;
}

int4 OVERLOAD read_imagei(image3d_t image, sampler_t sampler, int4 coord)
{
    int4 rs;

    __cpu_read_imageii(&rs, image, coord.x, coord.y, coord.z, sampler);

    return rs;
}

int4 OVERLOAD read_imagei(image2d_t image, sampler_t sampler, float2 coord)
{
    int4 rs;

    __cpu_read_imageif(&rs, image, coord.x, coord.y, 0.0f, sampler);

    return rs;
}

int4 OVERLOAD read_imagei(image3d_t image, sampler_t sampler, float4 coord)
{
    int4 rs;

    __cpu_read_imageif(&rs, image, coord.x, coord.y, coord.z, sampler);

    return rs;
}

uint4 OVERLOAD read_imageui(image2d_t image, sampler_t sampler, int2 coord)
{
    uint4 rs;

    __cpu_read_imageuii(&rs, image, coord.x, coord.y, 0, sampler);

    return rs;
}

uint4 OVERLOAD read_imageui(image3d_t image, sampler_t sampler, int4 coord)
{
    uint4 rs;

    __cpu_read_imageuii(&rs, image, coord.x, coord.y, coord.z, sampler);

    return rs;
}

uint4 OVERLOAD read_imageui(image2d_t image, sampler_t sampler, float2 coord)
{
    uint4 rs;

    __cpu_read_imageuif(&rs, image, coord.x, coord.y, 0.0f, sampler);

    return rs;
}

uint4 OVERLOAD read_imageui(image3d_t image, sampler_t sampler, float4 coord)
{
    uint4 rs;

    __cpu_read_imageuif(&rs, image, coord.x, coord.y, coord.z, sampler);

    return rs;
}

void OVERLOAD write_imagef(image2d_t image, int2 coord, float4 color)
{
    __cpu_write_imagef(image, coord.x, coord.y, 0, &color);
}

void OVERLOAD write_imagef(image3d_t image, int4 coord, float4 color)
{
    __cpu_write_imagef(image, coord.x, coord.y, coord.z, &color);
}

void OVERLOAD write_imagei(image2d_t image, int2 coord, int4 color)
{
    __cpu_write_imagei(image, coord.x, coord.y, 0, &color);
}

void OVERLOAD write_imagei(image3d_t image, int4 coord, int4 color)
{
    __cpu_write_imagei(image, coord.x, coord.y, coord.z, &color);
}

void OVERLOAD write_imageui(image2d_t image, int2 coord, uint4 color)
{
   __cpu_write_imageui(image, coord.x, coord.y, 0, &color);
}

void OVERLOAD write_imageui(image3d_t image, int4 coord, uint4 color)
{
    __cpu_write_imageui(image, coord.x, coord.y, coord.z, &color);
}

int2 OVERLOAD get_image_dim(image2d_t image)
{
    int2 result;

    result.x = get_image_width(image);
    result.y = get_image_height(image);

    return result;
}

int4 OVERLOAD get_image_dim(image3d_t image)
{
    int4 result;

    result.x = get_image_width(image);
    result.y = get_image_height(image);
    result.z = get_image_depth(image);
    result.w = 0;

    return result;
}

int OVERLOAD get_image_width(image2d_t image)
{
    return __cpu_get_image_width(image);
}

int OVERLOAD get_image_width(image3d_t image)
{
    return __cpu_get_image_width(image);
}

int OVERLOAD get_image_height(image2d_t image)
{
    return __cpu_get_image_height(image);
}

int OVERLOAD get_image_height(image3d_t image)
{
    return __cpu_get_image_height(image);
}

int OVERLOAD get_image_depth(image3d_t image)
{
    return __cpu_get_image_depth(image);
}

int OVERLOAD get_image_channel_data_type(image2d_t image)
{
    return __cpu_get_image_channel_data_type(image);
}

int OVERLOAD get_image_channel_data_type(image3d_t image)
{
    return __cpu_get_image_channel_data_type(image);
}

int OVERLOAD get_image_channel_order(image2d_t image)
{
    return __cpu_get_image_channel_order(image);
}

int OVERLOAD get_image_channel_order(image3d_t image)
{
    return __cpu_get_image_channel_order(image);
}

/*
 * Built-in functions generated by src/runtime/builtins.py
 */

#include <stdlib_impl.h>
