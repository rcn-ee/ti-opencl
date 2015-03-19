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

#define UNARY_VEC_DECL(type,utype,name) \
_CLC_OVERLOAD _CLC_DECL utype##2  name(type##2 x); \
_CLC_OVERLOAD _CLC_DECL utype##3  name(type##3 x); \
_CLC_OVERLOAD _CLC_DECL utype##4  name(type##4 x); \
_CLC_OVERLOAD _CLC_DECL utype##8  name(type##8 x); \
_CLC_OVERLOAD _CLC_DECL utype##16 name(type##16 x);\

#define BINARY_VEC_DECL(type,utype,name) \
_CLC_OVERLOAD _CLC_DECL utype##2   name(type##2 x, type##2 y);  \
_CLC_OVERLOAD _CLC_DECL utype##3   name(type##3 x, type##3 y);  \
_CLC_OVERLOAD _CLC_DECL utype##4   name(type##4 x, type##4 y);  \
_CLC_OVERLOAD _CLC_DECL utype##8   name(type##8 x, type##8 y);  \
_CLC_OVERLOAD _CLC_DECL utype##16  name(type##16 x, type##16 y);\

#define BINARY_VEC_DECL_ALT(type,utype,type2,name) \
_CLC_OVERLOAD _CLC_DECL utype##2   name(type##2 x, type2##2 y);  \
_CLC_OVERLOAD _CLC_DECL utype##3   name(type##3 x, type2##3 y);  \
_CLC_OVERLOAD _CLC_DECL utype##4   name(type##4 x, type2##4 y);  \
_CLC_OVERLOAD _CLC_DECL utype##8   name(type##8 x, type2##8 y);  \
_CLC_OVERLOAD _CLC_DECL utype##16  name(type##16 x, type2##16 y);\

#define TERNARY_VEC_DECL(type,utype,name) \
_CLC_OVERLOAD _CLC_DECL utype##2   name(type##2 x, type##2 y,  type##2 z);  \
_CLC_OVERLOAD _CLC_DECL utype##3   name(type##3 x, type##3 y,  type##3 z);  \
_CLC_OVERLOAD _CLC_DECL utype##4   name(type##4 x, type##4 y,  type##4 z);  \
_CLC_OVERLOAD _CLC_DECL utype##8   name(type##8 x, type##8 y,  type##8 z);  \
_CLC_OVERLOAD _CLC_DECL utype##16  name(type##16 x, type##16 y,type##16 z);\

#define UNARY_INLINE(type,utype,name,op) \
_CLC_PROTECTED utype op(type x); \
_CLC_OVERLOAD _CLC_INLINE utype name(type x) { return op(x); }

#define BINARY_INLINE(type,utype,name,op) \
_CLC_PROTECTED utype op(type x, type y); \
_CLC_OVERLOAD _CLC_INLINE utype  name(type x, type y) { return op(x, y); }

#define BINARY_INLINE_ALT(type,utype,type2,name,op) \
_CLC_PROTECTED utype op(type x, type2 y); \
_CLC_OVERLOAD _CLC_INLINE utype  name(type x, type2 y) { return op(x, y); }

