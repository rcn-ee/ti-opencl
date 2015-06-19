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

_CLC_PROTECTED void sincosf(float x, float * sinval, float * cosval);
_CLC_PROTECTED void sincosd(double x, double * sinval, double * cosval);

#define SINCOS_SCALAR_BODY(type, op) \
{ \
    type sin_val; \
    type cos_val; \
    op(x, &sin_val, &cos_val); \
    *cosval = cos_val; \
    return sin_val; \
} \

#define SINCOS_VECTOR_BODY_2(prim_type, op) \
    op(x.s0, &(((prim_type*)&sin_val)[0]), &(((prim_type*)&cos_val)[0])); \
    op(x.s1, &(((prim_type*)&sin_val)[1]), &(((prim_type*)&cos_val)[1])); \

#define SINCOS_VECTOR_BODY_3(prim_type, op) \
    SINCOS_VECTOR_BODY_2(prim_type, op) \
    op(x.s2, &(((prim_type*)&sin_val)[2]), &(((prim_type*)&cos_val)[2])); \

#define SINCOS_VECTOR_BODY_4(prim_type, op) \
    SINCOS_VECTOR_BODY_3(prim_type, op) \
    op(x.s3, &(((prim_type*)&sin_val)[3]), &(((prim_type*)&cos_val)[3])); \

#define SINCOS_VECTOR_BODY_8(prim_type, op) \
    SINCOS_VECTOR_BODY_4(prim_type, op) \
    op(x.s4, &(((prim_type*)&sin_val)[4]), &(((prim_type*)&cos_val)[4])); \
    op(x.s5, &(((prim_type*)&sin_val)[5]), &(((prim_type*)&cos_val)[5])); \
    op(x.s6, &(((prim_type*)&sin_val)[6]), &(((prim_type*)&cos_val)[6])); \
    op(x.s7, &(((prim_type*)&sin_val)[7]), &(((prim_type*)&cos_val)[7])); \

#define SINCOS_VECTOR_BODY_16(prim_type, op) \
    SINCOS_VECTOR_BODY_8(prim_type, op) \
    op(x.s8, &(((prim_type*)&sin_val)[8]), &(((prim_type*)&cos_val)[8])); \
    op(x.s9, &(((prim_type*)&sin_val)[9]), &(((prim_type*)&cos_val)[9])); \
    op(x.sa, &(((prim_type*)&sin_val)[10]), &(((prim_type*)&cos_val)[10])); \
    op(x.sb, &(((prim_type*)&sin_val)[11]), &(((prim_type*)&cos_val)[11])); \
    op(x.sc, &(((prim_type*)&sin_val)[12]), &(((prim_type*)&cos_val)[12])); \
    op(x.sd, &(((prim_type*)&sin_val)[13]), &(((prim_type*)&cos_val)[13])); \
    op(x.se, &(((prim_type*)&sin_val)[14]), &(((prim_type*)&cos_val)[14])); \
    op(x.sf, &(((prim_type*)&sin_val)[15]), &(((prim_type*)&cos_val)[15])); \

#define SINCOS_VECTOR_BODY(prim_type, num, op) \
{ \
    prim_type##num   sin_val; \
    prim_type##num   cos_val; \
    SINCOS_VECTOR_BODY_##num(prim_type, op)\
    *cosval =  cos_val; \
    return sin_val; \
} \

_CLC_OVERLOAD _CLC_INLINE float sincos(float x, global  float * cosval) SINCOS_SCALAR_BODY(float, sincosf)
_CLC_OVERLOAD _CLC_INLINE float sincos(float x, local   float * cosval) SINCOS_SCALAR_BODY(float, sincosf)
_CLC_OVERLOAD _CLC_INLINE float sincos(float x, private float * cosval) SINCOS_SCALAR_BODY(float, sincosf)

_CLC_OVERLOAD _CLC_DEF float2 sincos(float2 x, global  float2 * cosval) SINCOS_VECTOR_BODY(float, 2, sincosf)
_CLC_OVERLOAD _CLC_DEF float2 sincos(float2 x, local   float2 * cosval) SINCOS_VECTOR_BODY(float, 2, sincosf)
_CLC_OVERLOAD _CLC_DEF float2 sincos(float2 x, private float2 * cosval) SINCOS_VECTOR_BODY(float, 2, sincosf)

_CLC_OVERLOAD _CLC_DEF float3 sincos(float3 x, global  float3 * cosval) SINCOS_VECTOR_BODY(float, 3, sincosf)
_CLC_OVERLOAD _CLC_DEF float3 sincos(float3 x, local   float3 * cosval) SINCOS_VECTOR_BODY(float, 3, sincosf)
_CLC_OVERLOAD _CLC_DEF float3 sincos(float3 x, private float3 * cosval) SINCOS_VECTOR_BODY(float, 3, sincosf)

