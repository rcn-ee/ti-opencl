/******************************************************************************
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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

#define DEFN(tname) \
_CLC_OVERLOAD _CLC_DEF tname bitselect(tname a, tname b, tname c) { return a^(c&(b^a)); }

DEFN(char2)
DEFN(uchar2)
DEFN(long2)
DEFN(ulong2)

DEFN(char3)
DEFN(uchar3)
DEFN(short3)
DEFN(ushort3)
DEFN(int3)
DEFN(uint3)
DEFN(long3)
DEFN(ulong3)

DEFN(int4)
DEFN(uint4)
DEFN(long4)
DEFN(ulong4)

DEFN(short8)
DEFN(ushort8)
DEFN(int8)
DEFN(uint8)
DEFN(long8)
DEFN(ulong8)

DEFN(char16)
DEFN(uchar16)
DEFN(short16)
DEFN(ushort16)
DEFN(int16)
DEFN(uint16)
DEFN(long16)
DEFN(ulong16)

_CLC_OVERLOAD _CLC_DEF float bitselect (float a, float b, float c)
{ return __builtin_astype(__builtin_astype(a,int)^(__builtin_astype(c,int)&(__builtin_astype(b,int)^__builtin_astype(a,int))), float); }
_CLC_OVERLOAD _CLC_DEF float2 bitselect (float2 a, float2 b, float2 c)
{ return __builtin_astype(__builtin_astype(a,int2)^(__builtin_astype(c,int2)&(__builtin_astype(b,int2)^__builtin_astype(a,int2))), float2); }
_CLC_OVERLOAD _CLC_DEF float3 bitselect (float3 a, float3 b, float3 c)
{ return __builtin_astype(__builtin_astype(a,int3)^(__builtin_astype(c,int3)&(__builtin_astype(b,int3)^__builtin_astype(a,int3))), float3); }
_CLC_OVERLOAD _CLC_DEF float4 bitselect (float4 a, float4 b, float4 c)
{ return __builtin_astype(__builtin_astype(a,int4)^(__builtin_astype(c,int4)&(__builtin_astype(b,int4)^__builtin_astype(a,int4))), float4); }
_CLC_OVERLOAD _CLC_DEF float8 bitselect (float8 a, float8 b, float8 c)
{ return __builtin_astype(__builtin_astype(a,int8)^(__builtin_astype(c,int8)&(__builtin_astype(b,int8)^__builtin_astype(a,int8))), float8); }
_CLC_OVERLOAD _CLC_DEF float16 bitselect (float16 a, float16 b, float16 c)
{ return __builtin_astype(__builtin_astype(a,int16)^(__builtin_astype(c,int16)&(__builtin_astype(b,int16)^__builtin_astype(a,int16))), float16); }

_CLC_OVERLOAD _CLC_DEF double bitselect (double a, double b, double c)
{ return __builtin_astype(__builtin_astype(a,long)^(__builtin_astype(c,long)&(__builtin_astype(b,long)^__builtin_astype(a,long))), double); }
_CLC_OVERLOAD _CLC_DEF double2 bitselect (double2 a, double2 b, double2 c)
{ return __builtin_astype(__builtin_astype(a,long2)^(__builtin_astype(c,long2)&(__builtin_astype(b,long2)^__builtin_astype(a,long2))), double2); }
_CLC_OVERLOAD _CLC_DEF double3 bitselect (double3 a, double3 b, double3 c)
{ return __builtin_astype(__builtin_astype(a,long3)^(__builtin_astype(c,long3)&(__builtin_astype(b,long3)^__builtin_astype(a,long3))), double3); }
_CLC_OVERLOAD _CLC_DEF double4 bitselect (double4 a, double4 b, double4 c)
{ return __builtin_astype(__builtin_astype(a,long4)^(__builtin_astype(c,long4)&(__builtin_astype(b,long4)^__builtin_astype(a,long4))), double4); }
_CLC_OVERLOAD _CLC_DEF double8 bitselect (double8 a, double8 b, double8 c)
{ return __builtin_astype(__builtin_astype(a,long8)^(__builtin_astype(c,long8)&(__builtin_astype(b,long8)^__builtin_astype(a,long8))), double8); }
_CLC_OVERLOAD _CLC_DEF double16 bitselect (double16 a, double16 b, double16 c)
{ return __builtin_astype(__builtin_astype(a,long16)^(__builtin_astype(c,long16)&(__builtin_astype(b,long16)^__builtin_astype(a,long16))), double16); }
