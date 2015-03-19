/******************************************************************************
 * Copyright (c) 2011-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "clc.h"

#define SCALAR(type, the_max) \
{ \
    type the_floor = floor(x); \
    *ptr = the_floor; \
    if (isnan(x)) return x; \
    if (isinf(x)) return (type) 0.0; \
    return fmin(x - the_floor, (type) (the_max)); \
} \

#define BODY(type, the_max) \
{ \
    type the_floor = floor(x); \
    *ptr = the_floor; \
    type result = select(x - the_floor, (type) 0.0, isinf(x)); \
         result = fmin(result, (type) (the_max)); \
    return select(result, x, isnan(x)); \
} \

_CLC_OVERLOAD _CLC_DEF float fract(float x, global  float * ptr) SCALAR(float, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float fract(float x, local   float * ptr) SCALAR(float, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float fract(float x, private float * ptr) SCALAR(float, 0x1.fffffep-1f)

_CLC_OVERLOAD _CLC_DEF float2 fract(float2 x, global  float2 * ptr) BODY(float2, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float2 fract(float2 x, local   float2 * ptr) BODY(float2, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float2 fract(float2 x, private float2 * ptr) BODY(float2, 0x1.fffffep-1f)

_CLC_OVERLOAD _CLC_DEF float3 fract(float3 x, global  float3 * ptr) BODY(float3, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float3 fract(float3 x, local   float3 * ptr) BODY(float3, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float3 fract(float3 x, private float3 * ptr) BODY(float3, 0x1.fffffep-1f)

_CLC_OVERLOAD _CLC_DEF float4 fract(float4 x, global  float4 * ptr) BODY(float4, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float4 fract(float4 x, local   float4 * ptr) BODY(float4, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float4 fract(float4 x, private float4 * ptr) BODY(float4, 0x1.fffffep-1f)

_CLC_OVERLOAD _CLC_DEF float8 fract(float8 x, global  float8 * ptr) BODY(float8, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float8 fract(float8 x, local   float8 * ptr) BODY(float8, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float8 fract(float8 x, private float8 * ptr) BODY(float8, 0x1.fffffep-1f)

_CLC_OVERLOAD _CLC_DEF float16 fract(float16 x, global  float16 * ptr) BODY(float16, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float16 fract(float16 x, local   float16 * ptr) BODY(float16, 0x1.fffffep-1f)
_CLC_OVERLOAD _CLC_DEF float16 fract(float16 x, private float16 * ptr) BODY(float16, 0x1.fffffep-1f)

_CLC_OVERLOAD _CLC_DEF double fract(double x, global  double * ptr) SCALAR(double, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double fract(double x, local   double * ptr) SCALAR(double, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double fract(double x, private double * ptr) SCALAR(double, 0x1.fffffffffffffp-1)

_CLC_OVERLOAD _CLC_DEF double2 fract(double2 x, global  double2 * ptr) BODY(double2, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double2 fract(double2 x, local   double2 * ptr) BODY(double2, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double2 fract(double2 x, private double2 * ptr) BODY(double2, 0x1.fffffffffffffp-1)

_CLC_OVERLOAD _CLC_DEF double3 fract(double3 x, global  double3 * ptr) BODY(double3, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double3 fract(double3 x, local   double3 * ptr) BODY(double3, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double3 fract(double3 x, private double3 * ptr) BODY(double3, 0x1.fffffffffffffp-1)

_CLC_OVERLOAD _CLC_DEF double4 fract(double4 x, global  double4 * ptr) BODY(double4, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double4 fract(double4 x, local   double4 * ptr) BODY(double4, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double4 fract(double4 x, private double4 * ptr) BODY(double4, 0x1.fffffffffffffp-1)

_CLC_OVERLOAD _CLC_DEF double8 fract(double8 x, global  double8 * ptr) BODY(double8, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double8 fract(double8 x, local   double8 * ptr) BODY(double8, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double8 fract(double8 x, private double8 * ptr) BODY(double8, 0x1.fffffffffffffp-1)

_CLC_OVERLOAD _CLC_DEF double16 fract(double16 x, global  double16 * ptr) BODY(double16, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double16 fract(double16 x, local   double16 * ptr) BODY(double16, 0x1.fffffffffffffp-1)
_CLC_OVERLOAD _CLC_DEF double16 fract(double16 x, private double16 * ptr) BODY(double16, 0x1.fffffffffffffp-1)