#define UNARY_VEC_DEF(type,utype,name,op)\
_CLC_OVERLOAD _CLC_DEF utype##2  name(type##2 x)   \
{ return (utype##2)  (op(x.s0), op(x.s1)); }\
_CLC_OVERLOAD _CLC_DEF utype##3  name(type##3 x)   \
{ return (utype##3)  (op(x.s0), op(x.s1), op(x.s2)); }\
_CLC_OVERLOAD _CLC_DEF utype##4  name(type##4 x)   \
{ return (utype##4)  (op(x.s0), op(x.s1), op(x.s2), op(x.s3)); }\
_CLC_OVERLOAD _CLC_DEF utype##8  name(type##8 x)   \
{ return (utype##8)  (op(x.s0), op(x.s1), op(x.s2), op(x.s3),\
                     op(x.s4), op(x.s5), op(x.s6), op(x.s7)); }\
_CLC_OVERLOAD _CLC_DEF utype##16  name(type##16 x)   \
{ return (utype##16)  (op(x.s0), op(x.s1), op(x.s2), op(x.s3),\
                      op(x.s4), op(x.s5), op(x.s6), op(x.s7),\
                      op(x.s8), op(x.s9), op(x.sa), op(x.sb),\
                      op(x.sc), op(x.sd), op(x.se), op(x.sf)); }

#define BINARY_VEC_DEF(type,utype,name,op)\
_CLC_OVERLOAD _CLC_DEF utype##2  name(type##2 x, type##2 y)   \
{ return (utype##2)  (op(x.s0,y.s0), op(x.s1,y.s1)); }\
_CLC_OVERLOAD _CLC_DEF utype##3  name(type##3 x, type##3 y)   \
{ return (utype##3)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2)); }\
_CLC_OVERLOAD _CLC_DEF utype##4  name(type##4 x, type##4 y)   \
{ return (utype##4)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2), op(x.s3,y.s3)); }\
_CLC_OVERLOAD _CLC_DEF utype##8  name(type##8 x, type##8 y)   \
{ return (utype##8)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2), op(x.s3,y.s3),\
                     op(x.s4,y.s4), op(x.s5,y.s5), op(x.s6,y.s6), op(x.s7,y.s7)); }\
_CLC_OVERLOAD _CLC_DEF utype##16  name(type##16 x, type##16 y)   \
{ return (utype##16)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2), op(x.s3,y.s3),\
                      op(x.s4,y.s4), op(x.s5,y.s5), op(x.s6,y.s6), op(x.s7,y.s7),\
                      op(x.s8,y.s8), op(x.s9,y.s9), op(x.sa,y.sa), op(x.sb,y.sb),\
                      op(x.sc,y.sc), op(x.sd,y.sd), op(x.se,y.se), op(x.sf,y.sf)); }

#define BINARY_VEC_DEF_ALT(type,utype,type2,name,op)\
_CLC_OVERLOAD _CLC_DEF utype##2  name(type##2 x, type2##2 y)   \
{ return (utype##2)  (op(x.s0,y.s0), op(x.s1,y.s1)); }\
_CLC_OVERLOAD _CLC_DEF utype##3  name(type##3 x, type2##3 y)   \
{ return (utype##3)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2)); }\
_CLC_OVERLOAD _CLC_DEF utype##4  name(type##4 x, type2##4 y)   \
{ return (utype##4)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2), op(x.s3,y.s3)); }\
_CLC_OVERLOAD _CLC_DEF utype##8  name(type##8 x, type2##8 y)   \
{ return (utype##8)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2), op(x.s3,y.s3),\
                     op(x.s4,y.s4), op(x.s5,y.s5), op(x.s6,y.s6), op(x.s7,y.s7)); }\
_CLC_OVERLOAD _CLC_DEF utype##16  name(type##16 x, type2##16 y)   \
{ return (utype##16)  (op(x.s0,y.s0), op(x.s1,y.s1), op(x.s2,y.s2), op(x.s3,y.s3),\
                      op(x.s4,y.s4), op(x.s5,y.s5), op(x.s6,y.s6), op(x.s7,y.s7),\
                      op(x.s8,y.s8), op(x.s9,y.s9), op(x.sa,y.sa), op(x.sb,y.sb),\
                      op(x.sc,y.sc), op(x.sd,y.sd), op(x.se,y.se), op(x.sf,y.sf)); }

#define TERNARY_VEC_DEF(type,utype,name,op)\
_CLC_OVERLOAD _CLC_DEF utype##2  name(type##2 x, type##2 y, type##2 z)   \
{ return (utype##2)  (op(x.s0,y.s0,z.s0), op(x.s1,y.s1,z.s1)); }\
_CLC_OVERLOAD _CLC_DEF utype##3  name(type##3 x, type##3 y, type##3 z)   \
{ return (utype##3)  (op(x.s0,y.s0,z.s0), op(x.s1,y.s1,z.s1), op(x.s2,y.s2,z.s2)); }\
_CLC_OVERLOAD _CLC_DEF utype##4  name(type##4 x, type##4 y, type##4 z)   \
{ return (utype##4)  (op(x.s0,y.s0,z.s0), op(x.s1,y.s1,z.s1), \
                      op(x.s2,y.s2,z.s2), op(x.s3,y.s3,z.s3)); }\
_CLC_OVERLOAD _CLC_DEF utype##8  name(type##8 x, type##8 y, type##8 z)   \
{ return (utype##8)  (op(x.s0,y.s0,z.s0), op(x.s1,y.s1,z.s1), \
                      op(x.s2,y.s2,z.s2), op(x.s3,y.s3,z.s3),\
                      op(x.s4,y.s4,z.s4), op(x.s5,y.s5,z.s5), \
                      op(x.s6,y.s6,z.s6), op(x.s7,y.s7,z.s7)); }\
_CLC_OVERLOAD _CLC_DEF utype##16  name(type##16 x, type##16 y, type##16 z)   \
{ return (utype##16)  (op(x.s0,y.s0,z.s0), op(x.s1,y.s1,z.s1), \
                       op(x.s2,y.s2,z.s2), op(x.s3,y.s3,z.s3),\
                       op(x.s4,y.s4,z.s4), op(x.s5,y.s5,z.s5), \
                       op(x.s6,y.s6,z.s6), op(x.s7,y.s7,z.s7),\
                       op(x.s8,y.s8,z.s8), op(x.s9,y.s9,z.s9), \
                       op(x.sa,y.sa,z.sa), op(x.sb,y.sb,z.sb),\
                       op(x.sc,y.sc,z.sc), op(x.sd,y.sd,z.sd), \
                       op(x.se,y.se,z.se), op(x.sf,y.sf,z.sf)); }


#define _VEC_TYPE(type,sz) type##sz

#define _EXPAND_TYPES()   \
    EXPAND_SIZES(char)   \
    EXPAND_SIZES(uchar)  \
    EXPAND_SIZES(short)  \
    EXPAND_SIZES(ushort) \
    EXPAND_SIZES(int)    \
    EXPAND_SIZES(uint)   \
    EXPAND_SIZES(long)   \
    EXPAND_SIZES(ulong)  \
    EXPAND_SIZES(float)  \
    EXPAND_SIZES(double) 

#define _EXPAND_INTEGER_TYPES()   \
    EXPAND_SIZES(char)   \
    EXPAND_SIZES(uchar)  \
    EXPAND_SIZES(short)  \
    EXPAND_SIZES(ushort) \
    EXPAND_SIZES(int)    \
    EXPAND_SIZES(uint)   \
    EXPAND_SIZES(long)   \
    EXPAND_SIZES(ulong)  

typedef unsigned int cl_mem_fence_flags;

/*-----------------------------------------------------------------------------
* Standard types from Clang's stddef and stdint, Copyright (C) 2008 Eli Friedman
*----------------------------------------------------------------------------*/
typedef signed   __INT64_TYPE__ int64_t;
typedef unsigned __INT64_TYPE__ uint64_t;
typedef signed   __INT32_TYPE__ int32_t;
typedef unsigned __INT32_TYPE__ uint32_t;
typedef signed   __INT16_TYPE__ int16_t;
typedef unsigned __INT16_TYPE__ uint16_t;
typedef signed   __INT8_TYPE__  int8_t;
typedef unsigned __INT8_TYPE__  uint8_t;

#define __stdint_join3(a,b,c) a ## b ## c
#define  __intn_t(n) __stdint_join3( int, n, _t)
#define __uintn_t(n) __stdint_join3(uint, n, _t)

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
#define COAL_VECTOR(type, len)                                  \
   typedef type type##len __attribute__((ext_vector_type(len)))

#define COAL_VECTOR_SET(type) \
   COAL_VECTOR(type, 2);      \
   COAL_VECTOR(type, 3);      \
   COAL_VECTOR(type, 4);      \
   COAL_VECTOR(type, 8);      \
   COAL_VECTOR(type, 16);

COAL_VECTOR_SET(char)
COAL_VECTOR_SET(uchar)
COAL_VECTOR_SET(short)
COAL_VECTOR_SET(ushort)
COAL_VECTOR_SET(int)
COAL_VECTOR_SET(uint)
COAL_VECTOR_SET(long)
COAL_VECTOR_SET(ulong)
COAL_VECTOR_SET(float)
COAL_VECTOR_SET(double)

#undef COAL_VECTOR_SET
#undef COAL_VECTOR

#define CL_VERSION_1_0          100
#define CL_VERSION_1_1          110
#define __OPENCL_VERSION__      110
#define __ENDIAN_LITTLE__       1
#define __kernel_exec(X, typen) __kernel __attribute__((work_group_size_hint(X, 1, 1))) \
                                __attribute__((vec_type_hint(typen)))
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

_CLC_PROTECTED _CLC_NODUP void  barrier(cl_mem_fence_flags flags);
_CLC_PROTECTED void   mem_fence        (cl_mem_fence_flags flags);
_CLC_PROTECTED void   read_mem_fence   (cl_mem_fence_flags flags);
_CLC_PROTECTED void   write_mem_fence  (cl_mem_fence_flags flags);

/******************************************************************************
* AS_<type> functions
******************************************************************************/
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

#define _CLC_CONVERT_DECL(FROM_TYPE, TO_TYPE, SUFFIX) \
  _CLC_OVERLOAD _CLC_DECL TO_TYPE convert_##TO_TYPE##SUFFIX(FROM_TYPE x);

#define _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, TO_TYPE, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE, TO_TYPE, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##2, TO_TYPE##2, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##3, TO_TYPE##3, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##4, TO_TYPE##4, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##8, TO_TYPE##8, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##16, TO_TYPE##16, SUFFIX)

#define _CLC_VECTOR_CONVERT_FROM1(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, char, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, uchar, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, int, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, uint, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, short, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, ushort, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, long, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, ulong, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, float, SUFFIX)

#define _CLC_VECTOR_CONVERT_FROM(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM1(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, double, SUFFIX)

#define _CLC_VECTOR_CONVERT_TO1(SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(char, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(uchar, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(int, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(uint, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(short, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(ushort, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(long, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(ulong, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(float, SUFFIX)

#define _CLC_VECTOR_CONVERT_TO(SUFFIX) \
  _CLC_VECTOR_CONVERT_TO1(SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(double, SUFFIX)

#define _CLC_VECTOR_CONVERT_TO_SUFFIX(ROUND) \
  _CLC_VECTOR_CONVERT_TO(_sat##ROUND) \
  _CLC_VECTOR_CONVERT_TO(ROUND)

_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtn)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rte)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtz)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtp)
_CLC_VECTOR_CONVERT_TO_SUFFIX()

#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##2 vload2(size_t offset, const ADDR_SPACE PRIM_TYPE *x) \
   { return (PRIM_TYPE##2)(x[offset<<1] , x[1+(offset<<1)]); } \
  _CLC_OVERLOAD _CLC_DECL PRIM_TYPE##3 vload3(size_t offset, const ADDR_SPACE PRIM_TYPE *x); \
  _CLC_OVERLOAD _CLC_DECL PRIM_TYPE##4 vload4(size_t offset, const ADDR_SPACE PRIM_TYPE *x); \
  _CLC_OVERLOAD _CLC_DECL PRIM_TYPE##8 vload8(size_t offset, const ADDR_SPACE PRIM_TYPE *x); \
  _CLC_OVERLOAD _CLC_DECL PRIM_TYPE##16 vload16(size_t offset, const ADDR_SPACE PRIM_TYPE *x);

#define VLOAD_ADDR_SPACES(__CLC_SCALAR_GENTYPE) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __private) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __local) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __constant) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __global) \

#define VLOAD_TYPES() \
    VLOAD_ADDR_SPACES(char) \
    VLOAD_ADDR_SPACES(uchar) \
    VLOAD_ADDR_SPACES(short) \
    VLOAD_ADDR_SPACES(ushort) \
    VLOAD_ADDR_SPACES(int) \
    VLOAD_ADDR_SPACES(uint) \
    VLOAD_ADDR_SPACES(long) \
    VLOAD_ADDR_SPACES(ulong) \
    VLOAD_ADDR_SPACES(float) \
    VLOAD_ADDR_SPACES(double)\

VLOAD_TYPES()

#define VLOAD_HALF_VECTORIZE(SPACE) \
_CLC_OVERLOAD _CLC_DEF float  vload_half(size_t offset, const SPACE half *p);  \
_CLC_OVERLOAD _CLC_DEF float2 vload_half2(size_t offset, const SPACE half *p); \
_CLC_OVERLOAD _CLC_DEF float3 vload_half3(size_t offset, const SPACE half *p); \
_CLC_OVERLOAD _CLC_DEF float3 vloada_half3(size_t offset, const SPACE half *p);\
_CLC_OVERLOAD _CLC_DEF float4 vload_half4(size_t offset, const SPACE half *p); \
_CLC_OVERLOAD _CLC_DEF float8 vload_half8(size_t offset, const SPACE half *p); \
_CLC_OVERLOAD _CLC_DEF float16 vload_half16(size_t offset, const SPACE half *p);

VLOAD_HALF_VECTORIZE(__global)
VLOAD_HALF_VECTORIZE(__local)
VLOAD_HALF_VECTORIZE(__constant)
VLOAD_HALF_VECTORIZE(__private)

#define vloada_half vload_half
#define vloada_half2 vload_half2
#define vloada_half4 vload_half4
#define vloada_half8 vload_half8
#define vloada_half16 vload_half16

#undef VLOAD_VECTORIZE
#undef VLOAD_ADDR_SPACES
#undef VLOAD_TYPES
#undef VLOAD_HALF_VECTORIZE

#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE void vstore2(PRIM_TYPE##2 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) \
                                              { mem[offset<<1] = vec.s0; mem[1+(offset<<1)] = vec.s1; } \
  _CLC_OVERLOAD _CLC_DECL void vstore3(PRIM_TYPE##3 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem); \
  _CLC_OVERLOAD _CLC_DECL void vstore4(PRIM_TYPE##4 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem); \
  _CLC_OVERLOAD _CLC_DECL void vstore8(PRIM_TYPE##8 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem); \
  _CLC_OVERLOAD _CLC_DECL void vstore16(PRIM_TYPE##16 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem); \

#define VSTORE_ADDR_SPACES(__CLC_SCALAR___CLC_GENTYPE) \
    VSTORE_VECTORIZE(__CLC_SCALAR___CLC_GENTYPE, __private) \
    VSTORE_VECTORIZE(__CLC_SCALAR___CLC_GENTYPE, __local) \
    VSTORE_VECTORIZE(__CLC_SCALAR___CLC_GENTYPE, __global) \

#define VSTORE_TYPES() \
    VSTORE_ADDR_SPACES(char) \
    VSTORE_ADDR_SPACES(uchar) \
    VSTORE_ADDR_SPACES(short) \
    VSTORE_ADDR_SPACES(ushort) \
    VSTORE_ADDR_SPACES(int) \
    VSTORE_ADDR_SPACES(uint) \
    VSTORE_ADDR_SPACES(long) \
    VSTORE_ADDR_SPACES(ulong) \
    VSTORE_ADDR_SPACES(float) \
    VSTORE_ADDR_SPACES(double) \

VSTORE_TYPES()

#undef VSTORE_VECTORIZE
#undef VSTORE_ADDR_SPACES
#undef VSTORE_TYPES


#define DECL_VSTORE_HALF_SPACE_ROUND(SPACE, ROUND, FUNC) \
   _CLC_OVERLOAD _CLC_DECL void vstore_half##ROUND(float data, size_t offset, SPACE half *p); \
   _CLC_OVERLOAD _CLC_DECL void vstorea_half##ROUND(float data, size_t offset, SPACE half *p); \
   _CLC_OVERLOAD _CLC_DECL void vstore_half2##ROUND(float2 data, size_t offset, SPACE half *p);\
   _CLC_OVERLOAD _CLC_DECL void vstorea_half2##ROUND(float2 data, size_t offset, SPACE half *p);\
   _CLC_OVERLOAD _CLC_DECL void vstore_half3##ROUND(float3 data, size_t offset, SPACE half *p); \
   _CLC_OVERLOAD _CLC_DECL void vstorea_half3##ROUND(float3 data, size_t offset, SPACE half *p); \
   _CLC_OVERLOAD _CLC_DECL void vstore_half4##ROUND(float4 data, size_t offset, SPACE half *p); \
   _CLC_OVERLOAD _CLC_DECL void vstorea_half4##ROUND(float4 data, size_t offset, SPACE half *p);\
   _CLC_OVERLOAD _CLC_DECL void vstore_half8##ROUND(float8 data, size_t offset, SPACE half *p);\
   _CLC_OVERLOAD _CLC_DECL void vstorea_half8##ROUND(float8 data, size_t offset, SPACE half *p);\
   _CLC_OVERLOAD _CLC_DECL void vstore_half16##ROUND(float16 data, size_t offset, SPACE half *p);\
   _CLC_OVERLOAD _CLC_DECL void vstorea_half16##ROUND(float16 data, size_t offset, SPACE half *p);

#define DECL_VSTORE_HALF_SPACE(SPACE) \
  DECL_VSTORE_HALF_SPACE_ROUND(SPACE,     , __ocl_float_to_half) \
  DECL_VSTORE_HALF_SPACE_ROUND(SPACE, _rte, __ocl_float_to_half_rte) \
  DECL_VSTORE_HALF_SPACE_ROUND(SPACE, _rtz, __ocl_float_to_half_rtz) \
  DECL_VSTORE_HALF_SPACE_ROUND(SPACE, _rtp, __ocl_float_to_half_rtp) \
  DECL_VSTORE_HALF_SPACE_ROUND(SPACE, _rtn, __ocl_float_to_half_rtn) 

DECL_VSTORE_HALF_SPACE(__global)
DECL_VSTORE_HALF_SPACE(__local)
DECL_VSTORE_HALF_SPACE(__private)

#undef DECL_VSTORE_HALF_SPACE
#undef DECL_VSTORE_HALF_SPACE_ROUND

/*-----------------------------------------------------------------------------
* Relational
*----------------------------------------------------------------------------*/
#define INLN(type) \
_CLC_OVERLOAD _CLC_INLINE type bitselect(type a, type b, type c) { return a^(c&(b^a)); }

#define DECL(type) \
_CLC_OVERLOAD _CLC_DECL type bitselect(type a, type b, type c);

INLN(char)
INLN(uchar)
INLN(short)
INLN(ushort)
INLN(int)
INLN(uint)
INLN(long)
INLN(ulong)

DECL(char2)
DECL(uchar2)
INLN(short2)
INLN(ushort2)
INLN(int2)
INLN(uint2)
DECL(long2)
DECL(ulong2)

DECL(char3)
DECL(uchar3)
DECL(short3)
DECL(ushort3)
DECL(int3)
DECL(uint3)
DECL(long3)
DECL(ulong3)

INLN(char4)
INLN(uchar4)
INLN(short4)
INLN(ushort4)
DECL(int4)
DECL(uint4)
DECL(long4)
DECL(ulong4)

INLN(char8)
INLN(uchar8)
DECL(short8)
DECL(ushort8)
DECL(int8)
DECL(uint8)
DECL(long8)
DECL(ulong8)

DECL(char16)
DECL(uchar16)
DECL(short16)
DECL(ushort16)
DECL(int16)
DECL(uint16)
DECL(long16)
DECL(ulong16)

DECL(float)
DECL(float2)
DECL(float3)
DECL(float4)
DECL(float8)
DECL(float16)

DECL(double)
DECL(double2)
DECL(double3)
DECL(double4)
DECL(double8)
DECL(double16)

#undef INLN
#undef DECL

#define EXTU(x,l,r)     (((x) << l) >> r)

#define SIGND(x)        (as_uint2(x).hi >> 31)
#define EXPD(x)         EXTU(as_uint2(x).hi, 1, 21)
#define MANTD_HI(x)     EXTU(as_uint2(x).hi, 12, 12)
#define MANTD_LO(x)     as_uint2(x).lo
#define MANTD_ZERO(x)   (MANTD_HI(x) == 0 && MANTD_LO(x) == 0)
#define ANY_ZEROD(x)    ((as_ulong(x) << 1) == 0)
#define SUBNORMD(x)     (EXPD(x) == 0 && !MANTD_ZERO(x))

#define FABSF(x)        ((as_uint(x) << 1) >> 1)
#define SIGNF(x)        (as_uint(x) >> 31)
#define EXPF(x)         ((as_uint(x) << 1) >> 24)
#define MANTF(x)        ((as_uint(x) << 9) >> 9)

#define isordered(x,y)          (!isnan(x) & !isnan(y))
#define isunordered(x,y)        (isnan(x)  | isnan(y))

_CLC_OVERLOAD _CLC_INLINE int  isnan(float  x)   { return FABSF(x) > 0x7F800000; }
UNARY_INLINE  (double, int,  isnan, __builtin_isnan)
UNARY_VEC_DECL(float,  int,  isnan)
UNARY_VEC_DECL(double, long, isnan)

_CLC_OVERLOAD _CLC_INLINE int  isfinite(float  x)   { return EXPF(x) != 255; }
UNARY_INLINE  (double, int,  isfinite, __builtin_isfinite)
UNARY_VEC_DECL(float,  int,  isfinite)
UNARY_VEC_DECL(double, long, isfinite)

_CLC_OVERLOAD _CLC_INLINE int  isinf(float  x)   { return FABSF(x) == 0x7F800000; }
UNARY_INLINE  (double, int,  isinf, __builtin_isinf)
UNARY_VEC_DECL(float,  int,  isinf)
UNARY_VEC_DECL(double, long, isinf)

_CLC_OVERLOAD _CLC_INLINE int  isnormal(float  x)   { return EXPF(x) != 0 && EXPF(x) != 255; }
UNARY_INLINE  (double, int,  isnormal, __builtin_isnormal)
UNARY_VEC_DECL(float,  int,  isnormal)
UNARY_VEC_DECL(double, long, isnormal)

_CLC_OVERLOAD _CLC_INLINE int  signbit(float  x) { return SIGNF(x); }
_CLC_OVERLOAD _CLC_INLINE int  signbit(double x) { return SIGND(x); }
UNARY_VEC_DECL(float,  int,  signbit)
UNARY_VEC_DECL(double, long, signbit)

_CLC_OVERLOAD _CLC_INLINE float copysign(float x, float y)
    { return as_float(FABSF(x) | (SIGNF(y) << 31)); }

_CLC_OVERLOAD _CLC_INLINE double copysign(double x, double y)
{ return as_double(((as_ulong(x) << 1) >> 1) | ((as_ulong(y) >> 63) << 63)); }

BINARY_VEC_DECL(float,  float,  copysign)
BINARY_VEC_DECL(double, double, copysign)

_CLC_OVERLOAD _CLC_INLINE int  isequal(float  x, float y)  { return x == y; }
_CLC_OVERLOAD _CLC_INLINE int  isequal(double x, double y) 
{ 
    if (EXPD(x) == 0 && EXPD(y) == 0)
    {
        if (MANTD_ZERO(x) && MANTD_ZERO(y)) return 1;
        return as_ulong(x) == as_ulong(y);
    }
    return x == y;

}
BINARY_VEC_DECL(float,  int,  isequal)
BINARY_VEC_DECL(double, long, isequal)

_CLC_OVERLOAD _CLC_INLINE int  isnotequal(float  x, float y)  { return x != y; }
_CLC_OVERLOAD _CLC_INLINE int  isnotequal(double x, double y) 
{ 
    if (EXPD(x) == 0 && EXPD(y) == 0)
    {
        if (MANTD_ZERO(x) && MANTD_ZERO(y)) return 0;
        return as_ulong(x) != as_ulong(y);
    }
    return x != y;

}
BINARY_VEC_DECL(float,  int,  isnotequal)
BINARY_VEC_DECL(double, long, isnotequal)

_CLC_OVERLOAD _CLC_INLINE int  isless(float  x, float y)  { return x < y; }
_CLC_OVERLOAD _CLC_INLINE int  isless(double x, double y) 
{ 
    if (SUBNORMD(x) || SUBNORMD(y))
    {
        if (isunordered(x,y))    return 0;
        if (SIGND(x) ^ SIGND(y)) return (SIGND(x)) ? 1 : 0;
        return SIGND(x) ? as_ulong(x) > as_ulong(y)
                        : as_ulong(x) < as_ulong(y);
    }
    return x < y;
}
BINARY_VEC_DECL(float,  int,  isless)
BINARY_VEC_DECL(double, long, isless)

_CLC_OVERLOAD _CLC_INLINE int  islessequal(float  x, float y)  { return x <= y; }
_CLC_OVERLOAD _CLC_INLINE int  islessequal(double x, double y) 
{ 
    if (SUBNORMD(x) || SUBNORMD(y))
    {
        if (isunordered(x,y))    return 0;
        if (SIGND(x) ^ SIGND(y)) return (SIGND(x)) ? 1 : 0;
        return SIGND(x) ? as_ulong(x) >= as_ulong(y)
                        : as_ulong(x) <= as_ulong(y);
    }
    return x <= y;
}
BINARY_VEC_DECL(float,  int,  islessequal)
BINARY_VEC_DECL(double, long, islessequal)

_CLC_OVERLOAD _CLC_INLINE int  isgreater(float  x, float y)  { return x > y; }
_CLC_OVERLOAD _CLC_INLINE int  isgreater(double x, double y) 
{ 
    if (SUBNORMD(x) || SUBNORMD(y))
    {
        if (isunordered(x,y))    return 0;
        if (SIGND(x) ^ SIGND(y)) return (SIGND(x)) ? 0 : 1;
        return SIGND(x) ? as_ulong(x) < as_ulong(y)
                        : as_ulong(x) > as_ulong(y);
    }
    return x > y;
}
BINARY_VEC_DECL(float,  int,  isgreater)
BINARY_VEC_DECL(double, long, isgreater)

_CLC_OVERLOAD _CLC_INLINE int  isgreaterequal(float  x, float y)  { return x >= y; }
_CLC_OVERLOAD _CLC_INLINE int  isgreaterequal(double x, double y) 
{ 
    if (SUBNORMD(x) || SUBNORMD(y))
    {
        if (isunordered(x,y))    return 0;
        if (SIGND(x) ^ SIGND(y)) return (SIGND(x)) ? 0 : 1;
        return SIGND(x) ? as_ulong(x) <= as_ulong(y)
                        : as_ulong(x) >= as_ulong(y);
    }
    return x >= y;
}
BINARY_VEC_DECL(float,  int,  isgreaterequal)
BINARY_VEC_DECL(double, long, isgreaterequal)

_CLC_OVERLOAD _CLC_INLINE int  islessgreater(float  x, float y)  
{ return isless(x,y) | isgreater(x, y); }
_CLC_OVERLOAD _CLC_INLINE int  islessgreater(double x, double y) 
{ return isless(x,y) | isgreater(x, y); }
BINARY_VEC_DECL(float,  int,  islessgreater)
BINARY_VEC_DECL(double, long, islessgreater)

#undef EXPD
#undef MANTD_HI
#undef MANTD_LO
#undef MANTD_ZERO
#undef SIGND
#undef FABSF
#undef SIGNF
#undef EXPF
#undef MANTF
#undef EXTU


#define TEMPLATE(type) \
_CLC_OVERLOAD _CLC_INLINE int any(type x)     { return x < 0; } \
_CLC_OVERLOAD _CLC_INLINE int any(type##2 x)  { return (x.s0 | x.s1) < 0; } \
_CLC_OVERLOAD _CLC_DECL   int any(type##3 x); \
_CLC_OVERLOAD _CLC_DECL   int any(type##4 x); \
_CLC_OVERLOAD _CLC_DECL   int any(type##8 x); \
_CLC_OVERLOAD _CLC_DECL   int any(type##16 x); \

TEMPLATE(char)
TEMPLATE(short)
TEMPLATE(int)
TEMPLATE(long)

#undef TEMPLATE

#define TEMPLATE(type) \
_CLC_OVERLOAD _CLC_INLINE int all(type x)     { return x < 0; } \
_CLC_OVERLOAD _CLC_INLINE int all(type##2 x)  { return (x.s0 & x.s1) < 0; } \
_CLC_OVERLOAD _CLC_DECL   int all(type##3 x); \
_CLC_OVERLOAD _CLC_DECL   int all(type##4 x); \
_CLC_OVERLOAD _CLC_DECL   int all(type##8 x); \
_CLC_OVERLOAD _CLC_DECL   int all(type##16 x); \

TEMPLATE(char)
TEMPLATE(short)
TEMPLATE(int)
TEMPLATE(long)

#undef TEMPLATE

#define DEFINE(type, otype) \
_CLC_OVERLOAD _CLC_INLINE type select(type a, type b, otype c) { return c ? b : a; }

DEFINE(char, char)
DEFINE(char, uchar)
DEFINE(uchar, char)
DEFINE(uchar, uchar)
DEFINE(short, short)
DEFINE(short, ushort)
DEFINE(ushort, short)
DEFINE(ushort, ushort)
DEFINE(int, int)
DEFINE(int, uint)
DEFINE(uint, int)
DEFINE(uint, uint)
DEFINE(long, long)
DEFINE(long, ulong)
DEFINE(ulong, long)
DEFINE(ulong, ulong)
DEFINE(float, int)
DEFINE(float, uint)
DEFINE(double, long)
DEFINE(double, ulong)

#undef  DEFINE

#define DECLARATION(type, itype, utype) \
_CLC_OVERLOAD _CLC_DECL type select(type a, type b, itype c);\
_CLC_OVERLOAD _CLC_DECL type select(type a, type b, utype c);

#define SELECT_EXPAND_SIZES(type,itype,utype) \
    DECLARATION(_VEC_TYPE(type,2), _VEC_TYPE(itype,2), _VEC_TYPE(utype,2))  \
    DECLARATION(_VEC_TYPE(type,3), _VEC_TYPE(itype,3), _VEC_TYPE(utype,3))  \
    DECLARATION(_VEC_TYPE(type,4), _VEC_TYPE(itype,4), _VEC_TYPE(utype,4))  \
    DECLARATION(_VEC_TYPE(type,8), _VEC_TYPE(itype,8), _VEC_TYPE(utype,8))  \
    DECLARATION(_VEC_TYPE(type,16), _VEC_TYPE(itype,16), _VEC_TYPE(utype,16))  \

#define SELECT_EXPAND_TYPES   \
    SELECT_EXPAND_SIZES(char, char, uchar)   \
    SELECT_EXPAND_SIZES(uchar, char, uchar)  \
    SELECT_EXPAND_SIZES(short, short, ushort)  \
    SELECT_EXPAND_SIZES(ushort, short, ushort) \
    SELECT_EXPAND_SIZES(int, int, uint)    \
    SELECT_EXPAND_SIZES(uint, int, uint)   \
    SELECT_EXPAND_SIZES(long, long, ulong)   \
    SELECT_EXPAND_SIZES(ulong, long, ulong)  \
    SELECT_EXPAND_SIZES(float, int, uint)  \
    SELECT_EXPAND_SIZES(double, long, ulong)

SELECT_EXPAND_TYPES

#undef DECLARATION
#undef SELECT_EXPAND_SIZES
#undef SELECT_EXPAND_TYPES

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
#define FLT_MAX_10_EXP  +38
#define FLT_MAX_EXP     +128
#define FLT_MIN_10_EXP  -37
#define FLT_MIN_EXP     -125
#define FLT_RADIX       2
#define FLT_MAX         0x1.fffffep127f
#define FLT_MIN         0x1.0p-126f
#define FLT_EPSILON     0x1.0p-23f

#define DBL_DIG         15
#define DBL_MANT_DIG    53
#define DBL_MAX_10_EXP  +308
#define DBL_MAX_EXP     +1024
#define DBL_MIN_10_EXP  -307
#define DBL_MIN_EXP     -1021
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

#define UNARY(function) \
UNARY_INLINE  (float,  float,  function, function##f)\
UNARY_INLINE  (double, double, function, function##d)\
UNARY_VEC_DECL(float,  float,  function)\
UNARY_VEC_DECL(double, double, function)\

#define UNARYT(type1, type2, function,op) \
UNARY_INLINE  (type1,  type2,  function, op)\
UNARY_VEC_DECL(type1,  type2,  function)\

#define BINARY(function) \
BINARY_INLINE  (float,  float,  function, function##f)\
BINARY_INLINE  (double, double, function, function##d)\
BINARY_VEC_DECL(float,  float,  function)\
BINARY_VEC_DECL(double, double, function)\

/*-------------------------------------------------------------------------
* Prototypes for the math builtins 
*------------------------------------------------------------------------*/
UNARY(acos)
UNARY(acosh)

_CLC_PROTECTED float  acosf(float);
_CLC_PROTECTED double acosd(double);
_CLC_OVERLOAD _CLC_INLINE float  acospi(float x)  { return acosf(x) * M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double acospi(double x) { return acosd(x) * M_1_PI; }
UNARY_VEC_DECL(float,  float,  acospi)
UNARY_VEC_DECL(double, double, acospi)

UNARY(asin)
UNARY(asinh)

_CLC_PROTECTED float  asinf(float);
_CLC_PROTECTED double asind(double);
_CLC_OVERLOAD _CLC_INLINE float  asinpi(float x)  { return asinf(x) * M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double asinpi(double x) { return asind(x) * M_1_PI; }
UNARY_VEC_DECL(float,  float,  asinpi)
UNARY_VEC_DECL(double, double, asinpi)

UNARY(atan)
UNARY(atanh)

_CLC_PROTECTED float  atanf(float);
_CLC_PROTECTED double atand(double);
_CLC_OVERLOAD _CLC_INLINE float  atanpi(float x)  { return atanf(x) * M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double atanpi(double x) { return atand(x) * M_1_PI; }
UNARY_VEC_DECL(float,  float,  atanpi)
UNARY_VEC_DECL(double, double, atanpi)

BINARY(atan2)

_CLC_OVERLOAD _CLC_INLINE float atan2pi(float y, float x)
    { return atan2f(y,x) * (float) M_1_PI; }
_CLC_OVERLOAD _CLC_INLINE double atan2pi(double y, double x)
    { return atan2d(y,x) *  M_1_PI; }
BINARY_VEC_DECL(float,  float,  atan2pi)
BINARY_VEC_DECL(double, double, atan2pi)

UNARY(cbrt)
UNARY(ceil)

UNARY(cos)
UNARY(cosh)
UNARY(cospi)
UNARY(erf)
UNARY(erfc)
UNARY(exp)
UNARY(exp2)
UNARY(exp10)
UNARY(expm1)

UNARY_INLINE  (float,  float,  fabs, _fabsf)\
UNARY_INLINE  (double, double, fabs, _fabs)\
UNARY_VEC_DECL(float,  float,  fabs)
UNARY_VEC_DECL(double, double, fabs)

BINARY(fdim)
UNARY(floor)

_CLC_PROTECTED float  fmaf(float, float, float);
_CLC_PROTECTED double fmad(double, double, double);
_CLC_OVERLOAD _CLC_INLINE float  fma(float a,  float b,  float c)  { return fmaf(a,b,c); }
_CLC_OVERLOAD _CLC_INLINE double fma(double a, double b, double c) { return fmad(a,b,c); }
TERNARY_VEC_DECL(float,  float,  fma)
TERNARY_VEC_DECL(double, double, fma)

BINARY(fmax)
BINARY(fmin)
BINARY(fmod)
BINARY(hypot)

UNARYT(float,  int, ilogb, ilogbf)
UNARYT(double, int, ilogb, ilogbd)

BINARY_INLINE_ALT  (float,  float,  int, ldexp, ldexpf)
BINARY_INLINE_ALT  (double, double, int, ldexp, ldexpd)
BINARY_VEC_DECL_ALT(float,  float,  int, ldexp)
BINARY_VEC_DECL_ALT(double, double, int, ldexp)

UNARY(lgamma)
UNARY(log)
UNARY(log2)
UNARY(log10)
UNARY(log1p)
UNARY(logb)

_CLC_OVERLOAD _CLC_INLINE float  mad(float a,  float b,  float c)  { return (a*b)+c; }
_CLC_OVERLOAD _CLC_INLINE double mad(double a, double b, double c) { return (a*b)+c; }
TERNARY_VEC_DECL(float,  float,  mad)
TERNARY_VEC_DECL(double, double, mad)

BINARY(maxmag)
BINARY(minmag)

_CLC_OVERLOAD _CLC_INLINE float  nan(uint  nancode)
    { return as_float(0x7FC00000 | nancode); }
_CLC_OVERLOAD _CLC_INLINE double nan(ulong nancode)
    { return as_double(0x7FF8000000000000ul | nancode); }
UNARY_VEC_DECL(uint,  float,  nan)
UNARY_VEC_DECL(ulong, double, nan)

BINARY(nextafter)
BINARY(pow)

BINARY_INLINE_ALT  (float,  float,  int, pown, pownf)
BINARY_INLINE_ALT  (double, double, int, pown, pownd)
BINARY_VEC_DECL_ALT(float,  float,  int, pown)
BINARY_VEC_DECL_ALT(double, double, int, pown)

BINARY(powr)

BINARY(remainder)
UNARY(rint)

BINARY_INLINE_ALT  (float,  float,  int, rootn, rootnf)
BINARY_INLINE_ALT  (double, double, int, rootn, rootnd)
BINARY_VEC_DECL_ALT(float,  float,  int, rootn)
BINARY_VEC_DECL_ALT(double, double, int, rootn)

UNARY(round)
UNARY(rsqrt)
UNARY(sin)
UNARY(sinh)
UNARY(sinpi)
UNARY(sqrt)
UNARY(tan)
UNARY(tanh)
UNARY(trunc)
UNARY(tanpi)
UNARY(tgamma)

/*-----------------------------------------------------------------------------
* Half versions
*----------------------------------------------------------------------------*/
#define half_cos(x)             cos(x)
#define half_divide(x,y)        (x/y)
#define half_exp(x)             exp(x)
#define half_exp2(x)            exp2(x)
#define half_exp10(x)           exp10(x)
#define half_log(x)             log(x)
#define half_log2(x)            log2(x)
#define half_log10(x)           log10(x)
#define half_powr(x,y)          powr(x,y)
#define half_recip(x)           reciprocal(x)
#define half_rsqrt(x)           rsqrt(x)
#define half_sin(x)             sin(x)
#define half_sqrt(x)            sqrt(x)
#define half_tan(x)             tan(x)

/*-----------------------------------------------------------------------------
* Native versions
*----------------------------------------------------------------------------*/
#define native_sin(x)           sin(x)
#define native_cos(x)           cos(x)
#define native_powr(x,y)        powr(x,y)
#define native_exp(x)           exp(x)
#define native_exp2(x)          exp2(x)
#define native_exp10(x)         exp10(x)
#define native_log2(x)          log2(x)
#define native_log10(x)         log10(x)

UNARY(reciprocal)
UNARY(native_divide)
UNARY(native_recip)
UNARY(native_rsqrt)
UNARY(native_sqrt)

#define native_tan(x)           tan(x)

#undef UNARY
#undef UNARYT
#undef BINARY

_CLC_OVERLOAD _CLC_DECL float modf(float x, global  float * iptr);
_CLC_OVERLOAD _CLC_DECL float modf(float x, local   float * iptr);
_CLC_OVERLOAD _CLC_DECL float modf(float x, private float * iptr);

_CLC_OVERLOAD _CLC_DECL float2 modf(float2 x, global  float2 * iptr);
_CLC_OVERLOAD _CLC_DECL float2 modf(float2 x, local   float2 * iptr);
_CLC_OVERLOAD _CLC_DECL float2 modf(float2 x, private float2 * iptr);

_CLC_OVERLOAD _CLC_DECL float3 modf(float3 x, global  float3 * iptr);
_CLC_OVERLOAD _CLC_DECL float3 modf(float3 x, local   float3 * iptr);
_CLC_OVERLOAD _CLC_DECL float3 modf(float3 x, private float3 * iptr);

_CLC_OVERLOAD _CLC_DECL float4 modf(float4 x, global  float4 * iptr);
_CLC_OVERLOAD _CLC_DECL float4 modf(float4 x, local   float4 * iptr);
_CLC_OVERLOAD _CLC_DECL float4 modf(float4 x, private float4 * iptr);

_CLC_OVERLOAD _CLC_DECL float8 modf(float8 x, global  float8 * iptr);
_CLC_OVERLOAD _CLC_DECL float8 modf(float8 x, local   float8 * iptr);
_CLC_OVERLOAD _CLC_DECL float8 modf(float8 x, private float8 * iptr);

_CLC_OVERLOAD _CLC_DECL float16 modf(float16 x, global  float16 * iptr);
_CLC_OVERLOAD _CLC_DECL float16 modf(float16 x, local   float16 * iptr);
_CLC_OVERLOAD _CLC_DECL float16 modf(float16 x, private float16 * iptr);

_CLC_OVERLOAD _CLC_DECL double modf(double x, global  double * iptr);
_CLC_OVERLOAD _CLC_DECL double modf(double x, local   double * iptr);
_CLC_OVERLOAD _CLC_DECL double modf(double x, private double * iptr);

_CLC_OVERLOAD _CLC_DECL double2 modf(double2 x, global  double2 * iptr);
_CLC_OVERLOAD _CLC_DECL double2 modf(double2 x, local   double2 * iptr);
_CLC_OVERLOAD _CLC_DECL double2 modf(double2 x, private double2 * iptr);

_CLC_OVERLOAD _CLC_DECL double3 modf(double3 x, global  double3 * iptr);
_CLC_OVERLOAD _CLC_DECL double3 modf(double3 x, local   double3 * iptr);
_CLC_OVERLOAD _CLC_DECL double3 modf(double3 x, private double3 * iptr);

_CLC_OVERLOAD _CLC_DECL double4 modf(double4 x, global  double4 * iptr);
_CLC_OVERLOAD _CLC_DECL double4 modf(double4 x, local   double4 * iptr);
_CLC_OVERLOAD _CLC_DECL double4 modf(double4 x, private double4 * iptr);

_CLC_OVERLOAD _CLC_DECL double8 modf(double8 x, global  double8 * iptr);
_CLC_OVERLOAD _CLC_DECL double8 modf(double8 x, local   double8 * iptr);
_CLC_OVERLOAD _CLC_DECL double8 modf(double8 x, private double8 * iptr);

_CLC_OVERLOAD _CLC_DECL double16 modf(double16 x, global  double16 * iptr);
_CLC_OVERLOAD _CLC_DECL double16 modf(double16 x, local   double16 * iptr);
_CLC_OVERLOAD _CLC_DECL double16 modf(double16 x, private double16 * iptr);

_CLC_OVERLOAD _CLC_DECL float frexp(float x, global  int * ptr);
_CLC_OVERLOAD _CLC_DECL float frexp(float x, local   int * ptr);
_CLC_OVERLOAD _CLC_DECL float frexp(float x, private int * ptr);

_CLC_OVERLOAD _CLC_DECL float2 frexp(float2 x, global  int2 * ptr);
_CLC_OVERLOAD _CLC_DECL float2 frexp(float2 x, local   int2 * ptr);
_CLC_OVERLOAD _CLC_DECL float2 frexp(float2 x, private int2 * ptr);

_CLC_OVERLOAD _CLC_DECL float3 frexp(float3 x, global  int3 * ptr);
_CLC_OVERLOAD _CLC_DECL float3 frexp(float3 x, local   int3 * ptr);
_CLC_OVERLOAD _CLC_DECL float3 frexp(float3 x, private int3 * ptr);

_CLC_OVERLOAD _CLC_DECL float4 frexp(float4 x, global  int4 * ptr);
_CLC_OVERLOAD _CLC_DECL float4 frexp(float4 x, local   int4 * ptr);
_CLC_OVERLOAD _CLC_DECL float4 frexp(float4 x, private int4 * ptr);

_CLC_OVERLOAD _CLC_DECL float8 frexp(float8 x, global  int8 * ptr);
_CLC_OVERLOAD _CLC_DECL float8 frexp(float8 x, local   int8 * ptr);
_CLC_OVERLOAD _CLC_DECL float8 frexp(float8 x, private int8 * ptr);

_CLC_OVERLOAD _CLC_DECL float16 frexp(float16 x, global  int16 * ptr);
_CLC_OVERLOAD _CLC_DECL float16 frexp(float16 x, local   int16 * ptr);
_CLC_OVERLOAD _CLC_DECL float16 frexp(float16 x, private int16 * ptr);

_CLC_OVERLOAD _CLC_DECL double frexp(double x, global  int * ptr);
_CLC_OVERLOAD _CLC_DECL double frexp(double x, local   int * ptr);
_CLC_OVERLOAD _CLC_DECL double frexp(double x, private int * ptr);

_CLC_OVERLOAD _CLC_DECL double2 frexp(double2 x, global  int2 * ptr);
_CLC_OVERLOAD _CLC_DECL double2 frexp(double2 x, local   int2 * ptr);
_CLC_OVERLOAD _CLC_DECL double2 frexp(double2 x, private int2 * ptr);

_CLC_OVERLOAD _CLC_DECL double3 frexp(double3 x, global  int3 * ptr);
_CLC_OVERLOAD _CLC_DECL double3 frexp(double3 x, local   int3 * ptr);
_CLC_OVERLOAD _CLC_DECL double3 frexp(double3 x, private int3 * ptr);

_CLC_OVERLOAD _CLC_DECL double4 frexp(double4 x, global  int4 * ptr);
_CLC_OVERLOAD _CLC_DECL double4 frexp(double4 x, local   int4 * ptr);
_CLC_OVERLOAD _CLC_DECL double4 frexp(double4 x, private int4 * ptr);

_CLC_OVERLOAD _CLC_DECL double8 frexp(double8 x, global  int8 * ptr);
_CLC_OVERLOAD _CLC_DECL double8 frexp(double8 x, local   int8 * ptr);
_CLC_OVERLOAD _CLC_DECL double8 frexp(double8 x, private int8 * ptr);

_CLC_OVERLOAD _CLC_DECL double16 frexp(double16 x, global  int16 * ptr);
_CLC_OVERLOAD _CLC_DECL double16 frexp(double16 x, local   int16 * ptr);
_CLC_OVERLOAD _CLC_DECL double16 frexp(double16 x, private int16 * ptr);

_CLC_OVERLOAD _CLC_DECL float lgamma_r(float x, global  int * ptr);
_CLC_OVERLOAD _CLC_DECL float lgamma_r(float x, local   int * ptr);
_CLC_OVERLOAD _CLC_DECL float lgamma_r(float x, private int * ptr);

_CLC_OVERLOAD _CLC_DECL float2 lgamma_r(float2 x, global  int2 * ptr);
_CLC_OVERLOAD _CLC_DECL float2 lgamma_r(float2 x, local   int2 * ptr);
_CLC_OVERLOAD _CLC_DECL float2 lgamma_r(float2 x, private int2 * ptr);

_CLC_OVERLOAD _CLC_DECL float3 lgamma_r(float3 x, global  int3 * ptr);
_CLC_OVERLOAD _CLC_DECL float3 lgamma_r(float3 x, local   int3 * ptr);
_CLC_OVERLOAD _CLC_DECL float3 lgamma_r(float3 x, private int3 * ptr);

_CLC_OVERLOAD _CLC_DECL float4 lgamma_r(float4 x, global  int4 * ptr);
_CLC_OVERLOAD _CLC_DECL float4 lgamma_r(float4 x, local   int4 * ptr);
_CLC_OVERLOAD _CLC_DECL float4 lgamma_r(float4 x, private int4 * ptr);

_CLC_OVERLOAD _CLC_DECL float8 lgamma_r(float8 x, global  int8 * ptr);
_CLC_OVERLOAD _CLC_DECL float8 lgamma_r(float8 x, local   int8 * ptr);
_CLC_OVERLOAD _CLC_DECL float8 lgamma_r(float8 x, private int8 * ptr);

_CLC_OVERLOAD _CLC_DECL float16 lgamma_r(float16 x, global  int16 * ptr);
_CLC_OVERLOAD _CLC_DECL float16 lgamma_r(float16 x, local   int16 * ptr);
_CLC_OVERLOAD _CLC_DECL float16 lgamma_r(float16 x, private int16 * ptr);

_CLC_OVERLOAD _CLC_DECL double lgamma_r(double x, global  int * ptr);
_CLC_OVERLOAD _CLC_DECL double lgamma_r(double x, local   int * ptr);
_CLC_OVERLOAD _CLC_DECL double lgamma_r(double x, private int * ptr);

_CLC_OVERLOAD _CLC_DECL double2 lgamma_r(double2 x, global  int2 * ptr);
_CLC_OVERLOAD _CLC_DECL double2 lgamma_r(double2 x, local   int2 * ptr);
_CLC_OVERLOAD _CLC_DECL double2 lgamma_r(double2 x, private int2 * ptr);

_CLC_OVERLOAD _CLC_DECL double3 lgamma_r(double3 x, global  int3 * ptr);
_CLC_OVERLOAD _CLC_DECL double3 lgamma_r(double3 x, local   int3 * ptr);
_CLC_OVERLOAD _CLC_DECL double3 lgamma_r(double3 x, private int3 * ptr);

_CLC_OVERLOAD _CLC_DECL double4 lgamma_r(double4 x, global  int4 * ptr);
_CLC_OVERLOAD _CLC_DECL double4 lgamma_r(double4 x, local   int4 * ptr);
_CLC_OVERLOAD _CLC_DECL double4 lgamma_r(double4 x, private int4 * ptr);

_CLC_OVERLOAD _CLC_DECL double8 lgamma_r(double8 x, global  int8 * ptr);
_CLC_OVERLOAD _CLC_DECL double8 lgamma_r(double8 x, local   int8 * ptr);
_CLC_OVERLOAD _CLC_DECL double8 lgamma_r(double8 x, private int8 * ptr);

_CLC_OVERLOAD _CLC_DECL double16 lgamma_r(double16 x, global  int16 * ptr);
_CLC_OVERLOAD _CLC_DECL double16 lgamma_r(double16 x, local   int16 * ptr);
_CLC_OVERLOAD _CLC_DECL double16 lgamma_r(double16 x, private int16 * ptr);


_CLC_OVERLOAD _CLC_DECL float fract(float x, global  float * ptr);
_CLC_OVERLOAD _CLC_DECL float fract(float x, local   float * ptr);
_CLC_OVERLOAD _CLC_DECL float fract(float x, private float * ptr);

_CLC_OVERLOAD _CLC_DECL float2 fract(float2 x, global  float2 * ptr);
_CLC_OVERLOAD _CLC_DECL float2 fract(float2 x, local   float2 * ptr);
_CLC_OVERLOAD _CLC_DECL float2 fract(float2 x, private float2 * ptr);

_CLC_OVERLOAD _CLC_DECL float3 fract(float3 x, global  float3 * ptr);
_CLC_OVERLOAD _CLC_DECL float3 fract(float3 x, local   float3 * ptr);
_CLC_OVERLOAD _CLC_DECL float3 fract(float3 x, private float3 * ptr);

_CLC_OVERLOAD _CLC_DECL float4 fract(float4 x, global  float4 * ptr);
_CLC_OVERLOAD _CLC_DECL float4 fract(float4 x, local   float4 * ptr);
_CLC_OVERLOAD _CLC_DECL float4 fract(float4 x, private float4 * ptr);

_CLC_OVERLOAD _CLC_DECL float8 fract(float8 x, global  float8 * ptr);
_CLC_OVERLOAD _CLC_DECL float8 fract(float8 x, local   float8 * ptr);
_CLC_OVERLOAD _CLC_DECL float8 fract(float8 x, private float8 * ptr);

_CLC_OVERLOAD _CLC_DECL float16 fract(float16 x, global  float16 * ptr);
_CLC_OVERLOAD _CLC_DECL float16 fract(float16 x, local   float16 * ptr);
_CLC_OVERLOAD _CLC_DECL float16 fract(float16 x, private float16 * ptr);

_CLC_OVERLOAD _CLC_DECL double fract(double x, global  double * ptr);
_CLC_OVERLOAD _CLC_DECL double fract(double x, local   double * ptr);
_CLC_OVERLOAD _CLC_DECL double fract(double x, private double * ptr);

_CLC_OVERLOAD _CLC_DECL double2 fract(double2 x, global  double2 * ptr);
_CLC_OVERLOAD _CLC_DECL double2 fract(double2 x, local   double2 * ptr);
_CLC_OVERLOAD _CLC_DECL double2 fract(double2 x, private double2 * ptr);

_CLC_OVERLOAD _CLC_DECL double3 fract(double3 x, global  double3 * ptr);
_CLC_OVERLOAD _CLC_DECL double3 fract(double3 x, local   double3 * ptr);
_CLC_OVERLOAD _CLC_DECL double3 fract(double3 x, private double3 * ptr);

_CLC_OVERLOAD _CLC_DECL double4 fract(double4 x, global  double4 * ptr);
_CLC_OVERLOAD _CLC_DECL double4 fract(double4 x, local   double4 * ptr);
_CLC_OVERLOAD _CLC_DECL double4 fract(double4 x, private double4 * ptr);

_CLC_OVERLOAD _CLC_DECL double8 fract(double8 x, global  double8 * ptr);
_CLC_OVERLOAD _CLC_DECL double8 fract(double8 x, local   double8 * ptr);
_CLC_OVERLOAD _CLC_DECL double8 fract(double8 x, private double8 * ptr);

_CLC_OVERLOAD _CLC_DECL double16 fract(double16 x, global  double16 * ptr);
_CLC_OVERLOAD _CLC_DECL double16 fract(double16 x, local   double16 * ptr);
_CLC_OVERLOAD _CLC_DECL double16 fract(double16 x, private double16 * ptr);

_CLC_PROTECTED float remquof(float x, float y, int *ptr);

_CLC_OVERLOAD _CLC_INLINE float remquo(float x, float y, global  int * quo) 
    { return remquof(x, y, (int*)quo); }

_CLC_OVERLOAD _CLC_INLINE float remquo(float x, float y, local   int * quo) 
    { return remquof(x, y, (int*)quo); }

_CLC_OVERLOAD _CLC_INLINE float remquo(float x, float y, private int * quo) 
    { return remquof(x, y, (int*)quo); }

_CLC_OVERLOAD _CLC_DECL float2 remquo(float2 x, float2 y, global  int2 * quo);
_CLC_OVERLOAD _CLC_DECL float2 remquo(float2 x, float2 y, local   int2 * quo);
_CLC_OVERLOAD _CLC_DECL float2 remquo(float2 x, float2 y, private int2 * quo);

_CLC_OVERLOAD _CLC_DECL float3 remquo(float3 x, float3 y, global  int3 * quo);
_CLC_OVERLOAD _CLC_DECL float3 remquo(float3 x, float3 y, local   int3 * quo);
_CLC_OVERLOAD _CLC_DECL float3 remquo(float3 x, float3 y, private int3 * quo);

_CLC_OVERLOAD _CLC_DECL float4 remquo(float4 x, float4 y, global  int4 * quo);
_CLC_OVERLOAD _CLC_DECL float4 remquo(float4 x, float4 y, local   int4 * quo);
_CLC_OVERLOAD _CLC_DECL float4 remquo(float4 x, float4 y, private int4 * quo);

_CLC_OVERLOAD _CLC_DECL float8 remquo(float8 x, float8 y, global  int8 * quo);
_CLC_OVERLOAD _CLC_DECL float8 remquo(float8 x, float8 y, local   int8 * quo);
_CLC_OVERLOAD _CLC_DECL float8 remquo(float8 x, float8 y, private int8 * quo);

_CLC_OVERLOAD _CLC_DECL float16 remquo(float16 x, float16 y, global  int16 * quo);
_CLC_OVERLOAD _CLC_DECL float16 remquo(float16 x, float16 y, local   int16 * quo);
_CLC_OVERLOAD _CLC_DECL float16 remquo(float16 x, float16 y, private int16 * quo);

_CLC_PROTECTED double remquod(double x, double y, int *ptr);

_CLC_OVERLOAD _CLC_INLINE double remquo(double x, double y, global  int * quo) 
    { return remquod(x, y, (int*)quo); }

_CLC_OVERLOAD _CLC_INLINE double remquo(double x, double y, local   int * quo) 
    { return remquod(x, y, (int*)quo); }

_CLC_OVERLOAD _CLC_INLINE double remquo(double x, double y, private int * quo) 
    { return remquod(x, y, (int*)quo); }

_CLC_OVERLOAD _CLC_DECL double2 remquo(double2 x, double2 y, global  int2 * quo);
_CLC_OVERLOAD _CLC_DECL double2 remquo(double2 x, double2 y, local   int2 * quo);
_CLC_OVERLOAD _CLC_DECL double2 remquo(double2 x, double2 y, private int2 * quo);

_CLC_OVERLOAD _CLC_DECL double3 remquo(double3 x, double3 y, global  int3 * quo);
_CLC_OVERLOAD _CLC_DECL double3 remquo(double3 x, double3 y, local   int3 * quo);
_CLC_OVERLOAD _CLC_DECL double3 remquo(double3 x, double3 y, private int3 * quo);

_CLC_OVERLOAD _CLC_DECL double4 remquo(double4 x, double4 y, global  int4 * quo);
_CLC_OVERLOAD _CLC_DECL double4 remquo(double4 x, double4 y, local   int4 * quo);
_CLC_OVERLOAD _CLC_DECL double4 remquo(double4 x, double4 y, private int4 * quo);

_CLC_OVERLOAD _CLC_DECL double8 remquo(double8 x, double8 y, global  int8 * quo);
_CLC_OVERLOAD _CLC_DECL double8 remquo(double8 x, double8 y, local   int8 * quo);
_CLC_OVERLOAD _CLC_DECL double8 remquo(double8 x, double8 y, private int8 * quo);

_CLC_OVERLOAD _CLC_DECL double16 remquo(double16 x, double16 y, global  int16 * quo);
_CLC_OVERLOAD _CLC_DECL double16 remquo(double16 x, double16 y, local   int16 * quo);
_CLC_OVERLOAD _CLC_DECL double16 remquo(double16 x, double16 y, private int16 * quo);

_CLC_PROTECTED void sincosf(float x, float * sinval, float * cosval);

_CLC_OVERLOAD _CLC_INLINE float sincos(float x, global  float * cosval) 
    {   float sinval; 
        sincosf(x, (float*)&sinval, (float*)cosval); 
        return sinval;
    }

_CLC_OVERLOAD _CLC_INLINE float sincos(float x, local   float * cosval) 
    {   float sinval; 
        sincosf(x, (float*)&sinval, (float*)cosval); 
        return sinval;
    }

_CLC_OVERLOAD _CLC_INLINE float sincos(float x, private float * cosval) 
    {   float sinval; 
        sincosf(x, (float*)&sinval, (float*)cosval); 
        return sinval;
    }

_CLC_OVERLOAD _CLC_DECL float2 sincos(float2 x, global  float2 * cosval);
_CLC_OVERLOAD _CLC_DECL float2 sincos(float2 x, local   float2 * cosval);
_CLC_OVERLOAD _CLC_DECL float2 sincos(float2 x, private float2 * cosval);

_CLC_OVERLOAD _CLC_DECL float3 sincos(float3 x, global  float3 * cosval);
_CLC_OVERLOAD _CLC_DECL float3 sincos(float3 x, local   float3 * cosval);
_CLC_OVERLOAD _CLC_DECL float3 sincos(float3 x, private float3 * cosval);

_CLC_OVERLOAD _CLC_DECL float4 sincos(float4 x, global  float4 * cosval);
_CLC_OVERLOAD _CLC_DECL float4 sincos(float4 x, local   float4 * cosval);
_CLC_OVERLOAD _CLC_DECL float4 sincos(float4 x, private float4 * cosval);

_CLC_OVERLOAD _CLC_DECL float8 sincos(float8 x, global  float8 * cosval);
_CLC_OVERLOAD _CLC_DECL float8 sincos(float8 x, local   float8 * cosval);
_CLC_OVERLOAD _CLC_DECL float8 sincos(float8 x, private float8 * cosval);

_CLC_OVERLOAD _CLC_DECL float16 sincos(float16 x, global  float16 * cosval);
_CLC_OVERLOAD _CLC_DECL float16 sincos(float16 x, local   float16 * cosval);
_CLC_OVERLOAD _CLC_DECL float16 sincos(float16 x, private float16 * cosval);

_CLC_PROTECTED void sincosd(double x, double * sinval, double * cosval);

_CLC_OVERLOAD _CLC_INLINE double sincos(double x, global  double * cosval) 
    {   double sinval; 
        sincosd(x, (double*)&sinval, (double*)cosval); 
        return sinval;
    }

_CLC_OVERLOAD _CLC_INLINE double sincos(double x, local   double * cosval) 
    {   double sinval; 
        sincosd(x, (double*)&sinval, (double*)cosval); 
        return sinval;
    }

_CLC_OVERLOAD _CLC_INLINE double sincos(double x, private double * cosval) 
    {   double sinval; 
        sincosd(x, (double*)&sinval, (double*)cosval); 
        return sinval;
    }

_CLC_OVERLOAD _CLC_DECL double2 sincos(double2 x, global  double2 * cosval);
_CLC_OVERLOAD _CLC_DECL double2 sincos(double2 x, local   double2 * cosval);
_CLC_OVERLOAD _CLC_DECL double2 sincos(double2 x, private double2 * cosval);

_CLC_OVERLOAD _CLC_DECL double3 sincos(double3 x, global  double3 * cosval);
_CLC_OVERLOAD _CLC_DECL double3 sincos(double3 x, local   double3 * cosval);
_CLC_OVERLOAD _CLC_DECL double3 sincos(double3 x, private double3 * cosval);

_CLC_OVERLOAD _CLC_DECL double4 sincos(double4 x, global  double4 * cosval);
_CLC_OVERLOAD _CLC_DECL double4 sincos(double4 x, local   double4 * cosval);
_CLC_OVERLOAD _CLC_DECL double4 sincos(double4 x, private double4 * cosval);

_CLC_OVERLOAD _CLC_DECL double8 sincos(double8 x, global  double8 * cosval);
_CLC_OVERLOAD _CLC_DECL double8 sincos(double8 x, local   double8 * cosval);
_CLC_OVERLOAD _CLC_DECL double8 sincos(double8 x, private double8 * cosval);

_CLC_OVERLOAD _CLC_DECL double16 sincos(double16 x, global  double16 * cosval);
_CLC_OVERLOAD _CLC_DECL double16 sincos(double16 x, local   double16 * cosval);
_CLC_OVERLOAD _CLC_DECL double16 sincos(double16 x, private double16 * cosval);

/*-----------------------------------------------------------------------------
* Integer
*----------------------------------------------------------------------------*/
#define EXPAND_SIZES(type) \
    SCALAR(type)              \
    TEMPLATE(_VEC_TYPE(type,2))  \
    TEMPLATE(_VEC_TYPE(type,3))  \
    TEMPLATE(_VEC_TYPE(type,4))  \
    TEMPLATE(_VEC_TYPE(type,8))  \
    TEMPLATE(_VEC_TYPE(type,16)) \

#define TEMPLATE(gentype) \
    _CLC_OVERLOAD _CLC_DECL gentype hadd(gentype x1, gentype x2);\
    _CLC_OVERLOAD _CLC_DECL gentype rhadd(gentype x1, gentype x2);\

#define SCALAR(gentype) \
    _CLC_OVERLOAD _CLC_INLINE gentype hadd(gentype x, gentype y) \
    { return (x >> 1) + (y >> 1) + (x & y & 1); } \
    _CLC_OVERLOAD _CLC_INLINE gentype rhadd(gentype x, gentype y) \
    { return (x >> 1) + (y >> 1) + ((x&1)|(y&1)); } \

_EXPAND_INTEGER_TYPES()

#undef EXPAND_SIZES
#undef SCALAR
#undef TEMPLATE

#define EXPAND_SIZES(type)             \
    SCALAR_IMPLEMENTATION(type)              \
    DECLARATION(_VEC_TYPE(type,2), type)  \
    DECLARATION(_VEC_TYPE(type,3), type)  \
    DECLARATION(_VEC_TYPE(type,4), type)  \
    DECLARATION(_VEC_TYPE(type,8), type)  \
    DECLARATION(_VEC_TYPE(type,16), type) \

#define DECLARATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_DECL gentype clamp(gentype x, gentype minval, gentype maxval);  \
_CLC_OVERLOAD _CLC_DECL gentype clamp(gentype x, sgentype minval, sgentype maxval); \

#define SCALAR_IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype clamp(gentype x, gentype minval, gentype maxval)  \
    { return x > maxval ? maxval : x < minval ? minval : x; } \

_EXPAND_TYPES()

#undef EXPAND_SIZES
#undef IMPLEMENTATION
#undef DECLARATION
#undef SCALAR_IMPLEMENTATION

#define EXPAND_SIZES(type)                  \
    SCALAR_IMPLEMENTATION(type)             \
    IMPLEMENTATION(_VEC_TYPE(type,2), type)  \
    DECLARATION(_VEC_TYPE(type,3), type)  \
    DECLARATION(_VEC_TYPE(type,4), type)  \
    DECLARATION(_VEC_TYPE(type,8), type)  \
    DECLARATION(_VEC_TYPE(type,16), type) \

#define DECLARATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_DECL gentype min(gentype x, gentype y); \
_CLC_OVERLOAD _CLC_DECL gentype min(gentype x, sgentype y); \
_CLC_OVERLOAD _CLC_DECL gentype max(gentype x, gentype y); \
_CLC_OVERLOAD _CLC_DECL gentype max(gentype x, sgentype y); \

#define IMPLEMENTATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_INLINE gentype min(gentype x, gentype y)  \
    { return y < x ? y : x; } \
_CLC_OVERLOAD _CLC_INLINE gentype min(gentype x, sgentype y) \
    { return (gentype)y < x ? (gentype)y : x; } \
_CLC_OVERLOAD _CLC_INLINE gentype max(gentype x, gentype y)  \
    { return y > x ? y : x; } \
_CLC_OVERLOAD _CLC_INLINE gentype max(gentype x, sgentype y) \
    { return (gentype)y > x ? (gentype)y : x; } \

#define SCALAR_IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype min(gentype x, gentype y)  \
    { return y < x ? y : x; } \
_CLC_OVERLOAD _CLC_INLINE gentype max(gentype x, gentype y)  \
    { return y > x ? y : x; } \

_EXPAND_TYPES()

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION
#undef SCALAR_IMPLEMENTATION

#define EXPAND_SIZES(type)                  \
    SCALAR_IMPLEMENTATION(type)             \
    IMPLEMENTATION(_VEC_TYPE(type,2), type)  \
    DECLARATION(_VEC_TYPE(type,3), type)  \
    DECLARATION(_VEC_TYPE(type,4), type)  \
    DECLARATION(_VEC_TYPE(type,8), type)  \
    DECLARATION(_VEC_TYPE(type,16), type) \

#define DECLARATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_DECL gentype mix(gentype x, gentype y, gentype a); \
_CLC_OVERLOAD _CLC_DECL gentype mix(gentype x, gentype y, sgentype a); \

#define IMPLEMENTATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_INLINE gentype mix(gentype x, gentype y, gentype a)  \
    { return x + (y-x) * a; } \
_CLC_OVERLOAD _CLC_INLINE gentype mix(gentype x, gentype y, sgentype a) \
    { return x + (y-x) * (gentype)a; } \

#define SCALAR_IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype mix(gentype x, gentype y, gentype a)  \
    { return x + (y-x) * a; } \

EXPAND_SIZES(float)
EXPAND_SIZES(double)

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION
#undef SCALAR_IMPLEMENTATION

#define EXPAND_SIZES(type, utype) \
    TEMPLATE(_VEC_TYPE(type,2), _VEC_TYPE(utype,2))  \
    TEMPLATE(_VEC_TYPE(type,3), _VEC_TYPE(utype,3))  \
    TEMPLATE(_VEC_TYPE(type,4), _VEC_TYPE(utype,4))  \
    TEMPLATE(_VEC_TYPE(type,8), _VEC_TYPE(utype,8))  \
    TEMPLATE(_VEC_TYPE(type,16), _VEC_TYPE(utype,16)) \

#define TEMPLATE(gentype, ugentype) \
    _CLC_OVERLOAD _CLC_DECL ugentype abs_diff(gentype x, gentype y);\

EXPAND_SIZES(char, uchar)
EXPAND_SIZES(uchar, uchar)
EXPAND_SIZES(short, ushort)
EXPAND_SIZES(ushort, ushort)
EXPAND_SIZES(int, uint)
EXPAND_SIZES(uint, uint)
EXPAND_SIZES(long, ulong)
EXPAND_SIZES(ulong, ulong)

_CLC_OVERLOAD _CLC_INLINE uchar  abs_diff (char x,   char y)   { return x>y ? x-y : y-x; }
_CLC_OVERLOAD _CLC_INLINE uchar  abs_diff (uchar x,  uchar y)  { return x>y ? x-y : y-x; }
_CLC_OVERLOAD _CLC_INLINE ushort abs_diff (short x,  short y)  { return x>y ? x-y : y-x; }
_CLC_OVERLOAD _CLC_INLINE ushort abs_diff (ushort x, ushort y) { return x>y ? x-y : y-x; }
_CLC_OVERLOAD _CLC_INLINE uint   abs_diff (uint x,   uint y)   { return x>y ? x-y : y-x; }
_CLC_OVERLOAD _CLC_INLINE ulong  abs_diff (ulong x,  ulong y)  { return x>y ? x-y : y-x; }

_CLC_OVERLOAD _CLC_DECL uint abs_diff(int x,  int y);
_CLC_OVERLOAD _CLC_DECL ulong abs_diff(long x,  long y);

#undef EXPAND_SIZES
#undef TEMPLATE

#define mad_hi(a, b, c) (mul_hi((a),(b))+(c))
#define mul24(a, b)     ((a)*(b))
#define mad24(a, b, c)  (((a)*(b))+(c))

/*-----------------------------------------------------------------------------
* Common
*----------------------------------------------------------------------------*/
#define EXPAND_SIZES(type) \
    IMPLEMENTATION(type)              \
    IMPLEMENTATION(_VEC_TYPE(type,2))  \
    DECLARATION(_VEC_TYPE(type,3))  \
    DECLARATION(_VEC_TYPE(type,4))  \
    DECLARATION(_VEC_TYPE(type,8))  \
    DECLARATION(_VEC_TYPE(type,16)) \

#define DECLARATION(gentype) \
_CLC_OVERLOAD _CLC_DECL gentype degrees(gentype radians); \
_CLC_OVERLOAD _CLC_DECL gentype radians(gentype degrees); \

#define IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype degrees(gentype radians) { return radians * (gentype)180.0 * (gentype)M_1_PI; } \
_CLC_OVERLOAD _CLC_INLINE gentype radians(gentype degrees) { return degrees * (gentype)M_PI / (gentype)180.0; } 

EXPAND_SIZES(float)
EXPAND_SIZES(double)

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION

#define EXPAND_SIZES(type)                  \
    SCALAR_IMPLEMENTATION(type)             \
    IMPLEMENTATION(_VEC_TYPE(type,2), type)  \
    DECLARATION(_VEC_TYPE(type,3), type)  \
    DECLARATION(_VEC_TYPE(type,4), type)  \
    DECLARATION(_VEC_TYPE(type,8), type)  \
    DECLARATION(_VEC_TYPE(type,16), type) \

#define DECLARATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_DECL gentype step(gentype edge, gentype x); \
_CLC_OVERLOAD _CLC_DECL gentype step(sgentype edge, gentype x); \

#define IMPLEMENTATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_INLINE gentype step(gentype edge, gentype x)  \
    { return x < edge ? (gentype)0.0 : (gentype)1.0 ; } \
_CLC_OVERLOAD _CLC_INLINE gentype step(sgentype edge, gentype x) \
    { return x < (gentype)edge ? (gentype)0.0 : (gentype)1.0 ; } \

#define SCALAR_IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype step(gentype edge, gentype x)  \
    { return x < edge ? 0.0 : 1.0 ; } \

EXPAND_SIZES(float)
EXPAND_SIZES(double)

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION
#undef SCALAR_IMPLEMENTATION

_CLC_OVERLOAD _CLC_DECL float   smoothstep(float edge0, float edge1, float x);
_CLC_OVERLOAD _CLC_DECL float2  smoothstep(float2  edge0, float2  edge1,  
                                           float2  x);
_CLC_OVERLOAD _CLC_DECL float3  smoothstep(float3  edge0, float3  edge1,  
                                           float3  x);
_CLC_OVERLOAD _CLC_DECL float4  smoothstep(float4  edge0, float4  edge1,  
                                           float4  x);
_CLC_OVERLOAD _CLC_DECL float8  smoothstep(float8  edge0, float8  edge1,  
                                           float8  x);
_CLC_OVERLOAD _CLC_DECL float16 smoothstep(float16 edge0, float16 edge1,  
                                           float16 x);

_CLC_OVERLOAD _CLC_DECL float2  smoothstep(float edge0, float edge1, float2  x);
_CLC_OVERLOAD _CLC_DECL float3  smoothstep(float edge0, float edge1, float3  x);
_CLC_OVERLOAD _CLC_DECL float4  smoothstep(float edge0, float edge1, float4  x);
_CLC_OVERLOAD _CLC_DECL float8  smoothstep(float edge0, float edge1, float8  x);
_CLC_OVERLOAD _CLC_DECL float16 smoothstep(float edge0, float edge1, float16 x);

_CLC_OVERLOAD _CLC_DECL double   smoothstep(double edge0, double edge1, double x);
_CLC_OVERLOAD _CLC_DECL double2  smoothstep(double2  edge0, double2  edge1,  
                                            double2  x);
_CLC_OVERLOAD _CLC_DECL double3  smoothstep(double3  edge0, double3  edge1,  
                                            double3  x);
_CLC_OVERLOAD _CLC_DECL double4  smoothstep(double4  edge0, double4  edge1,  
                                            double4  x);
_CLC_OVERLOAD _CLC_DECL double8  smoothstep(double8  edge0, double8  edge1,  
                                            double8  x);
_CLC_OVERLOAD _CLC_DECL double16 smoothstep(double16 edge0, double16 edge1,  
                                            double16 x);

_CLC_OVERLOAD _CLC_DECL double2  smoothstep(double edge0, double edge1, 
                                            double2  x);
_CLC_OVERLOAD _CLC_DECL double3  smoothstep(double edge0, double edge1, 
                                            double3  x);
_CLC_OVERLOAD _CLC_DECL double4  smoothstep(double edge0, double edge1, 
                                            double4  x);
_CLC_OVERLOAD _CLC_DECL double8  smoothstep(double edge0, double edge1, 
                                            double8  x);
_CLC_OVERLOAD _CLC_DECL double16 smoothstep(double edge0, double edge1, 
                                            double16 x);

#define EXPAND_SIZES(type)            \
    IMPLEMENTATION(type)              \
    IMPLEMENTATION(_VEC_TYPE(type,2)) \
    DECLARATION(_VEC_TYPE(type,3))  \
    DECLARATION(_VEC_TYPE(type,4))  \
    DECLARATION(_VEC_TYPE(type,8))  \
    DECLARATION(_VEC_TYPE(type,16)) \

#define DECLARATION(gentype) \
_CLC_OVERLOAD _CLC_DECL gentype sign(gentype x); \

#define IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype sign(gentype x) \
{  return x > (gentype)0.0 ? (gentype) 1.0 : \
          x < (gentype)0.0 ? (gentype)-1.0 : \
          isnan(x)         ? (gentype) 0.0 : x; } \

EXPAND_SIZES(float)
EXPAND_SIZES(double)

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION

/*-----------------------------------------------------------------------------
* Geometric
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_INLINE float  dot(float p0, float p1)   {return p0*p1;}
_CLC_OVERLOAD _CLC_INLINE float  dot(float2 p0, float2 p1) {return p0.x*p1.x+p0.y*p1.y;}
_CLC_OVERLOAD _CLC_DECL   float  dot(float3 p0, float3 p1);
_CLC_OVERLOAD _CLC_DECL   float  dot(float4 p0, float4 p1);
_CLC_OVERLOAD _CLC_INLINE double dot(double p0, double p1)   {return p0*p1;}
_CLC_OVERLOAD _CLC_INLINE double dot(double2 p0, double2 p1) {return p0.x*p1.x+p0.y*p1.y;}
_CLC_OVERLOAD _CLC_DECL   double dot(double3 p0, double3 p1) ;
_CLC_OVERLOAD _CLC_DECL   double dot(double4 p0, double4 p1) ;

_CLC_OVERLOAD _CLC_DECL float3  cross(float3 p0, float3 p1);
_CLC_OVERLOAD _CLC_DECL float4  cross(float4 p0, float4 p1);
_CLC_OVERLOAD _CLC_DECL double3 cross(double3 p0, double3 p1);
_CLC_OVERLOAD _CLC_DECL double4 cross(double4 p0, double4 p1);

_CLC_OVERLOAD _CLC_INLINE float  length(float p)         {return fabs(p);}
_CLC_OVERLOAD _CLC_INLINE double length(double p)        {return fabs(p);}
_CLC_OVERLOAD _CLC_INLINE float  fast_length(float p)    {return fabs(p);}
_CLC_OVERLOAD _CLC_INLINE double fast_length(double p)   {return fabs(p);}

_CLC_OVERLOAD _CLC_DECL   float  length(float2 p);
_CLC_OVERLOAD _CLC_DECL   float  length(float3 p);
_CLC_OVERLOAD _CLC_DECL   float  length(float4 p);
_CLC_OVERLOAD _CLC_DECL   double length(double2 p);
_CLC_OVERLOAD _CLC_DECL   double length(double3 p);
_CLC_OVERLOAD _CLC_DECL   double length(double4 p);

_CLC_OVERLOAD _CLC_DECL   float  fast_length(float2 p);
_CLC_OVERLOAD _CLC_DECL   float  fast_length(float3 p);
_CLC_OVERLOAD _CLC_DECL   float  fast_length(float4 p);
_CLC_OVERLOAD _CLC_DECL   double fast_length(double2 p);
_CLC_OVERLOAD _CLC_DECL   double fast_length(double3 p);
_CLC_OVERLOAD _CLC_DECL   double fast_length(double4 p);

_CLC_OVERLOAD _CLC_INLINE float  distance(float p0,   float p1)    { return fabs(p1-p0);}
_CLC_OVERLOAD _CLC_INLINE float  distance(float2 p0,  float2 p1)   { return length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE float  distance(float3 p0,  float3 p1)   { return length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE float  distance(float4 p0,  float4 p1)   { return length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE double distance(double p0,  double p1)   { return fabs(p1-p0);}
_CLC_OVERLOAD _CLC_INLINE double distance(double2 p0, double2 p1)  { return length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE double distance(double3 p0, double3 p1)  { return length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE double distance(double4 p0, double4 p1)  { return length(p1-p0); }

_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float p0,   float p1)    { return fabs(p1-p0);}
_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float2 p0,  float2 p1)   { return fast_length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float3 p0,  float3 p1)   { return fast_length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE float  fast_distance(float4 p0,  float4 p1)   { return fast_length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double p0,  double p1)   { return fabs(p1-p0);}
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double2 p0, double2 p1)  { return fast_length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double3 p0, double3 p1)  { return fast_length(p1-p0); }
_CLC_OVERLOAD _CLC_INLINE double fast_distance(double4 p0, double4 p1)  { return fast_length(p1-p0); }

_CLC_OVERLOAD _CLC_INLINE float  normalize(float p)        
{return p > 0.0f ? 1.0f : p < 0.0f ? -1.0f : 0.0f;}

_CLC_OVERLOAD _CLC_INLINE double normalize(double p)       
{return p > 0.0 ? 1.0 : p < 0.0 ? -1.0 : 0.0;}

_CLC_OVERLOAD _CLC_INLINE float  fast_normalize(float p)   
{return p > 0.0f ? 1.0f : p < 0.0f ? -1.0f : 0.0f;}

_CLC_OVERLOAD _CLC_INLINE double fast_normalize(double p)  
{return p > 0.0 ? 1.0 : p < 0.0 ? -1.0 : 0.0;}

_CLC_OVERLOAD _CLC_INLINE float2  normalize(float2 p)        { return p / length(p); }
_CLC_OVERLOAD _CLC_INLINE float3  normalize(float3 p)        { return p / length(p); }
_CLC_OVERLOAD _CLC_INLINE float4  normalize(float4 p)        { return p / length(p); }
_CLC_OVERLOAD _CLC_INLINE double2 normalize(double2 p)       { return p / length(p); }
_CLC_OVERLOAD _CLC_INLINE double3 normalize(double3 p)       { return p / length(p); }
_CLC_OVERLOAD _CLC_INLINE double4 normalize(double4 p)       { return p / length(p); }

_CLC_OVERLOAD _CLC_INLINE float2  fast_normalize(float2 p)   { return p / fast_length(p); }
_CLC_OVERLOAD _CLC_INLINE float3  fast_normalize(float3 p)   { return p / fast_length(p); }
_CLC_OVERLOAD _CLC_INLINE float4  fast_normalize(float4 p)   { return p / fast_length(p); }
_CLC_OVERLOAD _CLC_INLINE double2 fast_normalize(double2 p)  { return p / fast_length(p); }
_CLC_OVERLOAD _CLC_INLINE double3 fast_normalize(double3 p)  { return p / fast_length(p); }
_CLC_OVERLOAD _CLC_INLINE double4 fast_normalize(double4 p)  { return p / fast_length(p); }

/*-----------------------------------------------------------------------------
* Atomics
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_DECL int  atomic_add(volatile global int*  p, int  val);
_CLC_OVERLOAD _CLC_DECL uint atomic_add(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_add(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_add(volatile local  uint* p, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_sub(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_sub(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_sub(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_sub(volatile local  uint* p, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_xchg(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xchg(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL float atomic_xchg(volatile global float* p, float val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_xchg(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xchg(volatile local  uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL float atomic_xchg(volatile local  float* p, float val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_inc(volatile global int*  p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_inc(volatile global uint* p); 
_CLC_OVERLOAD _CLC_DECL int  atomic_inc(volatile local  int*  p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_inc(volatile local  uint* p); 

_CLC_OVERLOAD _CLC_DECL int  atomic_dec(volatile global int*  p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_dec(volatile global uint* p); 
_CLC_OVERLOAD _CLC_DECL int  atomic_dec(volatile local  int*  p); 
_CLC_OVERLOAD _CLC_DECL uint atomic_dec(volatile local  uint* p); 

_CLC_OVERLOAD _CLC_DECL int  atomic_cmpxchg(volatile global int*  p, int  cmp, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_cmpxchg(volatile global uint* p, uint cmp, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_cmpxchg(volatile local  int*  p, int  cmp, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_cmpxchg(volatile local  uint* p, uint cmp, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_min(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_min(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_min(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_min(volatile local  uint* p, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_max(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_max(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_max(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_max(volatile local  uint* p, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_and(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_and(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_and(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_and(volatile local  uint* p, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_or(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_or(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_or(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_or(volatile local  uint* p, uint val); 

_CLC_OVERLOAD _CLC_DECL int  atomic_xor(volatile global int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xor(volatile global uint* p, uint val); 
_CLC_OVERLOAD _CLC_DECL int  atomic_xor(volatile local  int*  p, int  val); 
_CLC_OVERLOAD _CLC_DECL uint atomic_xor(volatile local  uint* p, uint val); 

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

#define TEMPLATE2(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##2 shuffle(res_elemt##val_vnum val, mask_elemt##2 mask);\
_CLC_OVERLOAD _CLC_DEF res_elemt##2 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##2 mask); 


#define TEMPLATE4(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##4 shuffle(res_elemt##val_vnum val, mask_elemt##4 mask); \
_CLC_OVERLOAD _CLC_DEF res_elemt##4 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##4 mask); 


#define TEMPLATE8(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##8 shuffle(res_elemt##val_vnum val, mask_elemt##8 mask); \
_CLC_OVERLOAD _CLC_DEF res_elemt##8 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##8 mask); 


#define TEMPLATE16(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##16 shuffle(res_elemt##val_vnum val, mask_elemt##16 mask); \
_CLC_OVERLOAD _CLC_DEF res_elemt##16 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##16 mask); 

#define CROSS_SIZE(type1, type2) \
TEMPLATE2(type1, 2, type2) \
TEMPLATE2(type1, 4, type2) \
TEMPLATE2(type1, 8, type2) \
TEMPLATE2(type1, 16, type2) \
TEMPLATE4(type1, 2, type2) \
TEMPLATE4(type1, 4, type2) \
TEMPLATE4(type1, 8, type2) \
TEMPLATE4(type1, 16, type2) \
TEMPLATE8(type1, 2, type2) \
TEMPLATE8(type1, 4, type2) \
TEMPLATE8(type1, 8, type2) \
TEMPLATE8(type1, 16, type2) \
TEMPLATE16(type1, 2, type2) \
TEMPLATE16(type1, 4, type2) \
TEMPLATE16(type1, 8, type2) \
TEMPLATE16(type1, 16, type2) \

#define CROSS_MASKTYPE(type) \
CROSS_SIZE(type, uchar) \
CROSS_SIZE(type, ushort) \
CROSS_SIZE(type, uint) \
CROSS_SIZE(type, ulong) \

CROSS_MASKTYPE(char)
CROSS_MASKTYPE(uchar)
CROSS_MASKTYPE(short)
CROSS_MASKTYPE(ushort)
CROSS_MASKTYPE(int)
CROSS_MASKTYPE(uint)
CROSS_MASKTYPE(long)
CROSS_MASKTYPE(ulong)
CROSS_MASKTYPE(float)
CROSS_MASKTYPE(double)

#undef TEMPLATE2
#undef TEMPLATE4
#undef TEMPLATE8
#undef TEMPLATE16
#undef CROSS_SIZE
#undef CROSS_MASKTYPE

#endif //_CLC_H_
