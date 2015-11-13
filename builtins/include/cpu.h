/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _CPU_CLC_H_
#define _CPU_CLC_H_

#include "clc.h"

#define _PREFETCH_VECTORIZE(_PRIM_TYPE) \
   _CLC_OVERLOAD _CLC_DECL void prefetch(const __global _PRIM_TYPE    *__p, size_t __num_gentypes); \
   _CLC_OVERLOAD _CLC_DECL void prefetch(const __global _PRIM_TYPE##2 *__p, size_t __num_gentypes); \
   _CLC_OVERLOAD _CLC_DECL void prefetch(const __global _PRIM_TYPE##3 *__p, size_t __num_gentypes); \
   _CLC_OVERLOAD _CLC_DECL void prefetch(const __global _PRIM_TYPE##4 *__p, size_t __num_gentypes); \
   _CLC_OVERLOAD _CLC_DECL void prefetch(const __global _PRIM_TYPE##8 *__p, size_t __num_gentypes); \
   _CLC_OVERLOAD _CLC_DECL void prefetch(const __global _PRIM_TYPE##16 *__p, size_t __num_gentypes); \

_PREFETCH_VECTORIZE(char)
_PREFETCH_VECTORIZE(uchar)
_PREFETCH_VECTORIZE(short)
_PREFETCH_VECTORIZE(ushort)
_PREFETCH_VECTORIZE(int)
_PREFETCH_VECTORIZE(uint)
_PREFETCH_VECTORIZE(long)
_PREFETCH_VECTORIZE(ulong)
_PREFETCH_VECTORIZE(float)
_PREFETCH_VECTORIZE(double)

#undef _PREFETCH_VECTORIZE

/*-----------------------------------------------------------------------------
* This can be empty since our copy routines are currently synchronous. When 
* the copy routines are improved to be asynchronous, then this function will
* need a real implementation.
*----------------------------------------------------------------------------*/
void wait_group_events(int num_events, event_t* event_list) {}

#define _CROSS_SIZES(__type) \
    _TEMPLATE(__type)              \
    _TEMPLATE(_VEC_TYPE(__type,2))  \
    _TEMPLATE(_VEC_TYPE(__type,3))  \
    _TEMPLATE(_VEC_TYPE(__type,4))  \
    _TEMPLATE(_VEC_TYPE(__type,8))  \
    _TEMPLATE(_VEC_TYPE(__type,16)) \

#define _TEMPLATE(__gentype) \
_CLC_OVERLOAD _CLC_DECL event_t async_work_group_copy(__local __gentype *__dst, const __global __gentype *__src, \
		              size_t __num_gentypes, event_t __event); \
_CLC_OVERLOAD _CLC_DECL event_t async_work_group_copy(__global __gentype *__dst, const __local __gentype *__src, \
		              size_t __num_gentypes, event_t __event); \
_CLC_OVERLOAD _CLC_DECL event_t async_work_group_copy(__global __gentype *__dst, const __global __gentype *__src, \
		              size_t __num_gentypes, event_t __event); \
_CLC_OVERLOAD _CLC_DECL event_t async_work_group_strided_copy(__local __gentype *__dst, const __global __gentype *__src, \
		              size_t __num_gentypes, size_t __src_stride, event_t __event); \
_CLC_OVERLOAD _CLC_DECL event_t async_work_group_strided_copy(__global __gentype *__dst, const __local __gentype *__src, \
		              size_t __num_gentypes, size_t __dst_stride, event_t __event); \

_CROSS_SIZES(char)
_CROSS_SIZES(uchar)
_CROSS_SIZES(short)
_CROSS_SIZES(ushort)
_CROSS_SIZES(int)
_CROSS_SIZES(uint)
_CROSS_SIZES(long)
_CROSS_SIZES(ulong)
_CROSS_SIZES(float)
_CROSS_SIZES(double)

#undef _CROSS_SIZES
#undef _TEMPLATE

_CLC_OVERLOAD _CLC_DECL char  rotate(char __v, char __i);
_CLC_OVERLOAD _CLC_DECL uchar rotate(uchar __v, uchar __i);
_CLC_OVERLOAD _CLC_DECL short  rotate(short __v, short __i);
_CLC_OVERLOAD _CLC_DECL ushort rotate(ushort __v, ushort __i);
_CLC_OVERLOAD _CLC_DECL long  rotate(long __v, long __i);
_CLC_OVERLOAD _CLC_DECL ulong rotate(ulong __v, ulong __i);
_CLC_OVERLOAD _CLC_DECL int  rotate(int __v, int __i);
_CLC_OVERLOAD _CLC_DECL uint rotate(uint __v, uint __i);

_BINARY_VEC_DECL(char, char, rotate)
_BINARY_VEC_DECL(uchar, uchar, rotate)
_BINARY_VEC_DECL(short, short, rotate)
_BINARY_VEC_DECL(ushort, ushort, rotate)
_BINARY_VEC_DECL(int, int, rotate)
_BINARY_VEC_DECL(uint, uint, rotate)
_BINARY_VEC_DECL(long, long, rotate)
_BINARY_VEC_DECL(ulong, ulong, rotate)

_CLC_OVERLOAD _CLC_DECL char   clz(char   __v) ;
_CLC_OVERLOAD _CLC_DECL uchar  clz(uchar  __v) ;
_CLC_OVERLOAD _CLC_DECL short  clz(short  __v) ;
_CLC_OVERLOAD _CLC_DECL ushort clz(ushort __v) ;
_CLC_OVERLOAD _CLC_DECL int    clz(int    __v) ;
_CLC_OVERLOAD _CLC_DECL uint   clz(uint   __v) ;
_CLC_OVERLOAD _CLC_DECL long clz(long   __v) ;
_CLC_OVERLOAD _CLC_DECL ulong clz(ulong  __v);

_UNARY_VEC_DECL(char, char, clz)
_UNARY_VEC_DECL(uchar, uchar, clz)
_UNARY_VEC_DECL(short, short, clz)
_UNARY_VEC_DECL(ushort, ushort, clz)
_UNARY_VEC_DECL(int, int, clz)
_UNARY_VEC_DECL(uint, uint, clz)
_UNARY_VEC_DECL(long, long, clz)
_UNARY_VEC_DECL(ulong, ulong, clz)

_CLC_OVERLOAD _CLC_DECL uchar  abs(char __x)   ;
_CLC_OVERLOAD _CLC_DECL ushort abs(short __x)  ;
_CLC_OVERLOAD _CLC_DECL uint   abs(int __x)    ;
_CLC_OVERLOAD _CLC_DECL ulong  abs(long __x)   ;
_CLC_OVERLOAD _CLC_DECL uchar  abs(uchar __x)  ;
_CLC_OVERLOAD _CLC_DECL ushort abs(ushort __x) ;
_CLC_OVERLOAD _CLC_DECL uint   abs(uint __x)   ;
_CLC_OVERLOAD _CLC_DECL ulong  abs(ulong __x)  ;

_UNARY_VEC_DECL(char,  uchar,  abs)
_UNARY_VEC_DECL(short, ushort, abs)
_UNARY_VEC_DECL(int,   uint,   abs)
_UNARY_VEC_DECL(long,  ulong,  abs)

/*-----------------------------------------------------------------------------
* ABS for unsigned types is straightforward
*----------------------------------------------------------------------------*/
#define _DEFINE(__utype) \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,2)  abs(_VEC_TYPE(__utype,2) __x)  {return __x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,3)  abs(_VEC_TYPE(__utype,3) __x)  {return __x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,4)  abs(_VEC_TYPE(__utype,4) __x)  {return __x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,8)  abs(_VEC_TYPE(__utype,8) __x)  {return __x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,16) abs(_VEC_TYPE(__utype,16) __x) {return __x;}\

_DEFINE(uchar)
_DEFINE(ushort)
_DEFINE(uint)
_DEFINE(ulong)

#undef _DEFINE

_CLC_OVERLOAD _CLC_DECL long   mul_hi(long __x,   long __y);
_CLC_OVERLOAD _CLC_DECL ulong  mul_hi(ulong __x,   ulong __y);
_CLC_OVERLOAD _CLC_DECL char  mul_hi(char __x,  char __y);        
_CLC_OVERLOAD _CLC_DECL uchar mul_hi(uchar __x, uchar __y);       
_CLC_OVERLOAD _CLC_DECL short  mul_hi(short __x,  short __y);     
_CLC_OVERLOAD _CLC_DECL ushort mul_hi(ushort __x, ushort __y);    
_CLC_OVERLOAD _CLC_DECL int    mul_hi(int __x,    int __y);       
_CLC_OVERLOAD _CLC_DECL uint   mul_hi(uint __x,   uint __y);      

_BINARY_VEC_DECL(char, char, mul_hi)
_BINARY_VEC_DECL(uchar, uchar, mul_hi)
_BINARY_VEC_DECL(short, short, mul_hi)
_BINARY_VEC_DECL(ushort, ushort, mul_hi)
_BINARY_VEC_DECL(int, int, mul_hi)
_BINARY_VEC_DECL(uint, uint, mul_hi)
_BINARY_VEC_DECL(long, long, mul_hi)
_BINARY_VEC_DECL(ulong, ulong, mul_hi)


_CLC_OVERLOAD _CLC_DECL char   add_sat(char __x, char __y)   ;
_CLC_OVERLOAD _CLC_DECL uchar  add_sat(uchar __x, uchar __y) ;
_CLC_OVERLOAD _CLC_DECL short  add_sat(short __x, short __y) ;
_CLC_OVERLOAD _CLC_DECL ushort add_sat(ushort __x, ushort __y) ;
_CLC_OVERLOAD _CLC_DECL int    add_sat(int __x, int __y)     ;
_CLC_OVERLOAD _CLC_DECL uint   add_sat(uint __x, uint __y)   ;
_CLC_OVERLOAD _CLC_DECL long   add_sat(long __x, long __y)   ;
_CLC_OVERLOAD _CLC_DECL ulong   add_sat(ulong __x, ulong __y)   ;

_BINARY_VEC_DECL(char, char, add_sat)
_BINARY_VEC_DECL(uchar, uchar, add_sat)
_BINARY_VEC_DECL(short, short, add_sat)
_BINARY_VEC_DECL(ushort, ushort, add_sat)
_BINARY_VEC_DECL(int, int, add_sat)
_BINARY_VEC_DECL(uint, uint, add_sat)
_BINARY_VEC_DECL(long, long, add_sat)
_BINARY_VEC_DECL(ulong, ulong, add_sat)


_CLC_OVERLOAD _CLC_DECL char   sub_sat(char __x, char __y)   ;
_CLC_OVERLOAD _CLC_DECL uchar  sub_sat(uchar __x, uchar __y) ;
_CLC_OVERLOAD _CLC_DECL short  sub_sat(short __x, short __y) ;
_CLC_OVERLOAD _CLC_DECL ushort sub_sat(ushort __x, ushort __y) ;
_CLC_OVERLOAD _CLC_DECL int    sub_sat(int __x, int __y)     ;
_CLC_OVERLOAD _CLC_DECL uint   sub_sat(uint __x, uint __y)   ;
_CLC_OVERLOAD _CLC_DECL long   sub_sat(long __x, long __y)   ;
_CLC_OVERLOAD _CLC_DECL ulong   sub_sat(ulong __x, ulong __y)   ;

_BINARY_VEC_DECL(char, char, sub_sat)
_BINARY_VEC_DECL(uchar, uchar, sub_sat)
_BINARY_VEC_DECL(short, short, sub_sat)
_BINARY_VEC_DECL(ushort, ushort, sub_sat)
_BINARY_VEC_DECL(int, int, sub_sat)
_BINARY_VEC_DECL(uint, uint, sub_sat)
_BINARY_VEC_DECL(long, long, sub_sat)
_BINARY_VEC_DECL(ulong, ulong, sub_sat)

_CLC_OVERLOAD _CLC_DECL short   upsample(char __x, uchar __y)   ;
_CLC_OVERLOAD _CLC_DECL ushort  upsample(uchar __x, uchar __y) ;
_CLC_OVERLOAD _CLC_DECL int     upsample(short __x, ushort __y) ;
_CLC_OVERLOAD _CLC_DECL uint    upsample(ushort __x, ushort __y) ;
_CLC_OVERLOAD _CLC_DECL long     upsample(int __x, uint __y) ;
_CLC_OVERLOAD _CLC_DECL ulong    upsample(uint __x, uint __y) ;

_BINARY_VEC_DECL_ALT(char,  short,  uchar, upsample)
_BINARY_VEC_DECL_ALT(uchar, ushort, uchar, upsample)
_BINARY_VEC_DECL_ALT(short,  int,  ushort, upsample)
_BINARY_VEC_DECL_ALT(ushort, uint, ushort, upsample)
_BINARY_VEC_DECL_ALT(int,  long,  uint, upsample)
_BINARY_VEC_DECL_ALT(uint, ulong, uint, upsample)

_CLC_OVERLOAD _CLC_DECL char mad_sat(char __a, char __b, char __c);
_CLC_OVERLOAD _CLC_DECL uchar mad_sat(uchar __a, uchar __b, uchar __c);
_CLC_OVERLOAD _CLC_DECL short mad_sat(short __a, short __b, short __c);
_CLC_OVERLOAD _CLC_DECL ushort mad_sat(ushort __a, ushort __b, ushort __c);
_CLC_OVERLOAD _CLC_DECL int mad_sat(int __a, int __b, int __c);
_CLC_OVERLOAD _CLC_DECL uint mad_sat(uint __a, uint __b, uint __c);
_CLC_OVERLOAD _CLC_DECL long mad_sat(long __a, long __b, long __c);
_CLC_OVERLOAD _CLC_DECL ulong mad_sat(ulong __a, ulong __b, ulong __c);

_TERNARY_VEC_DECL(char, char, mad_sat)
_TERNARY_VEC_DECL(uchar, uchar, mad_sat)
_TERNARY_VEC_DECL(short, short, mad_sat)
_TERNARY_VEC_DECL(ushort, ushort, mad_sat)
_TERNARY_VEC_DECL(int, int, mad_sat)
_TERNARY_VEC_DECL(uint, uint, mad_sat)
_TERNARY_VEC_DECL(long, long, mad_sat)
_TERNARY_VEC_DECL(ulong, ulong, mad_sat)

int printf(__constant char* restrict __format, ...);

_CLC_DECL  size_t  get_local_id     (uint __dim);
_CLC_DECL  uint    get_work_dim     (void)     ;
_CLC_DECL  size_t  get_global_size  (uint __dim) ;
_CLC_DECL  size_t  get_local_size   (uint __dim) ;
_CLC_DECL  size_t  get_global_offset(uint __dim) ;
_CLC_DECL  size_t __get_global_first(uint __dim) ;
_CLC_DECL  size_t  get_num_groups   (uint __dim)  ;
_CLC_DECL  size_t  get_global_id    (uint __dim) ;
_CLC_DECL  size_t  get_group_id     (uint __dim) ;
                     
#endif //_CPU_CLC_H_

