/******************************************************************************
 * Copyright (c) 2011-2013, Peter Collingbourne <peter@pcc.me.uk>
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
/******************************************************************************
* AS_<type> functions
******************************************************************************/
#ifndef __ASTYPE_H_
#define __ASTYPE_H_

#define as_char(x)      __builtin_astype(x, char)
#define as_uchar(x)     __builtin_astype(x, uchar)
#define as_short(x)     __builtin_astype(x, short)
#define as_ushort(x)    __builtin_astype(x, ushort)
#define as_int(x)       __builtin_astype(x, int)
#define as_uint(x)      __builtin_astype(x, uint)
#define as_long(x)      __builtin_astype(x, long)
#define as_ulong(x)     __builtin_astype(x, ulong)
#define as_float(x)     __builtin_astype(x, float)
#define as_double(x)    __builtin_astype(x, double)

#define as_char2(x)     __builtin_astype(x, char2)
#define as_uchar2(x)    __builtin_astype(x, uchar2)
#define as_short2(x)    __builtin_astype(x, short2)
#define as_ushort2(x)   __builtin_astype(x, ushort2)
#define as_int2(x)      __builtin_astype(x, int2)
#define as_uint2(x)     __builtin_astype(x, uint2)
#define as_long2(x)     __builtin_astype(x, long2)
#define as_ulong2(x)    __builtin_astype(x, ulong2)
#define as_float2(x)    __builtin_astype(x, float2)
#define as_double2(x)   __builtin_astype(x, double2)

#define as_char3(x)     __builtin_astype(x, char3)
#define as_uchar3(x)    __builtin_astype(x, uchar3)
#define as_short3(x)    __builtin_astype(x, short3)
#define as_ushort3(x)   __builtin_astype(x, ushort3)
#define as_int3(x)      __builtin_astype(x, int3)
#define as_uint3(x)     __builtin_astype(x, uint3)
#define as_long3(x)     __builtin_astype(x, long3)
#define as_ulong3(x)    __builtin_astype(x, ulong3)
#define as_float3(x)    __builtin_astype(x, float3)
#define as_double3(x)   __builtin_astype(x, double3)

#define as_char4(x)     __builtin_astype(x, char4)
#define as_uchar4(x)    __builtin_astype(x, uchar4)
#define as_short4(x)    __builtin_astype(x, short4)
#define as_ushort4(x)   __builtin_astype(x, ushort4)
#define as_int4(x)      __builtin_astype(x, int4)
#define as_uint4(x)     __builtin_astype(x, uint4)
#define as_long4(x)     __builtin_astype(x, long4)
#define as_ulong4(x)    __builtin_astype(x, ulong4)
#define as_float4(x)    __builtin_astype(x, float4)
#define as_double4(x)   __builtin_astype(x, double4)

#define as_char8(x)     __builtin_astype(x, char8)
#define as_uchar8(x)    __builtin_astype(x, uchar8)
#define as_short8(x)    __builtin_astype(x, short8)
#define as_ushort8(x)   __builtin_astype(x, ushort8)
#define as_int8(x)      __builtin_astype(x, int8)
#define as_uint8(x)     __builtin_astype(x, uint8)
#define as_long8(x)     __builtin_astype(x, long8)
#define as_ulong8(x)    __builtin_astype(x, ulong8)
#define as_float8(x)    __builtin_astype(x, float8)
#define as_double8(x)   __builtin_astype(x, double8)

#define as_char16(x)    __builtin_astype(x, char16)
#define as_uchar16(x)   __builtin_astype(x, uchar16)
#define as_short16(x)   __builtin_astype(x, short16)
#define as_ushort16(x)  __builtin_astype(x, ushort16)
#define as_int16(x)     __builtin_astype(x, int16)
#define as_uint16(x)    __builtin_astype(x, uint16)
#define as_long16(x)    __builtin_astype(x, long16)
#define as_ulong16(x)   __builtin_astype(x, ulong16)
#define as_float16(x)   __builtin_astype(x, float16)
#define as_double16(x)  __builtin_astype(x, double16)

#endif // __ASTYPE_H_
