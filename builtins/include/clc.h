/*
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2011-2014, Peter Collingbourne <peter@pcc.me.uk>
 * Copyright (c) 2013 Victor Oliveira <victormatheus@gmail.com>
 * Copyright (c) 2013 Jesse Towner <jessetowner@lavabit.com>
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

#ifndef _CLC_H_
#define _CLC_H_

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define _CLC_PROTECTED __attribute__((visibility("protected")))
#define _CLC_OVERLOAD  __attribute__((overloadable))
#define _CLC_DECL      __attribute__((visibility("protected")))
#define _CLC_DEF       __attribute__((visibility("protected")))
#define _CLC_INLINE    __attribute__((always_inline)) inline
#define _CLC_NODUP     __attribute__((noduplicate))

#define _UNARY_VEC_DECL(__type,__utype,__name) \
_CLC_OVERLOAD _CLC_DECL __utype##2  __name(__type##2 __x); \
_CLC_OVERLOAD _CLC_DECL __utype##3  __name(__type##3 __x); \
_CLC_OVERLOAD _CLC_DECL __utype##4  __name(__type##4 __x); \
_CLC_OVERLOAD _CLC_DECL __utype##8  __name(__type##8 __x); \
_CLC_OVERLOAD _CLC_DECL __utype##16 __name(__type##16 __x);\

#define _BINARY_VEC_DECL(__type,__utype,__name) \
_CLC_OVERLOAD _CLC_DECL __utype##2   __name(__type##2 __x, __type##2 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##3   __name(__type##3 __x, __type##3 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##4   __name(__type##4 __x, __type##4 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##8   __name(__type##8 __x, __type##8 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##16  __name(__type##16 __x, __type##16 __y);\

#define _BINARY_VEC_DECL_ALT(__type,__utype,__type2,__name) \
_CLC_OVERLOAD _CLC_DECL __utype##2   __name(__type##2 __x, __type2##2 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##3   __name(__type##3 __x, __type2##3 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##4   __name(__type##4 __x, __type2##4 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##8   __name(__type##8 __x, __type2##8 __y);  \
_CLC_OVERLOAD _CLC_DECL __utype##16  __name(__type##16 __x, __type2##16 __y);\

#define _TERNARY_VEC_DECL(__type,__utype,__name) \
_CLC_OVERLOAD _CLC_DECL __utype##2   __name(__type##2 __x, __type##2 __y,  __type##2 __z);  \
_CLC_OVERLOAD _CLC_DECL __utype##3   __name(__type##3 __x, __type##3 __y,  __type##3 __z);  \
_CLC_OVERLOAD _CLC_DECL __utype##4   __name(__type##4 __x, __type##4 __y,  __type##4 __z);  \
_CLC_OVERLOAD _CLC_DECL __utype##8   __name(__type##8 __x, __type##8 __y,  __type##8 __z);  \
_CLC_OVERLOAD _CLC_DECL __utype##16  __name(__type##16 __x, __type##16 __y,__type##16 __z);\

#define _UNARY_INLINE(__type,__utype,__name,__op) \
_CLC_PROTECTED __utype __op(__type __x); \
_CLC_OVERLOAD _CLC_INLINE __utype __name(__type __x) { return __op(__x); }

#define _BINARY_INLINE(__type,__utype,__name,__op) \
_CLC_PROTECTED __utype __op(__type __x, __type __y); \
_CLC_OVERLOAD _CLC_INLINE __utype  __name(__type __x, __type __y) { return __op(__x, __y); }

#define _BINARY_INLINE_ALT(__type,__utype,__type2,__name,__op) \
_CLC_PROTECTED __utype __op(__type __x, __type2 __y); \
_CLC_OVERLOAD _CLC_INLINE __utype  __name(__type __x, __type2 __y) { return __op(__x, __y); }

#define _UNARY_VEC_DEF(__type,__utype,__name,__op)\
_CLC_OVERLOAD _CLC_DEF __utype##2  __name(__type##2 __x)   \
{ return (__utype##2)  (__op(__x.s0), __op(__x.s1)); }\
_CLC_OVERLOAD _CLC_DEF __utype##3  __name(__type##3 __x)   \
{ return (__utype##3)  (__op(__x.s0), __op(__x.s1), __op(__x.s2)); }\
_CLC_OVERLOAD _CLC_DEF __utype##4  __name(__type##4 __x)   \
{ return (__utype##4)  (__op(__x.s0), __op(__x.s1), __op(__x.s2), __op(__x.s3)); }\
_CLC_OVERLOAD _CLC_DEF __utype##8  __name(__type##8 __x)   \
{ return (__utype##8)  (__op(__x.s0), __op(__x.s1), __op(__x.s2), __op(__x.s3),\
                     __op(__x.s4), __op(__x.s5), __op(__x.s6), __op(__x.s7)); }\
_CLC_OVERLOAD _CLC_DEF __utype##16  __name(__type##16 __x)   \
{ return (__utype##16)  (__op(__x.s0), __op(__x.s1), __op(__x.s2), __op(__x.s3),\
                      __op(__x.s4), __op(__x.s5), __op(__x.s6), __op(__x.s7),\
                      __op(__x.s8), __op(__x.s9), __op(__x.sa), __op(__x.sb),\
                      __op(__x.sc), __op(__x.sd), __op(__x.se), __op(__x.sf)); }

#define _BINARY_VEC_DEF(__type,__utype,__name,__op)\
_CLC_OVERLOAD _CLC_DEF __utype##2  __name(__type##2 __x, __type##2 __y)   \
{ return (__utype##2)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1)); }\
_CLC_OVERLOAD _CLC_DEF __utype##3  __name(__type##3 __x, __type##3 __y)   \
{ return (__utype##3)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2)); }\
_CLC_OVERLOAD _CLC_DEF __utype##4  __name(__type##4 __x, __type##4 __y)   \
{ return (__utype##4)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2), __op(__x.s3,__y.s3)); }\
_CLC_OVERLOAD _CLC_DEF __utype##8  __name(__type##8 __x, __type##8 __y)   \
{ return (__utype##8)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2), __op(__x.s3,__y.s3),\
                     __op(__x.s4,__y.s4), __op(__x.s5,__y.s5), __op(__x.s6,__y.s6), __op(__x.s7,__y.s7)); }\
_CLC_OVERLOAD _CLC_DEF __utype##16  __name(__type##16 __x, __type##16 __y)   \
{ return (__utype##16)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2), __op(__x.s3,__y.s3),\
                      __op(__x.s4,__y.s4), __op(__x.s5,__y.s5), __op(__x.s6,__y.s6), __op(__x.s7,__y.s7),\
                      __op(__x.s8,__y.s8), __op(__x.s9,__y.s9), __op(__x.sa,__y.sa), __op(__x.sb,__y.sb),\
                      __op(__x.sc,__y.sc), __op(__x.sd,__y.sd), __op(__x.se,__y.se), __op(__x.sf,__y.sf)); }

#define _BINARY_VEC_DEF_ALT(__type,__utype,__type2,__name,__op)\
_CLC_OVERLOAD _CLC_DEF __utype##2  __name(__type##2 __x, __type2##2 __y)   \
{ return (__utype##2)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1)); }\
_CLC_OVERLOAD _CLC_DEF __utype##3  __name(__type##3 __x, __type2##3 __y)   \
{ return (__utype##3)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2)); }\
_CLC_OVERLOAD _CLC_DEF __utype##4  __name(__type##4 __x, __type2##4 __y)   \
{ return (__utype##4)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2), __op(__x.s3,__y.s3)); }\
_CLC_OVERLOAD _CLC_DEF __utype##8  __name(__type##8 __x, __type2##8 __y)   \
{ return (__utype##8)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2), __op(__x.s3,__y.s3),\
                     __op(__x.s4,__y.s4), __op(__x.s5,__y.s5), __op(__x.s6,__y.s6), __op(__x.s7,__y.s7)); }\
_CLC_OVERLOAD _CLC_DEF __utype##16  __name(__type##16 __x, __type2##16 __y)   \
{ return (__utype##16)  (__op(__x.s0,__y.s0), __op(__x.s1,__y.s1), __op(__x.s2,__y.s2), __op(__x.s3,__y.s3),\
                      __op(__x.s4,__y.s4), __op(__x.s5,__y.s5), __op(__x.s6,__y.s6), __op(__x.s7,__y.s7),\
                      __op(__x.s8,__y.s8), __op(__x.s9,__y.s9), __op(__x.sa,__y.sa), __op(__x.sb,__y.sb),\
                      __op(__x.sc,__y.sc), __op(__x.sd,__y.sd), __op(__x.se,__y.se), __op(__x.sf,__y.sf)); }

#define _TERNARY_VEC_DEF_INLINE(__def_inline,__type,__utype,__name,__op)\
_CLC_OVERLOAD __def_inline __utype##2  __name(__type##2 __x, __type##2 __y, __type##2 __z)   \
{ return (__utype##2)  (__op(__x.s0,__y.s0,__z.s0), __op(__x.s1,__y.s1,__z.s1)); }\
_CLC_OVERLOAD __def_inline __utype##3  __name(__type##3 __x, __type##3 __y, __type##3 __z)   \
{ return (__utype##3)  (__op(__x.s0,__y.s0,__z.s0), __op(__x.s1,__y.s1,__z.s1), __op(__x.s2,__y.s2,__z.s2)); }\
_CLC_OVERLOAD __def_inline __utype##4  __name(__type##4 __x, __type##4 __y, __type##4 __z)   \
{ return (__utype##4)  (__op(__x.s0,__y.s0,__z.s0), __op(__x.s1,__y.s1,__z.s1), \
                      __op(__x.s2,__y.s2,__z.s2), __op(__x.s3,__y.s3,__z.s3)); }\
_CLC_OVERLOAD __def_inline __utype##8  __name(__type##8 __x, __type##8 __y, __type##8 __z)   \
{ return (__utype##8)  (__op(__x.s0,__y.s0,__z.s0), __op(__x.s1,__y.s1,__z.s1), \
                      __op(__x.s2,__y.s2,__z.s2), __op(__x.s3,__y.s3,__z.s3),\
                      __op(__x.s4,__y.s4,__z.s4), __op(__x.s5,__y.s5,__z.s5), \
                      __op(__x.s6,__y.s6,__z.s6), __op(__x.s7,__y.s7,__z.s7)); }\
_CLC_OVERLOAD __def_inline __utype##16  __name(__type##16 __x, __type##16 __y, __type##16 __z)   \
{ return (__utype##16)  (__op(__x.s0,__y.s0,__z.s0), __op(__x.s1,__y.s1,__z.s1), \
                       __op(__x.s2,__y.s2,__z.s2), __op(__x.s3,__y.s3,__z.s3),\
                       __op(__x.s4,__y.s4,__z.s4), __op(__x.s5,__y.s5,__z.s5), \
                       __op(__x.s6,__y.s6,__z.s6), __op(__x.s7,__y.s7,__z.s7),\
                       __op(__x.s8,__y.s8,__z.s8), __op(__x.s9,__y.s9,__z.s9), \
                       __op(__x.sa,__y.sa,__z.sa), __op(__x.sb,__y.sb,__z.sb),\
                       __op(__x.sc,__y.sc,__z.sc), __op(__x.sd,__y.sd,__z.sd), \
                       __op(__x.se,__y.se,__z.se), __op(__x.sf,__y.sf,__z.sf)); }

#define _TERNARY_VEC_DEF(__type,__utype,__name,__op)    _TERNARY_VEC_DEF_INLINE(_CLC_DEF,   __type,__utype,__name,__op)
#define _TERNARY_VEC_INLINE(__type,__utype,__name,__op) _TERNARY_VEC_DEF_INLINE(_CLC_INLINE,__type,__utype,__name,__op)


#define _VEC_TYPE(__type,__sz) __type##__sz

#define _EXPAND_TYPES()   \
    _EXPAND_SIZES(char)   \
    _EXPAND_SIZES(uchar)  \
    _EXPAND_SIZES(short)  \
    _EXPAND_SIZES(ushort) \
    _EXPAND_SIZES(int)    \
    _EXPAND_SIZES(uint)   \
    _EXPAND_SIZES(long)   \
    _EXPAND_SIZES(ulong)  \
    _EXPAND_SIZES(float)  \
    _EXPAND_SIZES(double) 

#define _EXPAND_INTEGER_TYPES()   \
    _EXPAND_SIZES(char)   \
    _EXPAND_SIZES(uchar)  \
    _EXPAND_SIZES(short)  \
    _EXPAND_SIZES(ushort) \
    _EXPAND_SIZES(int)    \
    _EXPAND_SIZES(uint)   \
    _EXPAND_SIZES(long)   \
    _EXPAND_SIZES(ulong)  

typedef unsigned int cl_mem_fence_flags;

/*-----------------------------------------------------------------------------
* Standard types from Clang's stddef and stdint, Copyright (C) 2008 Eli Friedman
*----------------------------------------------------------------------------*/
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT8_TYPE__  int8_t;
typedef __UINT8_TYPE__  uint8_t;

#define __stdint_join3(__a,__b,__c) __a ## __b ## __c
#define  __intn_t(__n) __stdint_join3( int, __n, _t)
#define __uintn_t(__n) __stdint_join3(uint, __n, _t)