_CLC_OVERLOAD _CLC_DEF float4 sincos(float4 x, global  float4 * cosval) SINCOS_VECTOR_BODY(float, 4, sincosf)
_CLC_OVERLOAD _CLC_DEF float4 sincos(float4 x, local   float4 * cosval) SINCOS_VECTOR_BODY(float, 4, sincosf)
_CLC_OVERLOAD _CLC_DEF float4 sincos(float4 x, private float4 * cosval) SINCOS_VECTOR_BODY(float, 4, sincosf)

_CLC_OVERLOAD _CLC_DEF float8 sincos(float8 x, global  float8 * cosval) SINCOS_VECTOR_BODY(float, 8, sincosf)
_CLC_OVERLOAD _CLC_DEF float8 sincos(float8 x, local   float8 * cosval) SINCOS_VECTOR_BODY(float, 8, sincosf)
_CLC_OVERLOAD _CLC_DEF float8 sincos(float8 x, private float8 * cosval) SINCOS_VECTOR_BODY(float, 8, sincosf)

_CLC_OVERLOAD _CLC_DEF float16 sincos(float16 x, global  float16 * cosval) SINCOS_VECTOR_BODY(float, 16, sincosf)
_CLC_OVERLOAD _CLC_DEF float16 sincos(float16 x, local   float16 * cosval) SINCOS_VECTOR_BODY(float, 16, sincosf)
_CLC_OVERLOAD _CLC_DEF float16 sincos(float16 x, private float16 * cosval) SINCOS_VECTOR_BODY(float, 16, sincosf)

_CLC_OVERLOAD _CLC_DEF double sincos(double x, global  double * cosval) SINCOS_SCALAR_BODY(double, sincosd)
_CLC_OVERLOAD _CLC_DEF double sincos(double x, local   double * cosval) SINCOS_SCALAR_BODY(double, sincosd)
_CLC_OVERLOAD _CLC_DEF double sincos(double x, private double * cosval) SINCOS_SCALAR_BODY(double, sincosd)

_CLC_OVERLOAD _CLC_DEF double2 sincos(double2 x, global  double2 * cosval) SINCOS_VECTOR_BODY(double, 2, sincosd)
_CLC_OVERLOAD _CLC_DEF double2 sincos(double2 x, local   double2 * cosval) SINCOS_VECTOR_BODY(double, 2, sincosd)
_CLC_OVERLOAD _CLC_DEF double2 sincos(double2 x, private double2 * cosval) SINCOS_VECTOR_BODY(double, 2, sincosd)

_CLC_OVERLOAD _CLC_DEF double3 sincos(double3 x, global  double3 * cosval) SINCOS_VECTOR_BODY(double, 3, sincosd)
_CLC_OVERLOAD _CLC_DEF double3 sincos(double3 x, local   double3 * cosval) SINCOS_VECTOR_BODY(double, 3, sincosd)
_CLC_OVERLOAD _CLC_DEF double3 sincos(double3 x, private double3 * cosval) SINCOS_VECTOR_BODY(double, 3, sincosd)

_CLC_OVERLOAD _CLC_DEF double4 sincos(double4 x, global  double4 * cosval) SINCOS_VECTOR_BODY(double, 4, sincosd)
_CLC_OVERLOAD _CLC_DEF double4 sincos(double4 x, local   double4 * cosval) SINCOS_VECTOR_BODY(double, 4, sincosd)
_CLC_OVERLOAD _CLC_DEF double4 sincos(double4 x, private double4 * cosval) SINCOS_VECTOR_BODY(double, 4, sincosd)

_CLC_OVERLOAD _CLC_DEF double8 sincos(double8 x, global  double8 * cosval) SINCOS_VECTOR_BODY(double, 8, sincosd)
_CLC_OVERLOAD _CLC_DEF double8 sincos(double8 x, local   double8 * cosval) SINCOS_VECTOR_BODY(double, 8, sincosd)
_CLC_OVERLOAD _CLC_DEF double8 sincos(double8 x, private double8 * cosval) SINCOS_VECTOR_BODY(double, 8, sincosd)

_CLC_OVERLOAD _CLC_DEF double16 sincos(double16 x, global  double16 * cosval) SINCOS_VECTOR_BODY(double, 16, sincosd)
_CLC_OVERLOAD _CLC_DEF double16 sincos(double16 x, local   double16 * cosval) SINCOS_VECTOR_BODY(double, 16, sincosd)
_CLC_OVERLOAD _CLC_DEF double16 sincos(double16 x, private double16 * cosval) SINCOS_VECTOR_BODY(double, 16, sincosd)