typedef __typeof__(((int*)0)-((int*)0)) ptrdiff_t;
typedef __typeof__(sizeof(int))         size_t;
typedef  __intn_t(__INTPTR_WIDTH__)     intptr_t;
typedef __uintn_t(__INTPTR_WIDTH__)     uintptr_t;

#undef __stdint_join3
#undef  __intn_t
#undef __uintn_t

/*-----------------------------------------------------------------------------
* OpenCL types 
*----------------------------------------------------------------------------*/
typedef uint8_t  uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

#if defined(CLANG_OLDER_THAN_3_3)
typedef unsigned int    sampler_t;
typedef struct image2d *image2d_t;
typedef struct image3d *image3d_t;
#endif

/*-----------------------------------------------------------------------------
* Vectors
*----------------------------------------------------------------------------*/
#define _COAL_VECTOR(__type, __len)                                  \
   typedef __type __type##__len __attribute__((ext_vector_type(__len)))

#define _COAL_VECTOR_SET(__type) \
   _COAL_VECTOR(__type, 2);      \
   _COAL_VECTOR(__type, 3);      \
   _COAL_VECTOR(__type, 4);      \
   _COAL_VECTOR(__type, 8);      \
   _COAL_VECTOR(__type, 16);

_COAL_VECTOR_SET(char)
_COAL_VECTOR_SET(uchar)
_COAL_VECTOR_SET(short)
_COAL_VECTOR_SET(ushort)
_COAL_VECTOR_SET(int)
_COAL_VECTOR_SET(uint)
_COAL_VECTOR_SET(long)
_COAL_VECTOR_SET(ulong)
_COAL_VECTOR_SET(float)
_COAL_VECTOR_SET(double)

#undef _COAL_VECTOR_SET
#undef _COAL_VECTOR

#define CL_VERSION_1_0          100
#define CL_VERSION_1_1          110
#define __OPENCL_VERSION__      110
#define __ENDIAN_LITTLE__       1
#define __kernel_exec(__X, __typen) __kernel __attribute__((work_group_size_hint(__X, 1, 1))) \
                                __attribute__((vec_type_hint(__typen)))
#define kernel_exec             __kernel_exec

#define __write_only
#define __read_only             const

#define write_only              __write_only
#define read_only               __read_only

#define CLK_NORMALIZED_COORDS_FALSE 0x00000000
#define CLK_NORMALIZED_COORDS_TRUE  0x00000001
#define CLK_ADDRESS_NONE            0x00000000
#define CLK_ADDRESS_MIRRORED_REPEAT 0x00000010
#define CLK_ADDRESS_REPEAT          0x00000020
#define CLK_ADDRESS_CLAMP_TO_EDGE   0x00000030
#define CLK_ADDRESS_CLAMP           0x00000040
#define CLK_FILTER_NEAREST          0x00000000
#define CLK_FILTER_LINEAR           0x00000100
#define CLK_LOCAL_MEM_FENCE         0x00000001
#define CLK_GLOBAL_MEM_FENCE        0x00000002
#define CLK_R                       0x10B0
#define CLK_A                       0x10B1
#define CLK_RG                      0x10B2
#define CLK_RA                      0x10B3
#define CLK_RGB                     0x10B4
#define CLK_RGBA                    0x10B5
#define CLK_BGRA                    0x10B6
#define CLK_ARGB                    0x10B7
#define CLK_INTENSITY               0x10B8
#define CLK_LUMINANCE               0x10B9
#define CLK_Rx                      0x10BA
#define CLK_RGx                     0x10BB
#define CLK_RGBx                    0x10BC
#define CLK_SNORM_INT8              0x10D0
#define CLK_SNORM_INT16             0x10D1
#define CLK_UNORM_INT8              0x10D2
#define CLK_UNORM_INT16             0x10D3
#define CLK_UNORM_SHORT_565         0x10D4
#define CLK_UNORM_SHORT_555         0x10D5
#define CLK_UNORM_INT_101010        0x10D6
#define CLK_SIGNED_INT8             0x10D7
#define CLK_SIGNED_INT16            0x10D8
#define CLK_SIGNED_INT32            0x10D9
#define CLK_UNSIGNED_INT8           0x10DA
#define CLK_UNSIGNED_INT16          0x10DB
#define CLK_UNSIGNED_INT32          0x10DC
#define CLK_HALF_FLOAT              0x10DD
#define CLK_FLOAT                   0x10DE

_CLC_PROTECTED _CLC_NODUP void  barrier(cl_mem_fence_flags __flags);
_CLC_PROTECTED void   mem_fence        (cl_mem_fence_flags __flags);
_CLC_PROTECTED void   read_mem_fence   (cl_mem_fence_flags __flags);
_CLC_PROTECTED void   write_mem_fence  (cl_mem_fence_flags __flags);

/******************************************************************************
* AS_<type> functions
******************************************************************************/
#define as_char(__x)      __builtin_astype(__x, char)
#define as_uchar(__x)     __builtin_astype(__x, uchar)
#define as_short(__x)     __builtin_astype(__x, short)
#define as_ushort(__x)    __builtin_astype(__x, ushort)
#define as_int(__x)       __builtin_astype(__x, int)
#define as_uint(__x)      __builtin_astype(__x, uint)
#define as_long(__x)      __builtin_astype(__x, long)
#define as_ulong(__x)     __builtin_astype(__x, ulong)
#define as_float(__x)     __builtin_astype(__x, float)
#define as_double(__x)    __builtin_astype(__x, double)

#define as_char2(__x)     __builtin_astype(__x, char2)
#define as_uchar2(__x)    __builtin_astype(__x, uchar2)
#define as_short2(__x)    __builtin_astype(__x, short2)
#define as_ushort2(__x)   __builtin_astype(__x, ushort2)
#define as_int2(__x)      __builtin_astype(__x, int2)
#define as_uint2(__x)     __builtin_astype(__x, uint2)
#define as_long2(__x)     __builtin_astype(__x, long2)
#define as_ulong2(__x)    __builtin_astype(__x, ulong2)
#define as_float2(__x)    __builtin_astype(__x, float2)
#define as_double2(__x)   __builtin_astype(__x, double2)

#define as_char3(__x)     __builtin_astype(__x, char3)
#define as_uchar3(__x)    __builtin_astype(__x, uchar3)
#define as_short3(__x)    __builtin_astype(__x, short3)
#define as_ushort3(__x)   __builtin_astype(__x, ushort3)
#define as_int3(__x)      __builtin_astype(__x, int3)
#define as_uint3(__x)     __builtin_astype(__x, uint3)
#define as_long3(__x)     __builtin_astype(__x, long3)
#define as_ulong3(__x)    __builtin_astype(__x, ulong3)
#define as_float3(__x)    __builtin_astype(__x, float3)
#define as_double3(__x)   __builtin_astype(__x, double3)

#define as_char4(__x)     __builtin_astype(__x, char4)
#define as_uchar4(__x)    __builtin_astype(__x, uchar4)
#define as_short4(__x)    __builtin_astype(__x, short4)
#define as_ushort4(__x)   __builtin_astype(__x, ushort4)
#define as_int4(__x)      __builtin_astype(__x, int4)
#define as_uint4(__x)     __builtin_astype(__x, uint4)
#define as_long4(__x)     __builtin_astype(__x, long4)
#define as_ulong4(__x)    __builtin_astype(__x, ulong4)
#define as_float4(__x)    __builtin_astype(__x, float4)
#define as_double4(__x)   __builtin_astype(__x, double4)

#define as_char8(__x)     __builtin_astype(__x, char8)
#define as_uchar8(__x)    __builtin_astype(__x, uchar8)
#define as_short8(__x)    __builtin_astype(__x, short8)
#define as_ushort8(__x)   __builtin_astype(__x, ushort8)
#define as_int8(__x)      __builtin_astype(__x, int8)
#define as_uint8(__x)     __builtin_astype(__x, uint8)
#define as_long8(__x)     __builtin_astype(__x, long8)
#define as_ulong8(__x)    __builtin_astype(__x, ulong8)
#define as_float8(__x)    __builtin_astype(__x, float8)
#define as_double8(__x)   __builtin_astype(__x, double8)

#define as_char16(__x)    __builtin_astype(__x, char16)
#define as_uchar16(__x)   __builtin_astype(__x, uchar16)
#define as_short16(__x)   __builtin_astype(__x, short16)
#define as_ushort16(__x)  __builtin_astype(__x, ushort16)
#define as_int16(__x)     __builtin_astype(__x, int16)
#define as_uint16(__x)    __builtin_astype(__x, uint16)
#define as_long16(__x)    __builtin_astype(__x, long16)
#define as_ulong16(__x)   __builtin_astype(__x, ulong16)
#define as_float16(__x)   __builtin_astype(__x, float16)
#define as_double16(__x)  __builtin_astype(__x, double16)

#define _CLC_CONVERT_DECL(_FROM_TYPE, _TO_TYPE, _SUFFIX) \
  _CLC_OVERLOAD _CLC_DECL _TO_TYPE convert_##_TO_TYPE##_SUFFIX(_FROM_TYPE __x);

#define _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, _TO_TYPE, _SUFFIX) \
  _CLC_CONVERT_DECL(_FROM_TYPE, _TO_TYPE, _SUFFIX) \
  _CLC_CONVERT_DECL(_FROM_TYPE##2, _TO_TYPE##2, _SUFFIX) \
  _CLC_CONVERT_DECL(_FROM_TYPE##3, _TO_TYPE##3, _SUFFIX) \
  _CLC_CONVERT_DECL(_FROM_TYPE##4, _TO_TYPE##4, _SUFFIX) \
  _CLC_CONVERT_DECL(_FROM_TYPE##8, _TO_TYPE##8, _SUFFIX) \
  _CLC_CONVERT_DECL(_FROM_TYPE##16, _TO_TYPE##16, _SUFFIX)

#define _CLC_VECTOR_CONVERT_FROM1(_FROM_TYPE, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, char, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, uchar, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, int, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, uint, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, short, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, ushort, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, long, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, ulong, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, float, _SUFFIX)

#define _CLC_VECTOR_CONVERT_FROM(_FROM_TYPE, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM1(_FROM_TYPE, _SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(_FROM_TYPE, double, _SUFFIX)

#define _CLC_VECTOR_CONVERT_TO1(_SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(char, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(uchar, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(int, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(uint, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(short, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(ushort, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(long, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(ulong, _SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(float, _SUFFIX)

#define _CLC_VECTOR_CONVERT_TO(_SUFFIX) \
  _CLC_VECTOR_CONVERT_TO1(_SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(double, _SUFFIX)

#define _CLC_VECTOR_CONVERT_TO_SUFFIX(_ROUND) \
  _CLC_VECTOR_CONVERT_TO(_sat##_ROUND) \
  _CLC_VECTOR_CONVERT_TO(_ROUND)

_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtn)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rte)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtz)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtp)
_CLC_VECTOR_CONVERT_TO_SUFFIX()

#define _VLOAD_HALF_VECTORIZE(_SPACE) \
_CLC_OVERLOAD _CLC_DEF float  vload_half(size_t __offset, const _SPACE half *__p);  \
_CLC_OVERLOAD _CLC_DEF float2 vload_half2(size_t __offset, const _SPACE half *__p); \
_CLC_OVERLOAD _CLC_DEF float3 vload_half3(size_t __offset, const _SPACE half *__p); \
_CLC_OVERLOAD _CLC_DEF float3 vloada_half3(size_t __offset, const _SPACE half *__p);\
_CLC_OVERLOAD _CLC_DEF float4 vload_half4(size_t __offset, const _SPACE half *__p); \
_CLC_OVERLOAD _CLC_DEF float8 vload_half8(size_t __offset, const _SPACE half *__p); \
_CLC_OVERLOAD _CLC_DEF float16 vload_half16(size_t __offset, const _SPACE half *__p);

_VLOAD_HALF_VECTORIZE(__global)
_VLOAD_HALF_VECTORIZE(__local)
_VLOAD_HALF_VECTORIZE(__constant)
_VLOAD_HALF_VECTORIZE(__private)

#define vloada_half vload_half
#define vloada_half2 vload_half2
#define vloada_half4 vload_half4
#define vloada_half8 vload_half8
#define vloada_half16 vload_half16

#undef _VLOAD_HALF_VECTORIZE

#define _DECL_VSTORE_HALF_SPACE_ROUND(_SPACE, _ROUND, _FUNC) \
   _CLC_OVERLOAD _CLC_DECL void vstore_half##_ROUND(float __data, size_t __offset, _SPACE half *__p); \
   _CLC_OVERLOAD _CLC_DECL void vstorea_half##_ROUND(float __data, size_t __offset, _SPACE half *__p); \
   _CLC_OVERLOAD _CLC_DECL void vstore_half2##_ROUND(float2 __data, size_t __offset, _SPACE half *__p);\
   _CLC_OVERLOAD _CLC_DECL void vstorea_half2##_ROUND(float2 __data, size_t __offset, _SPACE half *__p);\
   _CLC_OVERLOAD _CLC_DECL void vstore_half3##_ROUND(float3 __data, size_t __offset, _SPACE half *__p); \
   _CLC_OVERLOAD _CLC_DECL void vstorea_half3##_ROUND(float3 __data, size_t __offset, _SPACE half *__p); \
   _CLC_OVERLOAD _CLC_DECL void vstore_half4##_ROUND(float4 __data, size_t __offset, _SPACE half *__p); \
   _CLC_OVERLOAD _CLC_DECL void vstorea_half4##_ROUND(float4 __data, size_t __offset, _SPACE half *__p);\
   _CLC_OVERLOAD _CLC_DECL void vstore_half8##_ROUND(float8 __data, size_t __offset, _SPACE half *__p);\
   _CLC_OVERLOAD _CLC_DECL void vstorea_half8##_ROUND(float8 __data, size_t __offset, _SPACE half *__p);\
   _CLC_OVERLOAD _CLC_DECL void vstore_half16##_ROUND(float16 __data, size_t __offset, _SPACE half *__p);\
   _CLC_OVERLOAD _CLC_DECL void vstorea_half16##_ROUND(float16 __data, size_t __offset, _SPACE half *__p);

#define _DECL_VSTORE_HALF_SPACE(_SPACE) \
  _DECL_VSTORE_HALF_SPACE_ROUND(_SPACE,     , __ocl_float_to_half) \
  _DECL_VSTORE_HALF_SPACE_ROUND(_SPACE, _rte, __ocl_float_to_half_rte) \
  _DECL_VSTORE_HALF_SPACE_ROUND(_SPACE, _rtz, __ocl_float_to_half_rtz) \
  _DECL_VSTORE_HALF_SPACE_ROUND(_SPACE, _rtp, __ocl_float_to_half_rtp) \
  _DECL_VSTORE_HALF_SPACE_ROUND(_SPACE, _rtn, __ocl_float_to_half_rtn) 

_DECL_VSTORE_HALF_SPACE(__global)
_DECL_VSTORE_HALF_SPACE(__local)
_DECL_VSTORE_HALF_SPACE(__private)

#undef _DECL_VSTORE_HALF_SPACE
#undef _DECL_VSTORE_HALF_SPACE_ROUND

/*-----------------------------------------------------------------------------
* Relational
*----------------------------------------------------------------------------*/
#define _INLN(__type) \
_CLC_OVERLOAD _CLC_INLINE __type bitselect(__type __a, __type __b, __type __c) { return __a^(__c&(__b^__a)); }

#define _DECL(__type) \
_CLC_OVERLOAD _CLC_DECL __type bitselect(__type __a, __type __b, __type __c);

_INLN(char)
_INLN(uchar)
_INLN(short)
_INLN(ushort)
_INLN(int)
_INLN(uint)
_INLN(long)
_INLN(ulong)

_DECL(char2)
_DECL(uchar2)
_INLN(short2)
_INLN(ushort2)
_INLN(int2)
_INLN(uint2)
_DECL(long2)
_DECL(ulong2)

_DECL(char3)
_DECL(uchar3)
_DECL(short3)
_DECL(ushort3)
_DECL(int3)
_DECL(uint3)
_DECL(long3)
_DECL(ulong3)

_INLN(char4)
_INLN(uchar4)
_INLN(short4)
_INLN(ushort4)
_DECL(int4)
_DECL(uint4)
_DECL(long4)
_DECL(ulong4)

_INLN(char8)
_INLN(uchar8)
_DECL(short8)
_DECL(ushort8)
_DECL(int8)
_DECL(uint8)
_DECL(long8)
_DECL(ulong8)

_DECL(char16)
_DECL(uchar16)
_DECL(short16)
_DECL(ushort16)
_DECL(int16)
_DECL(uint16)
_DECL(long16)
_DECL(ulong16)

_DECL(float)
_DECL(float2)
_DECL(float3)
_DECL(float4)
_DECL(float8)
_DECL(float16)

_DECL(double)
_DECL(double2)
_DECL(double3)
_DECL(double4)
_DECL(double8)
_DECL(double16)

#undef _INLN
#undef _DECL

#define _EXTU(__x,__l,__r)     (((__x) << __l) >> __r)

#define _SIGND(__x)        (as_uint2(__x).hi >> 31)
#define _EXPD(__x)         _EXTU(as_uint2(__x).hi, 1, 21)
#define _MANTD_HI(__x)     _EXTU(as_uint2(__x).hi, 12, 12)
#define _MANTD_LO(__x)     as_uint2(__x).lo
#define _MANTD_ZERO(__x)   (_MANTD_HI(__x) == 0 && _MANTD_LO(__x) == 0)
#define ANY_ZEROD(__x)    ((as_ulong(__x) << 1) == 0)
#define SUBNORMD(__x)     (_EXPD(__x) == 0 && !_MANTD_ZERO(__x))

#define _FABSF(__x)        ((as_uint(__x) << 1) >> 1)
#define _SIGNF(__x)        (as_uint(__x) >> 31)
#define _EXPF(__x)         ((as_uint(__x) << 1) >> 24)
#define _MANTF(__x)        ((as_uint(__x) << 9) >> 9)

#define isordered(__x,__y)          (!isnan(__x) & !isnan(__y))
#define isunordered(__x,__y)        (isnan(__x)  | isnan(__y))

_CLC_OVERLOAD _CLC_INLINE int  isnan(float  __x)   { return _FABSF(__x) > 0x7F800000; }
_UNARY_INLINE  (double, int,  isnan, __builtin_isnan)
_UNARY_VEC_DECL(float,  int,  isnan)
_UNARY_VEC_DECL(double, long, isnan)

_CLC_OVERLOAD _CLC_INLINE int  isfinite(float  __x)   { return _EXPF(__x) != 255; }
_UNARY_INLINE  (double, int,  isfinite, __builtin_isfinite)
_UNARY_VEC_DECL(float,  int,  isfinite)
_UNARY_VEC_DECL(double, long, isfinite)

_CLC_OVERLOAD _CLC_INLINE int  isinf(float  __x)   { return _FABSF(__x) == 0x7F800000; }
_UNARY_INLINE  (double, int,  isinf, __builtin_isinf)
_UNARY_VEC_DECL(float,  int,  isinf)
_UNARY_VEC_DECL(double, long, isinf)

_CLC_OVERLOAD _CLC_INLINE int  isnormal(float  __x)   { return _EXPF(__x) != 0 && _EXPF(__x) != 255; }
_UNARY_INLINE  (double, int,  isnormal, __builtin_isnormal)
_UNARY_VEC_DECL(float,  int,  isnormal)
_UNARY_VEC_DECL(double, long, isnormal)

_CLC_OVERLOAD _CLC_INLINE int  signbit(float  __x) { return _SIGNF(__x); }
_CLC_OVERLOAD _CLC_INLINE int  signbit(double __x) { return _SIGND(__x); }
_UNARY_VEC_DECL(float,  int,  signbit)
_UNARY_VEC_DECL(double, long, signbit)

_CLC_OVERLOAD _CLC_INLINE float copysign(float __x, float __y)
    { return as_float(_FABSF(__x) | (_SIGNF(__y) << 31)); }

_CLC_OVERLOAD _CLC_INLINE double copysign(double __x, double __y)
{ return as_double(((as_ulong(__x) << 1) >> 1) | ((as_ulong(__y) >> 63) << 63)); }

_BINARY_VEC_DECL(float,  float,  copysign)
_BINARY_VEC_DECL(double, double, copysign)

_CLC_OVERLOAD _CLC_INLINE int  isequal(float  __x, float __y)  { return __x == __y; }
_CLC_OVERLOAD _CLC_INLINE int  isequal(double __x, double __y) 
{ 
    if (_EXPD(__x) == 0 && _EXPD(__y) == 0)
    {
        if (_MANTD_ZERO(__x) && _MANTD_ZERO(__y)) return 1;
        return as_ulong(__x) == as_ulong(__y);
    }
    return __x == __y;

}
_BINARY_VEC_DECL(float,  int,  isequal)
_BINARY_VEC_DECL(double, long, isequal)

_CLC_OVERLOAD _CLC_INLINE int  isnotequal(float  __x, float __y)  { return __x != __y; }
_CLC_OVERLOAD _CLC_INLINE int  isnotequal(double __x, double __y) 
{ 
    if (_EXPD(__x) == 0 && _EXPD(__y) == 0)
    {
        if (_MANTD_ZERO(__x) && _MANTD_ZERO(__y)) return 0;
        return as_ulong(__x) != as_ulong(__y);
    }
    return __x != __y;

}
_BINARY_VEC_DECL(float,  int,  isnotequal)
_BINARY_VEC_DECL(double, long, isnotequal)

_CLC_OVERLOAD _CLC_INLINE int  isless(float  __x, float __y)  { return __x < __y; }
_CLC_OVERLOAD _CLC_INLINE int  isless(double __x, double __y) 
{ 
    if (SUBNORMD(__x) || SUBNORMD(__y))
    {
        if (isunordered(__x,__y))    return 0;
        if (_SIGND(__x) ^ _SIGND(__y)) return (_SIGND(__x)) ? 1 : 0;
        return _SIGND(__x) ? as_ulong(__x) > as_ulong(__y)
                        : as_ulong(__x) < as_ulong(__y);
    }
    return __x < __y;
}
_BINARY_VEC_DECL(float,  int,  isless)
_BINARY_VEC_DECL(double, long, isless)

_CLC_OVERLOAD _CLC_INLINE int  islessequal(float  __x, float __y)  { return __x <= __y; }
_CLC_OVERLOAD _CLC_INLINE int  islessequal(double __x, double __y) 
{ 
    if (SUBNORMD(__x) || SUBNORMD(__y))
    {
        if (isunordered(__x,__y))    return 0;
        if (_SIGND(__x) ^ _SIGND(__y)) return (_SIGND(__x)) ? 1 : 0;
        return _SIGND(__x) ? as_ulong(__x) >= as_ulong(__y)
                        : as_ulong(__x) <= as_ulong(__y);
    }
    return __x <= __y;
}
_BINARY_VEC_DECL(float,  int,  islessequal)
_BINARY_VEC_DECL(double, long, islessequal)

_CLC_OVERLOAD _CLC_INLINE int  isgreater(float  __x, float __y)  { return __x > __y; }
_CLC_OVERLOAD _CLC_INLINE int  isgreater(double __x, double __y) 
{ 
    if (SUBNORMD(__x) || SUBNORMD(__y))
    {
        if (isunordered(__x,__y))    return 0;
        if (_SIGND(__x) ^ _SIGND(__y)) return (_SIGND(__x)) ? 0 : 1;
        return _SIGND(__x) ? as_ulong(__x) < as_ulong(__y)
                        : as_ulong(__x) > as_ulong(__y);
    }
    return __x > __y;
}
_BINARY_VEC_DECL(float,  int,  isgreater)
_BINARY_VEC_DECL(double, long, isgreater)

_CLC_OVERLOAD _CLC_INLINE int  isgreaterequal(float  __x, float __y)  { return __x >= __y; }
_CLC_OVERLOAD _CLC_INLINE int  isgreaterequal(double __x, double __y) 
{ 
    if (SUBNORMD(__x) || SUBNORMD(__y))
    {
        if (isunordered(__x,__y))    return 0;
        if (_SIGND(__x) ^ _SIGND(__y)) return (_SIGND(__x)) ? 0 : 1;
        return _SIGND(__x) ? as_ulong(__x) <= as_ulong(__y)
                        : as_ulong(__x) >= as_ulong(__y);
    }
    return __x >= __y;
}
_BINARY_VEC_DECL(float,  int,  isgreaterequal)
_BINARY_VEC_DECL(double, long, isgreaterequal)

_CLC_OVERLOAD _CLC_INLINE int  islessgreater(float  __x, float __y)  
{ return isless(__x,__y) | isgreater(__x, __y); }
_CLC_OVERLOAD _CLC_INLINE int  islessgreater(double __x, double __y) 
{ return isless(__x,__y) | isgreater(__x, __y); }
_BINARY_VEC_DECL(float,  int,  islessgreater)
_BINARY_VEC_DECL(double, long, islessgreater)

#undef _EXPD
#undef _MANTD_HI
#undef _MANTD_LO
#undef _MANTD_ZERO
#undef _SIGND
#undef _FABSF
#undef _SIGNF
#undef _EXPF
#undef _MANTF
#undef _EXTU


#define _TEMPLATE(__type) \
_CLC_OVERLOAD _CLC_INLINE int any(__type __x)     { return __x < 0; } \
_CLC_OVERLOAD _CLC_INLINE int any(__type##2 __x)  { return (__x.s0 | __x.s1) < 0; } \
_CLC_OVERLOAD _CLC_DECL   int any(__type##3 __x); \
_CLC_OVERLOAD _CLC_DECL   int any(__type##4 __x); \
_CLC_OVERLOAD _CLC_DECL   int any(__type##8 __x); \
_CLC_OVERLOAD _CLC_DECL   int any(__type##16 __x); \

_TEMPLATE(char)
_TEMPLATE(short)
_TEMPLATE(int)
_TEMPLATE(long)

#undef _TEMPLATE

#define _TEMPLATE(__type) \
_CLC_OVERLOAD _CLC_INLINE int all(__type __x)     { return __x < 0; } \
_CLC_OVERLOAD _CLC_INLINE int all(__type##2 __x)  { return (__x.s0 & __x.s1) < 0; } \
_CLC_OVERLOAD _CLC_DECL   int all(__type##3 __x); \
_CLC_OVERLOAD _CLC_DECL   int all(__type##4 __x); \
_CLC_OVERLOAD _CLC_DECL   int all(__type##8 __x); \
_CLC_OVERLOAD _CLC_DECL   int all(__type##16 __x); \

_TEMPLATE(char)
_TEMPLATE(short)
_TEMPLATE(int)
_TEMPLATE(long)

#undef _TEMPLATE

#define _DEFINE(__type, __otype) \
_CLC_OVERLOAD _CLC_INLINE __type select(__type __a, __type __b, __otype __c) { return __c ? __b : __a; }

_DEFINE(char, char)
_DEFINE(char, uchar)
_DEFINE(uchar, char)
_DEFINE(uchar, uchar)
_DEFINE(short, short)
_DEFINE(short, ushort)
_DEFINE(ushort, short)
_DEFINE(ushort, ushort)
_DEFINE(int, int)
_DEFINE(int, uint)
_DEFINE(uint, int)
_DEFINE(uint, uint)
_DEFINE(long, long)
_DEFINE(long, ulong)
_DEFINE(ulong, long)
_DEFINE(ulong, ulong)
_DEFINE(float, int)
_DEFINE(float, uint)
_DEFINE(double, long)
_DEFINE(double, ulong)

#undef  _DEFINE

#define _DECLARATION(__type, __itype, __utype) \
_CLC_OVERLOAD _CLC_DECL __type select(__type __a, __type __b, __itype __c);\
_CLC_OVERLOAD _CLC_DECL __type select(__type __a, __type __b, __utype __c);

#define _SELECT_EXPAND_SIZES(__type,__itype,__utype) \
    _DECLARATION(_VEC_TYPE(__type,2), _VEC_TYPE(__itype,2), _VEC_TYPE(__utype,2))  \
    _DECLARATION(_VEC_TYPE(__type,3), _VEC_TYPE(__itype,3), _VEC_TYPE(__utype,3))  \
    _DECLARATION(_VEC_TYPE(__type,4), _VEC_TYPE(__itype,4), _VEC_TYPE(__utype,4))  \
    _DECLARATION(_VEC_TYPE(__type,8), _VEC_TYPE(__itype,8), _VEC_TYPE(__utype,8))  \
    _DECLARATION(_VEC_TYPE(__type,16), _VEC_TYPE(__itype,16), _VEC_TYPE(__utype,16))  \

#define _SELECT_EXPAND_TYPES   \
    _SELECT_EXPAND_SIZES(char, char, uchar)   \
    _SELECT_EXPAND_SIZES(uchar, char, uchar)  \
    _SELECT_EXPAND_SIZES(short, short, ushort)  \
    _SELECT_EXPAND_SIZES(ushort, short, ushort) \
    _SELECT_EXPAND_SIZES(int, int, uint)    \
    _SELECT_EXPAND_SIZES(uint, int, uint)   \
    _SELECT_EXPAND_SIZES(long, long, ulong)   \
    _SELECT_EXPAND_SIZES(ulong, long, ulong)  \
    _SELECT_EXPAND_SIZES(float, int, uint)  \
    _SELECT_EXPAND_SIZES(double, long, ulong)

_SELECT_EXPAND_TYPES

#undef _DECLARATION
#undef _SELECT_EXPAND_SIZES
#undef _SELECT_EXPAND_TYPES

/*-----------------------------------------------------------------------------
* Math
*----------------------------------------------------------------------------*/
#define CHAR_BIT        8
#define CHAR_MAX        SCHAR_MAX
#define CHAR_MIN        SCHAR_MIN
#define INT_MAX         2147483647
#define INT_MIN         (-2147483647 - 1)
#define LONG_MAX        0x7fffffffffffffffL
#define LONG_MIN        (-0x7fffffffffffffffL - 1)
#define SCHAR_MAX       127
#define SCHAR_MIN       (-127 - 1)
#define SHRT_MAX        32767
#define SHRT_MIN        (-32767 - 1)
#define UCHAR_MAX       255
#define USHRT_MAX       65535
#define UINT_MAX        0xffffffff
#define ULONG_MAX       0xffffffffffffffffUL

#define FLT_DIG         6
#define FLT_MANT_DIG    24
#define FLT_MAX_10_EXP  (+38)
#define FLT_MAX_EXP     (+128)
#define FLT_MIN_10_EXP  (-37)
#define FLT_MIN_EXP     (-125)
#define FLT_RADIX       2
#define FLT_MAX         0x1.fffffep127f
#define FLT_MIN         0x1.0p-126f
#define FLT_EPSILON     0x1.0p-23f

#define DBL_DIG         15
#define DBL_MANT_DIG    53
#define DBL_MAX_10_EXP  (+308)
#define DBL_MAX_EXP     (+1024)
#define DBL_MIN_10_EXP  (-307)
#define DBL_MIN_EXP     (-1021)
#define DBL_RADIX       2
#define DBL_MAX         0x1.fffffffffffffp1023
#define DBL_MIN         0x1.0p-1022
#define DBL_EPSILON     0x1.0p-52

#define M_E             2.7182818284590452354   /* e */
#define M_LOG2E         1.4426950408889634074   /* log_2 e */
#define M_LOG10E        0.43429448190325182765  /* log_10 e */
#define M_LN2           0.69314718055994530942  /* log_e 2 */
#define M_LN10          2.30258509299404568402  /* log_e 10 */
#define M_PI            3.14159265358979323846  /* pi */
#define M_PI_2          1.57079632679489661923  /* pi/2 */
#define M_PI_4          0.78539816339744830962  /* pi/4 */
#define M_1_PI          0.31830988618379067154  /* 1/pi */
#define M_2_PI          0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI      1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2         1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2       0.70710678118654752440  /* 1/sqrt(2) */

#define M_E_F           M_E
#define M_LOG2E_F       M_LOG2E
#define M_LOG10E_F      M_LOG10E
#define M_LN2_F         M_LN2
#define M_LN10_F        M_LN10
#define M_PI_F          M_PI
#define M_PI_2_F        M_PI_2
#define M_PI_4_F        M_PI_4
#define M_1_PI_F        M_1_PI
#define M_2_PI_F        M_2_PI
#define M_2_SQRTPI_F    M_2_SQRTPI
#define M_SQRT2_F       M_SQRT2
#define M_SQRT1_2_F     M_SQRT1_2

#define MAXFLOAT        FLT_MAX
#define HUGE_VALF       __builtin_huge_valf()
#define INFINITY        __builtin_inff()
#define NAN             as_float((int)0x7FC00000)

#define HUGE_VAL        __builtin_huge_val()

#define FP_ILOGB0       (-INT_MAX)
#define FP_ILOGBNAN     (INT_MAX)

#define _UNARY(__function) \
_UNARY_INLINE  (float,  float,  __function, __function##f)\
_UNARY_INLINE  (double, double, __function, __function##d)\
_UNARY_VEC_DECL(float,  float,  __function)\
_UNARY_VEC_DECL(double, double, __function)\

#define _UNARYT(__type1, __type2, __function,__op) \
_UNARY_INLINE  (__type1,  __type2,  __function, __op)\
_UNARY_VEC_DECL(__type1,  __type2,  __function)\

#define _BINARY(__function) \
_BINARY_INLINE  (float,  float,  __function, __function##f)\
_BINARY_INLINE  (double, double, __function, __function##d)\
_BINARY_VEC_DECL(float,  float,  __function)\
_BINARY_VEC_DECL(double, double, __function)\

/*-------------------------------------------------------------------------
* Prototypes for the math builtins 
*------------------------------------------------------------------------*/
_UNARY(acos)
_UNARY(acosh)

_CLC_PROTECTED float  acosf(float);
_CLC_PROTECTED double acosd(double);
_CLC_OVERLOAD _CLC_INLINE float  acospi(float __x)  { return acosf(__x) * M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double acospi(double __x) { return acosd(__x) * M_1_PI; }
_UNARY_VEC_DECL(float,  float,  acospi)
_UNARY_VEC_DECL(double, double, acospi)

_UNARY(asin)
_UNARY(asinh)

_CLC_PROTECTED float  asinf(float);
_CLC_PROTECTED double asind(double);
_CLC_OVERLOAD _CLC_INLINE float  asinpi(float __x)  { return asinf(__x) * M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double asinpi(double __x) { return asind(__x) * M_1_PI; }
_UNARY_VEC_DECL(float,  float,  asinpi)
_UNARY_VEC_DECL(double, double, asinpi)

_UNARY(atan)
_UNARY(atanh)

_CLC_PROTECTED float  atanf(float);
_CLC_PROTECTED double atand(double);
_CLC_OVERLOAD _CLC_INLINE float  atanpi(float __x)  { return atanf(__x) * M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double atanpi(double __x) { return atand(__x) * M_1_PI; }
_UNARY_VEC_DECL(float,  float,  atanpi)
_UNARY_VEC_DECL(double, double, atanpi)

_BINARY(atan2)

_CLC_OVERLOAD _CLC_INLINE float atan2pi(float __y, float __x)
    { return atan2f(__y,__x) * (float) M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double atan2pi(double __y, double __x)
    { return atan2d(__y,__x) *  M_1_PI; }
_BINARY_VEC_DECL(float,  float,  atan2pi)
_BINARY_VEC_DECL(double, double, atan2pi)

_UNARY(cbrt)
_UNARY(ceil)

_UNARY(cos)
_UNARY(cosh)
_UNARY(cospi)
_UNARY(erf)
_UNARY(erfc)
_UNARY(exp)
_UNARY(exp2)
_UNARY(exp10)
_UNARY(expm1)

_UNARY_INLINE  (float,  float,  fabs, _fabsf)\
_UNARY_INLINE  (double, double, fabs, _fabs)\
_UNARY_VEC_DECL(float,  float,  fabs)
_UNARY_VEC_DECL(double, double, fabs)

_BINARY(fdim)
_UNARY(floor)

_CLC_PROTECTED float  fmaf(float, float, float);
_CLC_PROTECTED double fmad(double, double, double);
_CLC_OVERLOAD _CLC_INLINE float  fma(float __a,  float __b,  float __c)  { return fmaf(__a,__b,__c); }
_CLC_OVERLOAD _CLC_INLINE double fma(double __a, double __b, double __c) { return fmad(__a,__b,__c); }
_TERNARY_VEC_DECL(float,  float,  fma)
_TERNARY_VEC_DECL(double, double, fma)

_BINARY(fmax)
_BINARY(fmin)
_BINARY(fmod)
_BINARY(hypot)

_UNARYT(float,  int, ilogb, ilogbf)
_UNARYT(double, int, ilogb, ilogbd)

_BINARY_INLINE_ALT  (float,  float,  int, ldexp, ldexpf)
_BINARY_INLINE_ALT  (double, double, int, ldexp, ldexpd)
_BINARY_VEC_DECL_ALT(float,  float,  int, ldexp)
_BINARY_VEC_DECL_ALT(double, double, int, ldexp)

_UNARY(lgamma)
_UNARY(log)
_UNARY(log2)
_UNARY(log10)
_UNARY(log1p)
_UNARY(logb)

_CLC_OVERLOAD _CLC_INLINE float  mad(float __a,  float __b,  float __c)  { return (__a*__b)+__c; }
_CLC_OVERLOAD _CLC_INLINE double mad(double __a, double __b, double __c) { return (__a*__b)+__c; }
_TERNARY_VEC_INLINE(float,  float,  mad, mad)
_TERNARY_VEC_INLINE(double, double, mad, mad)

_BINARY(maxmag)
_BINARY(minmag)

_CLC_OVERLOAD _CLC_INLINE float  nan(uint  __nancode)
    { return as_float(0x7FC00000 | __nancode); }
_CLC_OVERLOAD _CLC_INLINE double nan(ulong __nancode)
    { return as_double(0x7FF8000000000000ul | __nancode); }
_UNARY_VEC_DECL(uint,  float,  nan)
_UNARY_VEC_DECL(ulong, double, nan)

_BINARY(nextafter)
_BINARY(pow)

_BINARY_INLINE_ALT  (float,  float,  int, pown, pownf)
_BINARY_INLINE_ALT  (double, double, int, pown, pownd)
_BINARY_VEC_DECL_ALT(float,  float,  int, pown)
_BINARY_VEC_DECL_ALT(double, double, int, pown)

_BINARY(powr)

_BINARY(remainder)
_UNARY(rint)

_BINARY_INLINE_ALT  (float,  float,  int, rootn, rootnf)
_BINARY_INLINE_ALT  (double, double, int, rootn, rootnd)
_BINARY_VEC_DECL_ALT(float,  float,  int, rootn)
_BINARY_VEC_DECL_ALT(double, double, int, rootn)

_UNARY(round)
_UNARY(rsqrt)
_UNARY(sin)
_UNARY(sinh)
_UNARY(sinpi)
_UNARY(sqrt)
_UNARY(tan)
_UNARY(tanh)
_UNARY(trunc)
_UNARY(tanpi)
_UNARY(tgamma)

/*-----------------------------------------------------------------------------
* Half versions
*----------------------------------------------------------------------------*/
#define half_cos(__x)             cos(__x)
#define half_divide(__x,__y)        (__x/__y)
#define half_exp(__x)             exp(__x)
#define half_exp2(__x)            exp2(__x)
#define half_exp10(__x)           exp10(__x)
#define half_log(__x)             log(__x)
#define half_log2(__x)            log2(__x)
#define half_log10(__x)           log10(__x)
#define half_powr(__x,__y)          powr(__x,__y)
#define half_recip(__x)           reciprocal(__x)
#define half_rsqrt(__x)           rsqrt(__x)
#define half_sin(__x)             sin(__x)
#define half_sqrt(__x)            sqrt(__x)
#define half_tan(__x)             tan(__x)

/*-----------------------------------------------------------------------------
* Native versions
*----------------------------------------------------------------------------*/
#define native_sin(__x)           sin(__x)
#define native_cos(__x)           cos(__x)
#define native_powr(__x,__y)        powr(__x,__y)
#define native_exp(__x)           exp(__x)
#define native_exp2(__x)          exp2(__x)
#define native_exp10(__x)         exp10(__x)
#define native_log(__x)           log(__x)
#define native_log2(__x)          log2(__x)
#define native_log10(__x)         log10(__x)

_UNARY(reciprocal)
_UNARY(native_divide)
_UNARY(native_recip)
_UNARY(native_rsqrt)
_UNARY(native_sqrt)

#define native_tan(__x)           tan(__x)

#undef _UNARY
#undef _UNARYT
#undef _BINARY

_CLC_OVERLOAD _CLC_DECL float modf(float __x, __global  float * __iptr);
_CLC_OVERLOAD _CLC_DECL float modf(float __x, __local   float * __iptr);
_CLC_OVERLOAD _CLC_DECL float modf(float __x, __private float * __iptr);

_CLC_OVERLOAD _CLC_DECL float2 modf(float2 __x, __global  float2 * __iptr);
_CLC_OVERLOAD _CLC_DECL float2 modf(float2 __x, __local   float2 * __iptr);
_CLC_OVERLOAD _CLC_DECL float2 modf(float2 __x, __private float2 * __iptr);

_CLC_OVERLOAD _CLC_DECL float3 modf(float3 __x, __global  float3 * __iptr);
_CLC_OVERLOAD _CLC_DECL float3 modf(float3 __x, __local   float3 * __iptr);
_CLC_OVERLOAD _CLC_DECL float3 modf(float3 __x, __private float3 * __iptr);

_CLC_OVERLOAD _CLC_DECL float4 modf(float4 __x, __global  float4 * __iptr);
_CLC_OVERLOAD _CLC_DECL float4 modf(float4 __x, __local   float4 * __iptr);
_CLC_OVERLOAD _CLC_DECL float4 modf(float4 __x, __private float4 * __iptr);

_CLC_OVERLOAD _CLC_DECL float8 modf(float8 __x, __global  float8 * __iptr);
_CLC_OVERLOAD _CLC_DECL float8 modf(float8 __x, __local   float8 * __iptr);
_CLC_OVERLOAD _CLC_DECL float8 modf(float8 __x, __private float8 * __iptr);

_CLC_OVERLOAD _CLC_DECL float16 modf(float16 __x, __global  float16 * __iptr);
_CLC_OVERLOAD _CLC_DECL float16 modf(float16 __x, __local   float16 * __iptr);
_CLC_OVERLOAD _CLC_DECL float16 modf(float16 __x, __private float16 * __iptr);

_CLC_OVERLOAD _CLC_DECL double modf(double __x, __global  double * __iptr);
_CLC_OVERLOAD _CLC_DECL double modf(double __x, __local   double * __iptr);
_CLC_OVERLOAD _CLC_DECL double modf(double __x, __private double * __iptr);

_CLC_OVERLOAD _CLC_DECL double2 modf(double2 __x, __global  double2 * __iptr);
_CLC_OVERLOAD _CLC_DECL double2 modf(double2 __x, __local   double2 * __iptr);
_CLC_OVERLOAD _CLC_DECL double2 modf(double2 __x, __private double2 * __iptr);

_CLC_OVERLOAD _CLC_DECL double3 modf(double3 __x, __global  double3 * __iptr);
_CLC_OVERLOAD _CLC_DECL double3 modf(double3 __x, __local   double3 * __iptr);
_CLC_OVERLOAD _CLC_DECL double3 modf(double3 __x, __private double3 * __iptr);

_CLC_OVERLOAD _CLC_DECL double4 modf(double4 __x, __global  double4 * __iptr);
_CLC_OVERLOAD _CLC_DECL double4 modf(double4 __x, __local   double4 * __iptr);
_CLC_OVERLOAD _CLC_DECL double4 modf(double4 __x, __private double4 * __iptr);

_CLC_OVERLOAD _CLC_DECL double8 modf(double8 __x, __global  double8 * __iptr);
_CLC_OVERLOAD _CLC_DECL double8 modf(double8 __x, __local   double8 * __iptr);
_CLC_OVERLOAD _CLC_DECL double8 modf(double8 __x, __private double8 * __iptr);

_CLC_OVERLOAD _CLC_DECL double16 modf(double16 __x, __global  double16 * __iptr);
_CLC_OVERLOAD _CLC_DECL double16 modf(double16 __x, __local   double16 * __iptr);
_CLC_OVERLOAD _CLC_DECL double16 modf(double16 __x, __private double16 * __iptr);

_CLC_OVERLOAD _CLC_DECL float frexp(float __x, __global  int * __ptr);
_CLC_OVERLOAD _CLC_DECL float frexp(float __x, __local   int * __ptr);
_CLC_OVERLOAD _CLC_DECL float frexp(float __x, __private int * __ptr);

_CLC_OVERLOAD _CLC_DECL float2 frexp(float2 __x, __global  int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL float2 frexp(float2 __x, __local   int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL float2 frexp(float2 __x, __private int2 * __ptr);

_CLC_OVERLOAD _CLC_DECL float3 frexp(float3 __x, __global  int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL float3 frexp(float3 __x, __local   int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL float3 frexp(float3 __x, __private int3 * __ptr);

_CLC_OVERLOAD _CLC_DECL float4 frexp(float4 __x, __global  int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL float4 frexp(float4 __x, __local   int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL float4 frexp(float4 __x, __private int4 * __ptr);

_CLC_OVERLOAD _CLC_DECL float8 frexp(float8 __x, __global  int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL float8 frexp(float8 __x, __local   int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL float8 frexp(float8 __x, __private int8 * __ptr);

_CLC_OVERLOAD _CLC_DECL float16 frexp(float16 __x, __global  int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL float16 frexp(float16 __x, __local   int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL float16 frexp(float16 __x, __private int16 * __ptr);

_CLC_OVERLOAD _CLC_DECL double frexp(double __x, __global  int * __ptr);
_CLC_OVERLOAD _CLC_DECL double frexp(double __x, __local   int * __ptr);
_CLC_OVERLOAD _CLC_DECL double frexp(double __x, __private int * __ptr);

_CLC_OVERLOAD _CLC_DECL double2 frexp(double2 __x, __global  int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL double2 frexp(double2 __x, __local   int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL double2 frexp(double2 __x, __private int2 * __ptr);

_CLC_OVERLOAD _CLC_DECL double3 frexp(double3 __x, __global  int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL double3 frexp(double3 __x, __local   int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL double3 frexp(double3 __x, __private int3 * __ptr);

_CLC_OVERLOAD _CLC_DECL double4 frexp(double4 __x, __global  int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL double4 frexp(double4 __x, __local   int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL double4 frexp(double4 __x, __private int4 * __ptr);

_CLC_OVERLOAD _CLC_DECL double8 frexp(double8 __x, __global  int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL double8 frexp(double8 __x, __local   int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL double8 frexp(double8 __x, __private int8 * __ptr);

_CLC_OVERLOAD _CLC_DECL double16 frexp(double16 __x, __global  int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL double16 frexp(double16 __x, __local   int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL double16 frexp(double16 __x, __private int16 * __ptr);

_CLC_OVERLOAD _CLC_DECL float lgamma_r(float __x, __global  int * __ptr);
_CLC_OVERLOAD _CLC_DECL float lgamma_r(float __x, __local   int * __ptr);
_CLC_OVERLOAD _CLC_DECL float lgamma_r(float __x, __private int * __ptr);

_CLC_OVERLOAD _CLC_DECL float2 lgamma_r(float2 __x, __global  int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL float2 lgamma_r(float2 __x, __local   int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL float2 lgamma_r(float2 __x, __private int2 * __ptr);

_CLC_OVERLOAD _CLC_DECL float3 lgamma_r(float3 __x, __global  int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL float3 lgamma_r(float3 __x, __local   int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL float3 lgamma_r(float3 __x, __private int3 * __ptr);

_CLC_OVERLOAD _CLC_DECL float4 lgamma_r(float4 __x, __global  int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL float4 lgamma_r(float4 __x, __local   int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL float4 lgamma_r(float4 __x, __private int4 * __ptr);

_CLC_OVERLOAD _CLC_DECL float8 lgamma_r(float8 __x, __global  int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL float8 lgamma_r(float8 __x, __local   int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL float8 lgamma_r(float8 __x, __private int8 * __ptr);

_CLC_OVERLOAD _CLC_DECL float16 lgamma_r(float16 __x, __global  int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL float16 lgamma_r(float16 __x, __local   int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL float16 lgamma_r(float16 __x, __private int16 * __ptr);

_CLC_OVERLOAD _CLC_DECL double lgamma_r(double __x, __global  int * __ptr);
_CLC_OVERLOAD _CLC_DECL double lgamma_r(double __x, __local   int * __ptr);
_CLC_OVERLOAD _CLC_DECL double lgamma_r(double __x, __private int * __ptr);

_CLC_OVERLOAD _CLC_DECL double2 lgamma_r(double2 __x, __global  int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL double2 lgamma_r(double2 __x, __local   int2 * __ptr);
_CLC_OVERLOAD _CLC_DECL double2 lgamma_r(double2 __x, __private int2 * __ptr);

_CLC_OVERLOAD _CLC_DECL double3 lgamma_r(double3 __x, __global  int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL double3 lgamma_r(double3 __x, __local   int3 * __ptr);
_CLC_OVERLOAD _CLC_DECL double3 lgamma_r(double3 __x, __private int3 * __ptr);

_CLC_OVERLOAD _CLC_DECL double4 lgamma_r(double4 __x, __global  int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL double4 lgamma_r(double4 __x, __local   int4 * __ptr);
_CLC_OVERLOAD _CLC_DECL double4 lgamma_r(double4 __x, __private int4 * __ptr);

_CLC_OVERLOAD _CLC_DECL double8 lgamma_r(double8 __x, __global  int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL double8 lgamma_r(double8 __x, __local   int8 * __ptr);
_CLC_OVERLOAD _CLC_DECL double8 lgamma_r(double8 __x, __private int8 * __ptr);

_CLC_OVERLOAD _CLC_DECL double16 lgamma_r(double16 __x, __global  int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL double16 lgamma_r(double16 __x, __local   int16 * __ptr);
_CLC_OVERLOAD _CLC_DECL double16 lgamma_r(double16 __x, __private int16 * __ptr);


_CLC_OVERLOAD _CLC_DECL float fract(float __x, __global  float * __ptr);
_CLC_OVERLOAD _CLC_DECL float fract(float __x, __local   float * __ptr);
_CLC_OVERLOAD _CLC_DECL float fract(float __x, __private float * __ptr);

_CLC_OVERLOAD _CLC_DECL float2 fract(float2 __x, __global  float2 * __ptr);
_CLC_OVERLOAD _CLC_DECL float2 fract(float2 __x, __local   float2 * __ptr);
_CLC_OVERLOAD _CLC_DECL float2 fract(float2 __x, __private float2 * __ptr);

_CLC_OVERLOAD _CLC_DECL float3 fract(float3 __x, __global  float3 * __ptr);
_CLC_OVERLOAD _CLC_DECL float3 fract(float3 __x, __local   float3 * __ptr);
_CLC_OVERLOAD _CLC_DECL float3 fract(float3 __x, __private float3 * __ptr);

_CLC_OVERLOAD _CLC_DECL float4 fract(float4 __x, __global  float4 * __ptr);
_CLC_OVERLOAD _CLC_DECL float4 fract(float4 __x, __local   float4 * __ptr);
_CLC_OVERLOAD _CLC_DECL float4 fract(float4 __x, __private float4 * __ptr);

_CLC_OVERLOAD _CLC_DECL float8 fract(float8 __x, __global  float8 * __ptr);
_CLC_OVERLOAD _CLC_DECL float8 fract(float8 __x, __local   float8 * __ptr);
_CLC_OVERLOAD _CLC_DECL float8 fract(float8 __x, __private float8 * __ptr);

_CLC_OVERLOAD _CLC_DECL float16 fract(float16 __x, __global  float16 * __ptr);
_CLC_OVERLOAD _CLC_DECL float16 fract(float16 __x, __local   float16 * __ptr);
_CLC_OVERLOAD _CLC_DECL float16 fract(float16 __x, __private float16 * __ptr);

_CLC_OVERLOAD _CLC_DECL double fract(double __x, __global  double * __ptr);
_CLC_OVERLOAD _CLC_DECL double fract(double __x, __local   double * __ptr);
_CLC_OVERLOAD _CLC_DECL double fract(double __x, __private double * __ptr);

_CLC_OVERLOAD _CLC_DECL double2 fract(double2 __x, __global  double2 * __ptr);
_CLC_OVERLOAD _CLC_DECL double2 fract(double2 __x, __local   double2 * __ptr);
_CLC_OVERLOAD _CLC_DECL double2 fract(double2 __x, __private double2 * __ptr);

_CLC_OVERLOAD _CLC_DECL double3 fract(double3 __x, __global  double3 * __ptr);
_CLC_OVERLOAD _CLC_DECL double3 fract(double3 __x, __local   double3 * __ptr);
_CLC_OVERLOAD _CLC_DECL double3 fract(double3 __x, __private double3 * __ptr);

_CLC_OVERLOAD _CLC_DECL double4 fract(double4 __x, __global  double4 * __ptr);
_CLC_OVERLOAD _CLC_DECL double4 fract(double4 __x, __local   double4 * __ptr);
_CLC_OVERLOAD _CLC_DECL double4 fract(double4 __x, __private double4 * __ptr);

_CLC_OVERLOAD _CLC_DECL double8 fract(double8 __x, __global  double8 * __ptr);
_CLC_OVERLOAD _CLC_DECL double8 fract(double8 __x, __local   double8 * __ptr);
_CLC_OVERLOAD _CLC_DECL double8 fract(double8 __x, __private double8 * __ptr);

_CLC_OVERLOAD _CLC_DECL double16 fract(double16 __x, __global  double16 * __ptr);
_CLC_OVERLOAD _CLC_DECL double16 fract(double16 __x, __local   double16 * __ptr);
_CLC_OVERLOAD _CLC_DECL double16 fract(double16 __x, __private double16 * __ptr);

_CLC_PROTECTED float remquof(float __x, float __y, int *__ptr);

_CLC_OVERLOAD _CLC_INLINE float remquo(float __x, float __y, __global  int * __quo) 
    { return remquof(__x, __y, (int*)__quo); }

_CLC_OVERLOAD _CLC_INLINE float remquo(float __x, float __y, __local   int * __quo) 
    { return remquof(__x, __y, (int*)__quo); }

_CLC_OVERLOAD _CLC_INLINE float remquo(float __x, float __y, __private int * __quo) 
    { return remquof(__x, __y, (int*)__quo); }

_CLC_OVERLOAD _CLC_DECL float2 remquo(float2 __x, float2 __y, __global  int2 * __quo);
_CLC_OVERLOAD _CLC_DECL float2 remquo(float2 __x, float2 __y, __local   int2 * __quo);
_CLC_OVERLOAD _CLC_DECL float2 remquo(float2 __x, float2 __y, __private int2 * __quo);

_CLC_OVERLOAD _CLC_DECL float3 remquo(float3 __x, float3 __y, __global  int3 * __quo);
_CLC_OVERLOAD _CLC_DECL float3 remquo(float3 __x, float3 __y, __local   int3 * __quo);
_CLC_OVERLOAD _CLC_DECL float3 remquo(float3 __x, float3 __y, __private int3 * __quo);

_CLC_OVERLOAD _CLC_DECL float4 remquo(float4 __x, float4 __y, __global  int4 * __quo);
_CLC_OVERLOAD _CLC_DECL float4 remquo(float4 __x, float4 __y, __local   int4 * __quo);
_CLC_OVERLOAD _CLC_DECL float4 remquo(float4 __x, float4 __y, __private int4 * __quo);

_CLC_OVERLOAD _CLC_DECL float8 remquo(float8 __x, float8 __y, __global  int8 * __quo);
_CLC_OVERLOAD _CLC_DECL float8 remquo(float8 __x, float8 __y, __local   int8 * __quo);
_CLC_OVERLOAD _CLC_DECL float8 remquo(float8 __x, float8 __y, __private int8 * __quo);

_CLC_OVERLOAD _CLC_DECL float16 remquo(float16 __x, float16 __y, __global  int16 * __quo);
_CLC_OVERLOAD _CLC_DECL float16 remquo(float16 __x, float16 __y, __local   int16 * __quo);
_CLC_OVERLOAD _CLC_DECL float16 remquo(float16 __x, float16 __y, __private int16 * __quo);

_CLC_PROTECTED double remquod(double __x, double __y, int *__ptr);

_CLC_OVERLOAD _CLC_INLINE double remquo(double __x, double __y, __global  int * __quo) 
    { return remquod(__x, __y, (int*)__quo); }

_CLC_OVERLOAD _CLC_INLINE double remquo(double __x, double __y, __local   int * __quo) 
    { return remquod(__x, __y, (int*)__quo); }

_CLC_OVERLOAD _CLC_INLINE double remquo(double __x, double __y, __private int * __quo) 
    { return remquod(__x, __y, (int*)__quo); }

_CLC_OVERLOAD _CLC_DECL double2 remquo(double2 __x, double2 __y, __global  int2 * __quo);
_CLC_OVERLOAD _CLC_DECL double2 remquo(double2 __x, double2 __y, __local   int2 * __quo);
_CLC_OVERLOAD _CLC_DECL double2 remquo(double2 __x, double2 __y, __private int2 * __quo);

_CLC_OVERLOAD _CLC_DECL double3 remquo(double3 __x, double3 __y, __global  int3 * __quo);
_CLC_OVERLOAD _CLC_DECL double3 remquo(double3 __x, double3 __y, __local   int3 * __quo);
_CLC_OVERLOAD _CLC_DECL double3 remquo(double3 __x, double3 __y, __private int3 * __quo);

_CLC_OVERLOAD _CLC_DECL double4 remquo(double4 __x, double4 __y, __global  int4 * __quo);
_CLC_OVERLOAD _CLC_DECL double4 remquo(double4 __x, double4 __y, __local   int4 * __quo);
_CLC_OVERLOAD _CLC_DECL double4 remquo(double4 __x, double4 __y, __private int4 * __quo);

_CLC_OVERLOAD _CLC_DECL double8 remquo(double8 __x, double8 __y, __global  int8 * __quo);
_CLC_OVERLOAD _CLC_DECL double8 remquo(double8 __x, double8 __y, __local   int8 * __quo);
_CLC_OVERLOAD _CLC_DECL double8 remquo(double8 __x, double8 __y, __private int8 * __quo);

_CLC_OVERLOAD _CLC_DECL double16 remquo(double16 __x, double16 __y, __global  int16 * __quo);
_CLC_OVERLOAD _CLC_DECL double16 remquo(double16 __x, double16 __y, __local   int16 * __quo);
_CLC_OVERLOAD _CLC_DECL double16 remquo(double16 __x, double16 __y, __private int16 * __quo);

_CLC_PROTECTED void sincosf(float __x, float * __sinval, float * __cosval);

_CLC_OVERLOAD _CLC_INLINE float sincos(float __x, __global  float * __cosval) 
    {   float __sinval; 
        sincosf(__x, (float*)&__sinval, (float*)__cosval); 
        return __sinval;
    }

_CLC_OVERLOAD _CLC_INLINE float sincos(float __x, __local   float * __cosval) 
    {   float __sinval; 
        sincosf(__x, (float*)&__sinval, (float*)__cosval); 
        return __sinval;
    }

_CLC_OVERLOAD _CLC_INLINE float sincos(float __x, __private float * __cosval) 
    {   float __sinval; 
        sincosf(__x, (float*)&__sinval, (float*)__cosval); 
        return __sinval;
    }

_CLC_OVERLOAD _CLC_DECL float2 sincos(float2 __x, __global  float2 * __cosval);
_CLC_OVERLOAD _CLC_DECL float2 sincos(float2 __x, __local   float2 * __cosval);
_CLC_OVERLOAD _CLC_DECL float2 sincos(float2 __x, __private float2 * __cosval);

_CLC_OVERLOAD _CLC_DECL float3 sincos(float3 __x, __global  float3 * __cosval);
_CLC_OVERLOAD _CLC_DECL float3 sincos(float3 __x, __local   float3 * __cosval);
_CLC_OVERLOAD _CLC_DECL float3 sincos(float3 __x, __private float3 * __cosval);

_CLC_OVERLOAD _CLC_DECL float4 sincos(float4 __x, __global  float4 * __cosval);
_CLC_OVERLOAD _CLC_DECL float4 sincos(float4 __x, __local   float4 * __cosval);
_CLC_OVERLOAD _CLC_DECL float4 sincos(float4 __x, __private float4 * __cosval);

_CLC_OVERLOAD _CLC_DECL float8 sincos(float8 __x, __global  float8 * __cosval);
_CLC_OVERLOAD _CLC_DECL float8 sincos(float8 __x, __local   float8 * __cosval);
_CLC_OVERLOAD _CLC_DECL float8 sincos(float8 __x, __private float8 * __cosval);

_CLC_OVERLOAD _CLC_DECL float16 sincos(float16 __x, __global  float16 * __cosval);
_CLC_OVERLOAD _CLC_DECL float16 sincos(float16 __x, __local   float16 * __cosval);
_CLC_OVERLOAD _CLC_DECL float16 sincos(float16 __x, __private float16 * __cosval);

_CLC_PROTECTED void sincosd(double __x, double * __sinval, double * __cosval);

_CLC_OVERLOAD _CLC_INLINE double sincos(double __x, __global  double * __cosval) 
    {   double __sinval; 
        sincosd(__x, (double*)&__sinval, (double*)__cosval); 
        return __sinval;
    }

_CLC_OVERLOAD _CLC_INLINE double sincos(double __x, __local   double * __cosval) 
    {   double __sinval; 
        sincosd(__x, (double*)&__sinval, (double*)__cosval); 
        return __sinval;
    }

_CLC_OVERLOAD _CLC_INLINE double sincos(double __x, __private double * __cosval) 
    {   double __sinval; 
        sincosd(__x, (double*)&__sinval, (double*)__cosval); 
        return __sinval;
    }

_CLC_OVERLOAD _CLC_DECL double2 sincos(double2 __x, __global  double2 * __cosval);
_CLC_OVERLOAD _CLC_DECL double2 sincos(double2 __x, __local   double2 * __cosval);
_CLC_OVERLOAD _CLC_DECL double2 sincos(double2 __x, __private double2 * __cosval);

_CLC_OVERLOAD _CLC_DECL double3 sincos(double3 __x, __global  double3 * __cosval);
_CLC_OVERLOAD _CLC_DECL double3 sincos(double3 __x, __local   double3 * __cosval);
_CLC_OVERLOAD _CLC_DECL double3 sincos(double3 __x, __private double3 * __cosval);

_CLC_OVERLOAD _CLC_DECL double4 sincos(double4 __x, __global  double4 * __cosval);
_CLC_OVERLOAD _CLC_DECL double4 sincos(double4 __x, __local   double4 * __cosval);
_CLC_OVERLOAD _CLC_DECL double4 sincos(double4 __x, __private double4 * __cosval);

_CLC_OVERLOAD _CLC_DECL double8 sincos(double8 __x, __global  double8 * __cosval);
_CLC_OVERLOAD _CLC_DECL double8 sincos(double8 __x, __local   double8 * __cosval);
_CLC_OVERLOAD _CLC_DECL double8 sincos(double8 __x, __private double8 * __cosval);

_CLC_OVERLOAD _CLC_DECL double16 sincos(double16 __x, __global  double16 * __cosval);
_CLC_OVERLOAD _CLC_DECL double16 sincos(double16 __x, __local   double16 * __cosval);
_CLC_OVERLOAD _CLC_DECL double16 sincos(double16 __x, __private double16 * __cosval);

/*-----------------------------------------------------------------------------
* Integer
*----------------------------------------------------------------------------*/
#define _EXPAND_SIZES(__type) \
    _SCALAR(__type)              \
    _TEMPLATE(_VEC_TYPE(__type,2))  \
    _TEMPLATE(_VEC_TYPE(__type,3))  \
    _TEMPLATE(_VEC_TYPE(__type,4))  \
    _TEMPLATE(_VEC_TYPE(__type,8))  \
    _TEMPLATE(_VEC_TYPE(__type,16)) \

#define _TEMPLATE(__gentype) \
    _CLC_OVERLOAD _CLC_DECL __gentype hadd(__gentype __x1, __gentype __x2);

#define _SCALAR(__gentype) \
    _CLC_OVERLOAD _CLC_INLINE __gentype hadd(__gentype __x, __gentype __y) \
    { return (__x >> 1) + (__y >> 1) + (__x & __y & 1); } 

_EXPAND_INTEGER_TYPES()

#undef _SCALAR
#undef _TEMPLATE

#define _TEMPLATE(__gentype) \
    _CLC_OVERLOAD _CLC_DECL __gentype rhadd(__gentype __x1, __gentype __x2);\

#define _SCALAR(__gentype) \
    _CLC_OVERLOAD _CLC_INLINE __gentype rhadd(__gentype __x, __gentype __y) \
    { return (__x >> 1) + (__y >> 1) + ((__x&1)|(__y&1)); } \

_EXPAND_SIZES(char)   
// _EXPAND_SIZES(uchar)  uchar inlined in dsp.h
// _EXPAND_SIZES(short)  short inlined in dsp.h
_EXPAND_SIZES(ushort) 
_EXPAND_SIZES(int)    
_EXPAND_SIZES(uint)   
_EXPAND_SIZES(long)   
_EXPAND_SIZES(ulong)  

#undef _EXPAND_SIZES
#undef _SCALAR
#undef _TEMPLATE

#define _EXPAND_SIZES(__type)             \
    _SCALAR_IMPLEMENTATION(__type)              \
    _DECLARATION(_VEC_TYPE(__type,2), __type)  \
    _DECLARATION(_VEC_TYPE(__type,3), __type)  \
    _DECLARATION(_VEC_TYPE(__type,4), __type)  \
    _DECLARATION(_VEC_TYPE(__type,8), __type)  \
    _DECLARATION(_VEC_TYPE(__type,16), __type) \

#define _DECLARATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_DECL __gentype clamp(__gentype __x, __gentype __minval, __gentype __maxval);  \
_CLC_OVERLOAD _CLC_DECL __gentype clamp(__gentype __x, __sgentype __minval, __sgentype __maxval); \

#define _SCALAR_IMPLEMENTATION(__gentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype clamp(__gentype __x, __gentype __minval, __gentype __maxval)  \
    { return __x > __maxval ? __maxval : __x < __minval ? __minval : __x; } \

_EXPAND_TYPES()

#undef _EXPAND_SIZES
#undef _IMPLEMENTATION
#undef _DECLARATION
#undef _SCALAR_IMPLEMENTATION

#define _EXPAND_SIZES(__type)                  \
    _SCALAR_IMPLEMENTATION(__type)             \
    _IMPLEMENTATION(_VEC_TYPE(__type,2), __type)  \
    _DECLARATION(_VEC_TYPE(__type,3), __type)  \
    _DECLARATION(_VEC_TYPE(__type,4), __type)  \
    _DECLARATION(_VEC_TYPE(__type,8), __type)  \
    _DECLARATION(_VEC_TYPE(__type,16), __type) \

#define _DECLARATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_DECL __gentype min(__gentype __x, __gentype __y); \
_CLC_OVERLOAD _CLC_DECL __gentype min(__gentype __x, __sgentype __y); \
_CLC_OVERLOAD _CLC_DECL __gentype max(__gentype __x, __gentype __y); \
_CLC_OVERLOAD _CLC_DECL __gentype max(__gentype __x, __sgentype __y); \

#define _IMPLEMENTATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype min(__gentype __x, __gentype __y)  \
    { return __y < __x ? __y : __x; } \
_CLC_OVERLOAD _CLC_INLINE __gentype min(__gentype __x, __sgentype __y) \
    { return (__gentype)__y < __x ? (__gentype)__y : __x; } \
_CLC_OVERLOAD _CLC_INLINE __gentype max(__gentype __x, __gentype __y)  \
    { return __y > __x ? __y : __x; } \
_CLC_OVERLOAD _CLC_INLINE __gentype max(__gentype __x, __sgentype __y) \
    { return (__gentype)__y > __x ? (__gentype)__y : __x; } \

#define _SCALAR_IMPLEMENTATION(__gentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype min(__gentype __x, __gentype __y)  \
    { return __y < __x ? __y : __x; } \
_CLC_OVERLOAD _CLC_INLINE __gentype max(__gentype __x, __gentype __y)  \
    { return __y > __x ? __y : __x; } \

_EXPAND_TYPES()

#undef _EXPAND_SIZES
#undef _DECLARATION
#undef _IMPLEMENTATION
#undef _SCALAR_IMPLEMENTATION

#define _EXPAND_SIZES(__type)                  \
    _SCALAR_IMPLEMENTATION(__type)             \
    _IMPLEMENTATION(_VEC_TYPE(__type,2), __type)  \
    _DECLARATION(_VEC_TYPE(__type,3), __type)  \
    _DECLARATION(_VEC_TYPE(__type,4), __type)  \
    _DECLARATION(_VEC_TYPE(__type,8), __type)  \
    _DECLARATION(_VEC_TYPE(__type,16), __type) \

#define _DECLARATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_DECL __gentype mix(__gentype __x, __gentype __y, __gentype __a); \
_CLC_OVERLOAD _CLC_DECL __gentype mix(__gentype __x, __gentype __y, __sgentype __a); \

#define _IMPLEMENTATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype mix(__gentype __x, __gentype __y, __gentype __a)  \
    { return __x + (__y-__x) * __a; } \
_CLC_OVERLOAD _CLC_INLINE __gentype mix(__gentype __x, __gentype __y, __sgentype __a) \
    { return __x + (__y-__x) * (__gentype)__a; } \

#define _SCALAR_IMPLEMENTATION(__gentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype mix(__gentype __x, __gentype __y, __gentype __a)  \
    { return __x + (__y-__x) * __a; } \

_EXPAND_SIZES(float)
_EXPAND_SIZES(double)

#undef _EXPAND_SIZES
#undef _DECLARATION
#undef _IMPLEMENTATION
#undef _SCALAR_IMPLEMENTATION

#define _EXPAND_SIZES(__type, __utype) \
    _TEMPLATE(_VEC_TYPE(__type,2), _VEC_TYPE(__utype,2))  \
    _TEMPLATE(_VEC_TYPE(__type,3), _VEC_TYPE(__utype,3))  \
    _TEMPLATE(_VEC_TYPE(__type,4), _VEC_TYPE(__utype,4))  \
    _TEMPLATE(_VEC_TYPE(__type,8), _VEC_TYPE(__utype,8))  \
    _TEMPLATE(_VEC_TYPE(__type,16), _VEC_TYPE(__utype,16)) \

#define _TEMPLATE(__gentype, __ugentype) \
    _CLC_OVERLOAD _CLC_DECL __ugentype abs_diff(__gentype __x, __gentype __y);\

_EXPAND_SIZES(char, uchar)
_EXPAND_SIZES(uchar, uchar)
_EXPAND_SIZES(short, ushort)
_EXPAND_SIZES(ushort, ushort)
_EXPAND_SIZES(int, uint)
_EXPAND_SIZES(uint, uint)
_EXPAND_SIZES(long, ulong)
_EXPAND_SIZES(ulong, ulong)

_CLC_OVERLOAD _CLC_INLINE uchar  abs_diff (char __x,   char __y)   { return __x>__y ? __x-__y : __y-__x; }
_CLC_OVERLOAD _CLC_INLINE uchar  abs_diff (uchar __x,  uchar __y)  { return __x>__y ? __x-__y : __y-__x; }
_CLC_OVERLOAD _CLC_INLINE ushort abs_diff (short __x,  short __y)  { return __x>__y ? __x-__y : __y-__x; }
_CLC_OVERLOAD _CLC_INLINE ushort abs_diff (ushort __x, ushort __y) { return __x>__y ? __x-__y : __y-__x; }
_CLC_OVERLOAD _CLC_INLINE uint   abs_diff (uint __x,   uint __y)   { return __x>__y ? __x-__y : __y-__x; }
_CLC_OVERLOAD _CLC_INLINE ulong  abs_diff (ulong __x,  ulong __y)  { return __x>__y ? __x-__y : __y-__x; }

_CLC_OVERLOAD _CLC_DECL uint abs_diff(int __x,  int __y);
_CLC_OVERLOAD _CLC_DECL ulong abs_diff(long __x,  long __y);

#undef _EXPAND_SIZES
#undef _TEMPLATE

#define mad_hi(__a, __b, __c) (mul_hi((__a),(__b))+(__c))
#define mul24(__a, __b)     ((__a)*(__b))
#define mad24(__a, __b, __c)  (((__a)*(__b))+(__c))

/*-----------------------------------------------------------------------------
* Common
*----------------------------------------------------------------------------*/
#define _EXPAND_SIZES(__type) \
    _IMPLEMENTATION(__type)              \
    _IMPLEMENTATION(_VEC_TYPE(__type,2))  \
    _DECLARATION(_VEC_TYPE(__type,3))  \
    _DECLARATION(_VEC_TYPE(__type,4))  \
    _DECLARATION(_VEC_TYPE(__type,8))  \
    _DECLARATION(_VEC_TYPE(__type,16)) \

#define _DECLARATION(__gentype) \
_CLC_OVERLOAD _CLC_DECL __gentype degrees(__gentype __radians); \
_CLC_OVERLOAD _CLC_DECL __gentype radians(__gentype __degrees); \

#define _IMPLEMENTATION(__gentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype degrees(__gentype __radians) { return __radians * (__gentype)180.0 * (__gentype)M_1_PI; } \
_CLC_OVERLOAD _CLC_INLINE __gentype radians(__gentype __degrees) { return __degrees * (__gentype)M_PI / (__gentype)180.0; } 

_EXPAND_SIZES(float)
_EXPAND_SIZES(double)

#undef _EXPAND_SIZES
#undef _DECLARATION
#undef _IMPLEMENTATION

#define _EXPAND_SIZES(__type)                  \
    _SCALAR_IMPLEMENTATION(__type)             \
    _IMPLEMENTATION(_VEC_TYPE(__type,2), __type)  \
    _DECLARATION(_VEC_TYPE(__type,3), __type)  \
    _DECLARATION(_VEC_TYPE(__type,4), __type)  \
    _DECLARATION(_VEC_TYPE(__type,8), __type)  \
    _DECLARATION(_VEC_TYPE(__type,16), __type) \

#define _DECLARATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_DECL __gentype step(__gentype __edge, __gentype __x); \
_CLC_OVERLOAD _CLC_DECL __gentype step(__sgentype __edge, __gentype __x); \

#define _IMPLEMENTATION(__gentype, __sgentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype step(__gentype __edge, __gentype __x)  \
    { return __x < __edge ? (__gentype)0.0 : (__gentype)1.0 ; } \
_CLC_OVERLOAD _CLC_INLINE __gentype step(__sgentype __edge, __gentype __x) \
    { return __x < (__gentype)__edge ? (__gentype)0.0 : (__gentype)1.0 ; } \

#define _SCALAR_IMPLEMENTATION(__gentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype step(__gentype __edge, __gentype __x)  \
    { return __x < __edge ? 0.0 : 1.0 ; } \

_EXPAND_SIZES(float)
_EXPAND_SIZES(double)

#undef _EXPAND_SIZES
#undef _DECLARATION
#undef _IMPLEMENTATION
#undef _SCALAR_IMPLEMENTATION

_CLC_OVERLOAD _CLC_DECL float   smoothstep(float __edge0, float __edge1, float __x);
_CLC_OVERLOAD _CLC_DECL float2  smoothstep(float2  __edge0, float2  __edge1,  
                                           float2  __x);
_CLC_OVERLOAD _CLC_DECL float3  smoothstep(float3  __edge0, float3  __edge1,  
                                           float3  __x);
_CLC_OVERLOAD _CLC_DECL float4  smoothstep(float4  __edge0, float4  __edge1,  
                                           float4  __x);
_CLC_OVERLOAD _CLC_DECL float8  smoothstep(float8  __edge0, float8  __edge1,  
                                           float8  __x);
_CLC_OVERLOAD _CLC_DECL float16 smoothstep(float16 __edge0, float16 __edge1,  
                                           float16 __x);

_CLC_OVERLOAD _CLC_DECL float2  smoothstep(float __edge0, float __edge1, float2  __x);
_CLC_OVERLOAD _CLC_DECL float3  smoothstep(float __edge0, float __edge1, float3  __x);
_CLC_OVERLOAD _CLC_DECL float4  smoothstep(float __edge0, float __edge1, float4  __x);
_CLC_OVERLOAD _CLC_DECL float8  smoothstep(float __edge0, float __edge1, float8  __x);
_CLC_OVERLOAD _CLC_DECL float16 smoothstep(float __edge0, float __edge1, float16 __x);

_CLC_OVERLOAD _CLC_DECL double   smoothstep(double __edge0, double __edge1, double __x);
_CLC_OVERLOAD _CLC_DECL double2  smoothstep(double2  __edge0, double2  __edge1,  
                                            double2  __x);
_CLC_OVERLOAD _CLC_DECL double3  smoothstep(double3  __edge0, double3  __edge1,  
                                            double3  __x);
_CLC_OVERLOAD _CLC_DECL double4  smoothstep(double4  __edge0, double4  __edge1,  
                                            double4  __x);
_CLC_OVERLOAD _CLC_DECL double8  smoothstep(double8  __edge0, double8  __edge1,  
                                            double8  __x);
_CLC_OVERLOAD _CLC_DECL double16 smoothstep(double16 __edge0, double16 __edge1,  
                                            double16 __x);

_CLC_OVERLOAD _CLC_DECL double2  smoothstep(double __edge0, double __edge1, 
                                            double2  __x);
_CLC_OVERLOAD _CLC_DECL double3  smoothstep(double __edge0, double __edge1, 
                                            double3  __x);
_CLC_OVERLOAD _CLC_DECL double4  smoothstep(double __edge0, double __edge1, 
                                            double4  __x);
_CLC_OVERLOAD _CLC_DECL double8  smoothstep(double __edge0, double __edge1, 
                                            double8  __x);
_CLC_OVERLOAD _CLC_DECL double16 smoothstep(double __edge0, double __edge1, 
                                            double16 __x);

#define _EXPAND_SIZES(__type)            \
    _IMPLEMENTATION(__type)              \
    _IMPLEMENTATION(_VEC_TYPE(__type,2)) \
    _DECLARATION(_VEC_TYPE(__type,3))  \
    _DECLARATION(_VEC_TYPE(__type,4))  \
    _DECLARATION(_VEC_TYPE(__type,8))  \
    _DECLARATION(_VEC_TYPE(__type,16)) \

#define _DECLARATION(__gentype) \
_CLC_OVERLOAD _CLC_DECL __gentype sign(__gentype __x); \

#define _IMPLEMENTATION(__gentype) \
_CLC_OVERLOAD _CLC_INLINE __gentype sign(__gentype __x) \
{  return __x > (__gentype)0.0 ? (__gentype) 1.0 : \
          __x < (__gentype)0.0 ? (__gentype)-1.0 : \
          isnan(__x)         ? (__gentype) 0.0 : __x; } \

_EXPAND_SIZES(float)
_EXPAND_SIZES(double)

#undef _EXPAND_SIZES
#undef _DECLARATION
#undef _IMPLEMENTATION

/*-----------------------------------------------------------------------------
* Geometric
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_INLINE float  dot(float __p0, float __p1)   {return __p0*__p1;}
_CLC_OVERLOAD _CLC_INLINE float  dot(float2 __p0, float2 __p1) {return __p0.x*__p1.x+__p0.y*__p1.y;}
_CLC_OVERLOAD _CLC_INLINE float  dot(float3 __p0, float3 __p1) { return __p0.x*__p1.x + __p0.y*__p1.y + __p0.z*__p1.z; }
_CLC_OVERLOAD _CLC_INLINE float  dot(float4 __p0, float4 __p1) { return __p0.x*__p1.x + __p0.y*__p1.y + __p0.z*__p1.z + __p0.w*__p1.w; }
_CLC_OVERLOAD _CLC_INLINE double dot(double __p0, double __p1)   {return __p0*__p1;}
_CLC_OVERLOAD _CLC_INLINE double dot(double2 __p0, double2 __p1) {return __p0.x*__p1.x+__p0.y*__p1.y;}
_CLC_OVERLOAD _CLC_INLINE double dot(double3 __p0, double3 __p1) { return __p0.x*__p1.x + __p0.y*__p1.y + __p0.z*__p1.z; }
_CLC_OVERLOAD _CLC_INLINE double dot(double4 __p0, double4 __p1) { return __p0.x*__p1.x + __p0.y*__p1.y + __p0.z*__p1.z + __p0.w*__p1.w; }

_CLC_OVERLOAD _CLC_DECL float3  cross(float3 __p0, float3 __p1);
_CLC_OVERLOAD _CLC_DECL float4  cross(float4 __p0, float4 __p1);
_CLC_OVERLOAD _CLC_DECL double3 cross(double3 __p0, double3 __p1);
_CLC_OVERLOAD _CLC_DECL double4 cross(double4 __p0, double4 __p1);

_CLC_OVERLOAD _CLC_INLINE float  length(float __p)         {return fabs(__p);}
_CLC_OVERLOAD _CLC_INLINE double length(double __p)        {return fabs(__p);}
_CLC_OVERLOAD _CLC_INLINE float  fast_length(float __p)    {return fabs(__p);}
_CLC_OVERLOAD _CLC_INLINE double fast_length(double __p)   {return fabs(__p);}

_CLC_OVERLOAD _CLC_DECL   float  length(float2 __p);
_CLC_OVERLOAD _CLC_DECL   float  length(float3 __p);
_CLC_OVERLOAD _CLC_DECL   float  length(float4 __p);
_CLC_OVERLOAD _CLC_DECL   double length(double2 __p);
_CLC_OVERLOAD _CLC_DECL   double length(double3 __p);
_CLC_OVERLOAD _CLC_DECL   double length(double4 __p);

_CLC_OVERLOAD _CLC_DECL   float  fast_length(float2 __p);
_CLC_OVERLOAD _CLC_DECL   float  fast_length(float3 __p);
_CLC_OVERLOAD _CLC_DECL   float  fast_length(float4 __p);
_CLC_OVERLOAD _CLC_DECL   double fast_length(double2 __p);
_CLC_OVERLOAD _CLC_DECL   double fast_length(double3 __p);
_CLC_OVERLOAD _CLC_DECL   double fast_length(double4 __p);

_CLC_OVERLOAD _CLC_INLINE float  distance(float __p0,   float __p1)    { return fabs(__p1-__p0);}
_CLC_OVERLOAD _CLC_INLINE float  distance(float2 __p0,  float2 __p1)   { return length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE float  distance(float3 __p0,  float3 __p1)   { return length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE float  distance(float4 __p0,  float4 __p1)   { return length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE double distance(double __p0,  double __p1)   { return fabs(__p1-__p0);}
_CLC_OVERLOAD _CLC_INLINE double distance(double2 __p0, double2 __p1)  { return length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE double distance(double3 __p0, double3 __p1)  { return length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE double distance(double4 __p0, double4 __p1)  { return length(__p1-__p0); }

_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float __p0,   float __p1)    { return fabs(__p1-__p0);}
_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float2 __p0,  float2 __p1)   { return fast_length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float3 __p0,  float3 __p1)   { return fast_length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float4 __p0,  float4 __p1)   { return fast_length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double __p0,  double __p1)   { return fabs(__p1-__p0);}
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double2 __p0, double2 __p1)  { return fast_length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double3 __p0, double3 __p1)  { return fast_length(__p1-__p0); }
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double4 __p0, double4 __p1)  { return fast_length(__p1-__p0); }

_CLC_OVERLOAD _CLC_INLINE float  normalize(float __p)        
{return __p > 0.0f ? 1.0f : __p < 0.0f ? -1.0f : 0.0f;}

_CLC_OVERLOAD _CLC_INLINE double normalize(double __p)       
{return __p > 0.0 ? 1.0 : __p < 0.0 ? -1.0 : 0.0;}

_CLC_OVERLOAD _CLC_INLINE float  fast_normalize(float __p)   
{return __p > 0.0f ? 1.0f : __p < 0.0f ? -1.0f : 0.0f;}

_CLC_OVERLOAD _CLC_INLINE double fast_normalize(double __p)  
{return __p > 0.0 ? 1.0 : __p < 0.0 ? -1.0 : 0.0;}

_CLC_OVERLOAD _CLC_INLINE float2  normalize(float2 __p)        { return __p / length(__p); }
_CLC_OVERLOAD _CLC_INLINE float3  normalize(float3 __p)        { return __p / length(__p); }
_CLC_OVERLOAD _CLC_INLINE float4  normalize(float4 __p)        { return __p / length(__p); }
_CLC_OVERLOAD _CLC_INLINE double2 normalize(double2 __p)       { return __p / length(__p); }
_CLC_OVERLOAD _CLC_INLINE double3 normalize(double3 __p)       { return __p / length(__p); }
_CLC_OVERLOAD _CLC_INLINE double4 normalize(double4 __p)       { return __p / length(__p); }

_CLC_OVERLOAD _CLC_INLINE float2  fast_normalize(float2 __p)   { return __p / fast_length(__p); }
_CLC_OVERLOAD _CLC_INLINE float3  fast_normalize(float3 __p)   { return __p / fast_length(__p); }
_CLC_OVERLOAD _CLC_INLINE float4  fast_normalize(float4 __p)   { return __p / fast_length(__p); }
_CLC_OVERLOAD _CLC_INLINE double2 fast_normalize(double2 __p)  { return __p / fast_length(__p); }
_CLC_OVERLOAD _CLC_INLINE double3 fast_normalize(double3 __p)  { return __p / fast_length(__p); }
_CLC_OVERLOAD _CLC_INLINE double4 fast_normalize(double4 __p)  { return __p / fast_length(__p); }

/*-----------------------------------------------------------------------------
* Atomics
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_DECL int  atomic_add(volatile __global int*  __p, int  __val);
_CLC_OVERLOAD _CLC_DECL uint atomic_add(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_add(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_add(volatile __local  uint* __p, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_sub(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_sub(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_sub(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_sub(volatile __local  uint* __p, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_xchg(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xchg(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL float atomic_xchg(volatile __global float* __p, float __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_xchg(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xchg(volatile __local  uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL float atomic_xchg(volatile __local  float* __p, float __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_inc(volatile __global int*  __p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_inc(volatile __global uint* __p); 
_CLC_OVERLOAD _CLC_DECL int  atomic_inc(volatile __local  int*  __p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_inc(volatile __local  uint* __p); 

_CLC_OVERLOAD _CLC_DECL int  atomic_dec(volatile __global int*  __p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_dec(volatile __global uint* __p); 
_CLC_OVERLOAD _CLC_DECL int  atomic_dec(volatile __local  int*  __p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_dec(volatile __local  uint* __p); 

_CLC_OVERLOAD _CLC_DECL int  atomic_cmpxchg(volatile __global int*  __p, int  __cmp, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_cmpxchg(volatile __global uint* __p, uint __cmp, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_cmpxchg(volatile __local  int*  __p, int  __cmp, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_cmpxchg(volatile __local  uint* __p, uint __cmp, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_min(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_min(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_min(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_min(volatile __local  uint* __p, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_max(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_max(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_max(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_max(volatile __local  uint* __p, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_and(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_and(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_and(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_and(volatile __local  uint* __p, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_or(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_or(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_or(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_or(volatile __local  uint* __p, uint __val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_xor(volatile __global int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xor(volatile __global uint* __p, uint __val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_xor(volatile __local  int*  __p, int  __val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xor(volatile __local  uint* __p, uint __val); 

#define atom_add atomic_add
#define atom_sub atomic_sub
#define atom_xchg atomic_xchg
#define atom_inc atomic_inc
#define atom_dec atomic_dec
#define atom_cmpxchg atomic_cmpxchg
#define atom_min atomic_min
#define atom_max atomic_max
#define atom_and atomic_and
#define atom_or atomic_or
#define atom_xor atomic_xor

#define _TEMPLATE2(__res_elemt, __val_vnum, __mask_elemt) \
_CLC_OVERLOAD _CLC_DEF __res_elemt##2 shuffle(__res_elemt##__val_vnum __val, __mask_elemt##2 __mask);\
_CLC_OVERLOAD _CLC_DEF __res_elemt##2 shuffle2(__res_elemt##__val_vnum __val1, __res_elemt##__val_vnum __val2, __mask_elemt##2 __mask); 


#define _TEMPLATE4(__res_elemt, __val_vnum, __mask_elemt) \
_CLC_OVERLOAD _CLC_DEF __res_elemt##4 shuffle(__res_elemt##__val_vnum __val, __mask_elemt##4 __mask); \
_CLC_OVERLOAD _CLC_DEF __res_elemt##4 shuffle2(__res_elemt##__val_vnum __val1, __res_elemt##__val_vnum __val2, __mask_elemt##4 __mask); 


#define _TEMPLATE8(__res_elemt, __val_vnum, __mask_elemt) \
_CLC_OVERLOAD _CLC_DEF __res_elemt##8 shuffle(__res_elemt##__val_vnum __val, __mask_elemt##8 __mask); \
_CLC_OVERLOAD _CLC_DEF __res_elemt##8 shuffle2(__res_elemt##__val_vnum __val1, __res_elemt##__val_vnum __val2, __mask_elemt##8 __mask); 


#define _TEMPLATE16(__res_elemt, __val_vnum, __mask_elemt) \
_CLC_OVERLOAD _CLC_DEF __res_elemt##16 shuffle(__res_elemt##__val_vnum __val, __mask_elemt##16 __mask); \
_CLC_OVERLOAD _CLC_DEF __res_elemt##16 shuffle2(__res_elemt##__val_vnum __val1, __res_elemt##__val_vnum __val2, __mask_elemt##16 __mask); 

#define _CROSS_SIZE(__type1, __type2) \
_TEMPLATE2(__type1, 2, __type2) \
_TEMPLATE2(__type1, 4, __type2) \
_TEMPLATE2(__type1, 8, __type2) \
_TEMPLATE2(__type1, 16, __type2) \
_TEMPLATE4(__type1, 2, __type2) \
_TEMPLATE4(__type1, 4, __type2) \
_TEMPLATE4(__type1, 8, __type2) \
_TEMPLATE4(__type1, 16, __type2) \
_TEMPLATE8(__type1, 2, __type2) \
_TEMPLATE8(__type1, 4, __type2) \
_TEMPLATE8(__type1, 8, __type2) \
_TEMPLATE8(__type1, 16, __type2) \
_TEMPLATE16(__type1, 2, __type2) \
_TEMPLATE16(__type1, 4, __type2) \
_TEMPLATE16(__type1, 8, __type2) \
_TEMPLATE16(__type1, 16, __type2) \

#define _CROSS_MASKTYPE(__type) \
_CROSS_SIZE(__type, uchar) \
_CROSS_SIZE(__type, ushort) \
_CROSS_SIZE(__type, uint) \
_CROSS_SIZE(__type, ulong) \

_CROSS_MASKTYPE(char)
_CROSS_MASKTYPE(uchar)
_CROSS_MASKTYPE(short)
_CROSS_MASKTYPE(ushort)
_CROSS_MASKTYPE(int)
_CROSS_MASKTYPE(uint)
_CROSS_MASKTYPE(long)
_CROSS_MASKTYPE(ulong)
_CROSS_MASKTYPE(float)
_CROSS_MASKTYPE(double)

#undef _TEMPLATE2
#undef _TEMPLATE4
#undef _TEMPLATE8
#undef _TEMPLATE16
#undef _CROSS_SIZE
#undef _CROSS_MASKTYPE

#endif //_CLC_H_
