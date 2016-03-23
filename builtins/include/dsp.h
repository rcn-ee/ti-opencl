/******************************************************************************
 * Copyright (c) 2011-2014, Peter Collingbourne <peter@pcc.me.uk>
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#ifndef _DSP_CLC_H_
#define _DSP_CLC_H_

#include "clc.h"


#if 0
#define event_t __local int*
#else
struct __ocl_event { int __dummy; };
typedef struct __ocl_event* __ocl_event_t;
#define event_t __ocl_event_t
#endif


/*-----------------------------------------------------------------------------
* The extended TI built-in function set
*----------------------------------------------------------------------------*/
event_t  __copy_1D1D(event_t __event, void *__dst, void *__src, uint32_t __bytes);
event_t  __copy_2D1D(event_t __event, void *__dst, void *__src, uint32_t __bytes, uint32_t __num_lines, int32_t __pitch); 
event_t  __copy_1D2D(event_t __event, void *__dst, void *__src, uint32_t __bytes, uint32_t __num_lines, int32_t __pitch); 
void     __copy_wait(event_t __event);
void     __touch(const __global char *__p, uint32_t __size);


void __touch (const __global char *__p, uint32_t __size);
#define _PREFETCH_VECTORIZE(_PRIM_TYPE) \
   _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global _PRIM_TYPE    *__p, size_t __num_gentypes) \
    { __touch((const __global char*)__p, (uint32_t)(__num_gentypes * sizeof(*__p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global _PRIM_TYPE##2 *__p, size_t __num_gentypes) \
    { __touch((const __global char*)__p, (uint32_t)(__num_gentypes * sizeof(*__p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global _PRIM_TYPE##3 *__p, size_t __num_gentypes) \
    { __touch((const __global char*)__p, (uint32_t)(__num_gentypes * sizeof(*__p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global _PRIM_TYPE##4 *__p, size_t __num_gentypes) \
    { __touch((const __global char*)__p, (uint32_t)(__num_gentypes * sizeof(*__p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global _PRIM_TYPE##8 *__p, size_t __num_gentypes) \
    { __touch((const __global char*)__p, (uint32_t)(__num_gentypes * sizeof(*__p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global _PRIM_TYPE##16 *__p, size_t __num_gentypes) \
    { __touch((const __global char*)__p, (uint32_t)(__num_gentypes * sizeof(*__p))); }

#define _PREFETCH_TYPES()           \
        _PREFETCH_VECTORIZE(char)   \
        _PREFETCH_VECTORIZE(uchar)  \
        _PREFETCH_VECTORIZE(short)  \
        _PREFETCH_VECTORIZE(ushort) \
        _PREFETCH_VECTORIZE(int)    \
        _PREFETCH_VECTORIZE(uint)   \
        _PREFETCH_VECTORIZE(long)   \
        _PREFETCH_VECTORIZE(ulong)  \
        _PREFETCH_VECTORIZE(float)  \
        _PREFETCH_VECTORIZE(double)

_PREFETCH_TYPES()

#undef _PREFETCH_TYPES
#undef _PREFETCH_VECTORIZE
    
_CLC_DECL size_t  get_local_id     (uint __dim);

_CLC_OVERLOAD _CLC_NODUP _CLC_DECL void wait_group_events(int __num_events, event_t *__event_list);

#define _CROSS_SIZES(__type) \
    _TEMPLATE(__type)              \
    _TEMPLATE(_VEC_TYPE(__type,2))  \
    _TEMPLATE(_VEC_TYPE(__type,3))  \
    _TEMPLATE(_VEC_TYPE(__type,4))  \
    _TEMPLATE(_VEC_TYPE(__type,8))  \
    _TEMPLATE(_VEC_TYPE(__type,16)) \

#define _CROSS_TYPES()   \
    _CROSS_SIZES(char)   \
    _CROSS_SIZES(uchar)  \
    _CROSS_SIZES(short)  \
    _CROSS_SIZES(ushort) \
    _CROSS_SIZES(int)    \
    _CROSS_SIZES(uint)   \
    _CROSS_SIZES(long)   \
    _CROSS_SIZES(ulong)  \
    _CROSS_SIZES(float)  \
    _CROSS_SIZES(double) \

#define _TEMPLATE(__gentype) \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_copy(__local __gentype *__dst, \
              const __global __gentype *__src, size_t __num_gentypes, event_t __event); \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_copy(__global __gentype *__dst, \
              const __local __gentype *__src, size_t __num_gentypes, event_t __event); \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_copy(__global __gentype *__dst, \
              const __global __gentype *__src, size_t __num_gentypes, event_t __event); \

_CROSS_TYPES()


#undef _TEMPLATE
#define _TEMPLATE(__gentype) \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_strided_copy(__local __gentype *__dst, \
        const __global __gentype *__src, size_t __num_gentypes, size_t __src_stride, event_t __event); \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_strided_copy(__global __gentype *dst, \
        const __local __gentype *__src, size_t __num_gentypes, size_t __dst_stride, event_t __event); \

_CROSS_TYPES()

#undef _CROSS_SIZES
#undef _CROSS_TYPES
#undef _TEMPLATE


_CLC_OVERLOAD _CLC_DECL char  rotate(char __v, char __i);
_CLC_OVERLOAD _CLC_DECL uchar rotate(uchar __v, uchar __i);
_CLC_OVERLOAD _CLC_DECL short  rotate(short __v, short __i);
_CLC_OVERLOAD _CLC_DECL ushort rotate(ushort __v, ushort __i);
_CLC_OVERLOAD _CLC_INLINE int  rotate(int __v, int __i)        { return _rotl(__v,__i); }
_CLC_OVERLOAD _CLC_INLINE uint rotate(uint __v, uint __i)      { return _rotl(__v,__i); }
_CLC_OVERLOAD _CLC_DECL long  rotate(long __v, long __i);
_CLC_OVERLOAD _CLC_DECL ulong rotate(ulong __v, ulong __i);

_BINARY_VEC_DECL(char, char, rotate)
_BINARY_VEC_DECL(uchar, uchar, rotate)
_BINARY_VEC_DECL(short, short, rotate)
_BINARY_VEC_DECL(ushort, ushort, rotate)
_BINARY_VEC_DECL(int, int, rotate)
_BINARY_VEC_DECL(uint, uint, rotate)
_BINARY_VEC_DECL(long, long, rotate)
_BINARY_VEC_DECL(ulong, ulong, rotate)

_CLC_OVERLOAD _CLC_INLINE char   clz(char   __v) { return __v<0?0: _lmbd(1,__v)-24; } 
_CLC_OVERLOAD _CLC_INLINE uchar  clz(uchar  __v) { return _lmbd(1, __v) - 24; } 
_CLC_OVERLOAD _CLC_INLINE short  clz(short  __v) { return __v<0?0: _lmbd(1,__v)-16; } 
_CLC_OVERLOAD _CLC_INLINE ushort clz(ushort __v) { return _lmbd(1, __v) - 16; } 
_CLC_OVERLOAD _CLC_INLINE int    clz(int    __v) { return _lmbd(1, __v); } 
_CLC_OVERLOAD _CLC_INLINE uint   clz(uint   __v) { return _lmbd(1, __v); } 

_CLC_OVERLOAD _CLC_INLINE long clz(long   __v) 
{
    uint2 __tmp  = as_uint2(__v);
    return __tmp.hi ? _lmbd(1, __tmp.hi) : _lmbd(1, __tmp.lo) + 32;
}

_CLC_OVERLOAD _CLC_INLINE ulong clz(ulong  __v)
{
    uint2 __tmp  = as_uint2(__v);
    return __tmp.hi ? _lmbd(1, __tmp.hi) : _lmbd(1, __tmp.lo) + 32;
}

_UNARY_VEC_DECL(char, char, clz)
_UNARY_VEC_DECL(uchar, uchar, clz)
_UNARY_VEC_DECL(short, short, clz)
_UNARY_VEC_DECL(ushort, ushort, clz)
_UNARY_VEC_DECL(int, int, clz)
_UNARY_VEC_DECL(uint, uint, clz)
_UNARY_VEC_DECL(long, long, clz)
_UNARY_VEC_DECL(ulong, ulong, clz)


_CLC_OVERLOAD _CLC_INLINE uchar  abs(char __x)    { return _abs(__x); }
_CLC_OVERLOAD _CLC_INLINE ushort abs(short __x)   { return _abs(__x); }
_CLC_OVERLOAD _CLC_INLINE uint   abs(int __x)     { return _abs(__x); }
_CLC_OVERLOAD _CLC_INLINE ulong  abs(long __x)    { if (__x < 0) __x = -__x; return __x; }

_CLC_OVERLOAD _CLC_INLINE uchar  abs(uchar __x)   { return __x; }
_CLC_OVERLOAD _CLC_INLINE ushort abs(ushort __x)  { return __x; }
_CLC_OVERLOAD _CLC_INLINE uint   abs(uint __x)    { return __x; }
_CLC_OVERLOAD _CLC_INLINE ulong  abs(ulong __x)   { return __x; }

_UNARY_VEC_DECL(char,  uchar,  abs)
_UNARY_VEC_DECL(short, ushort, abs)
_UNARY_VEC_DECL(int,   uint,   abs)
_UNARY_VEC_DECL(long,  ulong,  abs)

/*-----------------------------------------------------------------------------
* ABS for unsigned types is straightforward
*----------------------------------------------------------------------------*/
#define _DEFINE(__utype) \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,2)  abs(_VEC_TYPE(__utype,2) __x)  {return __x;} \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,3)  abs(_VEC_TYPE(__utype,3) __x)  {return __x;} \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,4)  abs(_VEC_TYPE(__utype,4) __x)  {return __x;} \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,8)  abs(_VEC_TYPE(__utype,8) __x)  {return __x;} \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(__utype,16) abs(_VEC_TYPE(__utype,16) __x) {return __x;} \

_DEFINE(uchar)
_DEFINE(ushort)
_DEFINE(uint)
_DEFINE(ulong)

#undef _DEFINE

_CLC_OVERLOAD _CLC_DECL long   mul_hi(long __x,   long __y);
_CLC_OVERLOAD _CLC_DECL ulong  mul_hi(ulong __x,   ulong __y);

_CLC_OVERLOAD _CLC_INLINE char  mul_hi(char __x,  char __y)        
{ return _mpy(__x,__y)  >> 8; }

_CLC_OVERLOAD _CLC_INLINE uchar mul_hi(uchar __x, uchar __y)       
{ return _mpyu(__x,__y) >> 8; }

_CLC_OVERLOAD _CLC_INLINE short  mul_hi(short __x,  short __y)     
{ return _mpy(__x,__y)  >> 16; }

_CLC_OVERLOAD _CLC_INLINE ushort mul_hi(ushort __x, ushort __y)    
{ return _mpyu(__x,__y) >> 16; }

_CLC_OVERLOAD _CLC_INLINE int    mul_hi(int __x,    int __y)       
{ return ((long)__x * (long)__y)   >> 32; }

_CLC_OVERLOAD _CLC_INLINE uint   mul_hi(uint __x,   uint __y)      
{ return ((ulong)__x * (ulong)__y) >> 32; }

_BINARY_VEC_DECL(char, char, mul_hi)
_BINARY_VEC_DECL(uchar, uchar, mul_hi)
_BINARY_VEC_DECL(short, short, mul_hi)
_BINARY_VEC_DECL(ushort, ushort, mul_hi)
_BINARY_VEC_DECL(int, int, mul_hi)
_BINARY_VEC_DECL(uint, uint, mul_hi)
_BINARY_VEC_DECL(long, long, mul_hi)
_BINARY_VEC_DECL(ulong, ulong, mul_hi)

_CLC_OVERLOAD _CLC_INLINE char   add_sat(char __x, char __y)   
{ return _sadd(__x<<24, __y<<24)>>24; }

_CLC_OVERLOAD _CLC_INLINE uchar  add_sat(uchar __x, uchar __y) 
{ return _saddu4(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE short  add_sat(short __x, short __y) 
{ return _sadd2(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE ushort add_sat(ushort __x, ushort __y) 
{ 
    int __tmp = __x + __y;
    if (__tmp >> 16) return USHRT_MAX;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE int    add_sat(int __x, int __y)     
{ return _sadd(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE uint   add_sat(uint __x, uint __y)   
{ 
    ulong __tmp = (ulong)__x + (ulong)__y;
    if (__tmp >> 32) return UINT_MAX;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE long   add_sat(long __x, long __y)   
{ 
    if (__x > 0 && __y > (LONG_MAX-__x)) return LONG_MAX;
    if (__x < 0 && __y < (LONG_MIN-__x)) return LONG_MIN;
    return __x + __y;
}

_CLC_OVERLOAD _CLC_INLINE ulong   add_sat(ulong __x, ulong __y)   
{ 
    if (__y > (ULONG_MAX-__x)) return ULONG_MAX;
    return __x + __y;
}

_BINARY_VEC_DECL(char, char, add_sat)
_BINARY_VEC_DECL(uchar, uchar, add_sat)
_BINARY_VEC_DECL(short, short, add_sat)
_BINARY_VEC_DECL(ushort, ushort, add_sat)
_BINARY_VEC_DECL(int, int, add_sat)
_BINARY_VEC_DECL(uint, uint, add_sat)
_BINARY_VEC_DECL(long, long, add_sat)
_BINARY_VEC_DECL(ulong, ulong, add_sat)


_CLC_OVERLOAD _CLC_INLINE char   sub_sat(char __x, char __y)   
{ return _ssub(__x<<24, __y<<24)>>24; }

_CLC_OVERLOAD _CLC_INLINE uchar  sub_sat(uchar __x, uchar __y) 
{ 
    if (__y > __x) return 0;
    return __x-__y; 
}

_CLC_OVERLOAD _CLC_INLINE short  sub_sat(short __x, short __y) 
{ return _ssub2(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE ushort sub_sat(ushort __x, ushort __y) 
{ 
    if (__y > __x) return 0;
    return __x-__y; 
}

_CLC_OVERLOAD _CLC_INLINE int    sub_sat(int __x, int __y)     
{ return _ssub(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE uint   sub_sat(uint __x, uint __y)   
{ 
    if (__y > __x) return 0;
    return __x-__y; 
}

_CLC_OVERLOAD _CLC_INLINE long   sub_sat(long __x, long __y)   
{ 
    if (__x > 0 && -__y > (LONG_MAX-__x)) return LONG_MAX;
    if (__x < 0 && -__y < (LONG_MIN-__x)) return LONG_MIN;

    return __x - __y;
}

_CLC_OVERLOAD _CLC_INLINE ulong   sub_sat(ulong __x, ulong __y)   
{ 
    if (__y > __x) return 0;
    return __x-__y; 
}

_BINARY_VEC_DECL(char, char, sub_sat)
_BINARY_VEC_DECL(uchar, uchar, sub_sat)
_BINARY_VEC_DECL(short, short, sub_sat)
_BINARY_VEC_DECL(ushort, ushort, sub_sat)
_BINARY_VEC_DECL(int, int, sub_sat)
_BINARY_VEC_DECL(uint, uint, sub_sat)
_BINARY_VEC_DECL(long, long, sub_sat)
_BINARY_VEC_DECL(ulong, ulong, sub_sat)


_CLC_OVERLOAD _CLC_INLINE short   upsample(char __x, uchar __y)   
{ return (short)__x << 8 | __y; }

_CLC_OVERLOAD _CLC_INLINE ushort  upsample(uchar __x, uchar __y) 
{ return (ushort)__x << 8 | __y; } 

_CLC_OVERLOAD _CLC_INLINE int     upsample(short __x, ushort __y) 
{ return (int) _pack2(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE uint    upsample(ushort __x, ushort __y) 
{ return (uint) _pack2(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE long     upsample(int __x, uint __y) 
{ return (long) _itoll(__x,__y); }

_CLC_OVERLOAD _CLC_INLINE ulong    upsample(uint __x, uint __y) 
{ return (ulong) _itoll(__x,__y); }

_BINARY_VEC_DECL_ALT(char,  short,  uchar, upsample)
_BINARY_VEC_DECL_ALT(uchar, ushort, uchar, upsample)
_BINARY_VEC_DECL_ALT(short,  int,  ushort, upsample)
_BINARY_VEC_DECL_ALT(ushort, uint, ushort, upsample)
_BINARY_VEC_DECL_ALT(int,  long,  uint, upsample)
_BINARY_VEC_DECL_ALT(uint, ulong, uint, upsample)


_CLC_OVERLOAD _CLC_INLINE char mad_sat(char __a, char __b, char __c)
{
    int __tmp = _mpy32(__a,__b);
    __tmp += __c;

    if (__tmp > (int)CHAR_MAX) return CHAR_MAX;
    if (__tmp < (int)CHAR_MIN) return CHAR_MIN;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE uchar mad_sat(uchar __a, uchar __b, uchar __c)
{
    uint __tmp = _mpy32u(__a,__b);
    __tmp += __c;

    if (__tmp > (uint)UCHAR_MAX) return UCHAR_MAX;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE short mad_sat(short __a, short __b, short __c)
{
    int __tmp = _mpy32(__a,__b);
    __tmp += __c;

    if (__tmp > (int)SHRT_MAX) return SHRT_MAX;
    if (__tmp < (int)SHRT_MIN) return SHRT_MIN;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE ushort mad_sat(ushort __a, ushort __b, ushort __c)
{
    uint __tmp = _mpy32u(__a,__b);
    __tmp += __c;

    if (__tmp > (uint)USHRT_MAX) return USHRT_MAX;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE int mad_sat(int __a, int __b, int __c)
{
    long __tmp = (long)__a * (long)__b + (long)__c;
    if (__tmp > (long)INT_MAX) return INT_MAX;
    if (__tmp < (long)INT_MIN) return INT_MIN;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE uint mad_sat(uint __a, uint __b, uint __c)
{
    ulong __tmp = _mpy32u(__a,__b);
    __tmp += __c;

    if (__tmp > (ulong)UINT_MAX) return UINT_MAX;
    return __tmp;
}

_CLC_OVERLOAD _CLC_INLINE long mad_sat(long __a, long __b, long __c)
{
    if (__a > 0 && __b > 0 && __a > (LONG_MAX/__b)) return LONG_MAX;
    if (__a > 0 && __b < 0 && __b < (LONG_MIN/__a)) return LONG_MIN;
    if (__a < 0 && __b > 0 && __a < (LONG_MIN/__b)) return LONG_MIN;
    if (__a < 0 && __b < 0 && __b < (LONG_MAX/__a)) return LONG_MAX;

    return add_sat(__a*__b, __c);
}

_CLC_OVERLOAD _CLC_INLINE ulong mad_sat(ulong __a, ulong __b, ulong __c)
{
    if (__a > (ULONG_MAX/__b)) return ULONG_MAX;
    return add_sat(__a*__b, __c);
}

_TERNARY_VEC_DECL(char, char, mad_sat)
_TERNARY_VEC_DECL(uchar, uchar, mad_sat)
_TERNARY_VEC_DECL(short, short, mad_sat)
_TERNARY_VEC_DECL(ushort, ushort, mad_sat)
_TERNARY_VEC_DECL(int, int, mad_sat)
_TERNARY_VEC_DECL(uint, uint, mad_sat)
_TERNARY_VEC_DECL(long, long, mad_sat)
_TERNARY_VEC_DECL(ulong, ulong, mad_sat)

/******************************************************************************
* VLOAD
* char, uchar, short, ushort, int, uint, long, ulong, float, double
* n = 2,3,4,8,16
******************************************************************************/
#define _VLOAD_ADDR_SPACES(_ADDR_SPACE) \
_CLC_OVERLOAD _CLC_INLINE uchar2 vload2(size_t __offset, _ADDR_SPACE const uchar *__p) \
{ return as_uchar2(_mem2((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE uchar4 vload4(size_t __offset, _ADDR_SPACE const uchar *__p) \
{ return as_uchar4(_mem4((void*)(__p+(__offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE uchar8 vload8(size_t __offset, _ADDR_SPACE const uchar *__p) \
{ return as_uchar8(_memd8((void*)(__p+(__offset*8)))); }\
\
_CLC_OVERLOAD _CLC_INLINE char2 vload2(size_t __offset, _ADDR_SPACE const char *__p) \
{ return as_char2(_mem2((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE char4 vload4(size_t __offset, _ADDR_SPACE const char *__p) \
{ return as_char4(_mem4((void*)(__p+(__offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE char8 vload8(size_t __offset, _ADDR_SPACE const char *__p) \
{ return as_char8(_memd8((void*)(__p+(__offset*8)))); }\
\
_CLC_OVERLOAD _CLC_INLINE ushort2 vload2(size_t __offset, _ADDR_SPACE const ushort *__p) \
{ return as_ushort2(_mem4((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE ushort4 vload4(size_t __offset, _ADDR_SPACE const ushort *__p) \
{ return as_ushort4(_memd8((void*)(__p+(__offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE short2 vload2(size_t __offset, _ADDR_SPACE const short *__p) \
{ return as_short2(_mem4((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE short4 vload4(size_t __offset, _ADDR_SPACE const short *__p) \
{ return as_short4(_memd8((void*)(__p+(__offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE uint2 vload2(size_t __offset, _ADDR_SPACE const uint *__p) \
{ return as_uint2(_memd8((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE int2 vload2(size_t __offset, _ADDR_SPACE const int *__p) \
{ return as_int2(_memd8((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE float2 vload2(size_t __offset, _ADDR_SPACE const float *__p) \
{ return as_float2(_memd8((void*)(__p+(__offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE ulong2   vload2 (size_t __offset, _ADDR_SPACE const ulong *__p) \
{ return (ulong2)( as_ulong(_memd8((void*)(__p+(__offset*2)))), as_ulong(_memd8((void*)(__p+1+(__offset*2))))); }\
\
_CLC_OVERLOAD _CLC_INLINE long2    vload2 (size_t __offset, _ADDR_SPACE const long *__p) \
{ return (long2)( as_long(_memd8((void*)(__p+(__offset*2)))), as_long(_memd8((void*)(__p+1+(__offset*2))))); }\
\
_CLC_OVERLOAD _CLC_INLINE double2  vload2 (size_t __offset, _ADDR_SPACE const double *__p) \
{ return (double2)( as_double(_memd8((void*)(__p+(__offset*2)))), as_double(_memd8((void*)(__p+1+(__offset*2))))); }\
\
_CLC_OVERLOAD _CLC_INLINE uchar16  vload16(size_t __offset, _ADDR_SPACE const uchar *__p)\
{ return (uchar16)(vload8(__offset*2, __p), vload8(__offset*2+1, __p)); }\
\
_CLC_OVERLOAD _CLC_INLINE char16   vload16(size_t __offset, _ADDR_SPACE const char *__p)\
{ return (char16)(vload8(__offset*2, __p), vload8(__offset*2+1, __p)); }\
\
_CLC_OVERLOAD _CLC_INLINE ushort8  vload8 (size_t __offset, _ADDR_SPACE const ushort *__p)\
{ return (ushort8)(vload4(__offset*2, __p), vload4(__offset*2+1, __p)); }\
\
_CLC_OVERLOAD _CLC_INLINE short8   vload8 (size_t __offset, _ADDR_SPACE const short *__p)\
{ return (short8)(vload4(__offset*2, __p), vload4(__offset*2+1, __p)); }\
\
_CLC_OVERLOAD _CLC_INLINE uint4    vload4 (size_t __offset, _ADDR_SPACE const uint *__p)\
{ return (uint4)(vload2(__offset*2, __p), vload2(__offset*2+1, __p)); }\
\
_CLC_OVERLOAD _CLC_INLINE int4     vload4 (size_t __offset, _ADDR_SPACE const int *__p)\
{ return (int4)(vload2(__offset*2, __p), vload2(__offset*2+1, __p)); }\
\
_CLC_OVERLOAD _CLC_INLINE float4   vload4 (size_t __offset, _ADDR_SPACE const float *__p)\
{ return (float4)(vload2(__offset*2, __p), vload2(__offset*2+1, __p)); }\
\

_VLOAD_ADDR_SPACES(__private)
_VLOAD_ADDR_SPACES(__constant)
_VLOAD_ADDR_SPACES(__global)
_VLOAD_ADDR_SPACES(__local)


#undef _VLOAD_ADDR_SPACES

/******************************************************************************
* VSTORE
* char, uchar, short, ushort, int, uint, long, ulong, float, double
* n = 2,3,4,8,16
******************************************************************************/
#define _VSTORE_ADDR_SPACES(_ADDR_SPACE) \
_CLC_OVERLOAD _CLC_INLINE void vstore2(uchar2 __data, size_t __offset, _ADDR_SPACE uchar *__p) \
{ _mem2((void*)(__p+(__offset*2))) = as_ushort(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(uchar4 __data, size_t __offset, _ADDR_SPACE uchar *__p) \
{ _mem4((void*)(__p+(__offset*4))) = as_uint(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore8(uchar8 __data, size_t __offset, _ADDR_SPACE uchar *__p) \
{ _memd8((void*)(__p+(__offset*8))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(char2 __data, size_t __offset, _ADDR_SPACE char *__p) \
{ _mem2((void*)(__p+(__offset*2))) = as_ushort(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(char4 __data, size_t __offset, _ADDR_SPACE char *__p) \
{ _mem4((void*)(__p+(__offset*4))) = as_uint(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore8(char8 __data, size_t __offset, _ADDR_SPACE char *__p) \
{ _memd8((void*)(__p+(__offset*8))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(ushort2 __data, size_t __offset, _ADDR_SPACE ushort *__p) \
{ _mem4((void*)(__p+(__offset*2))) = as_uint(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(ushort4 __data, size_t __offset, _ADDR_SPACE ushort *__p) \
{ _memd8((void*)(__p+(__offset*4))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(short2 __data, size_t __offset, _ADDR_SPACE short *__p) \
{ _mem4((void*)(__p+(__offset*2))) = as_uint(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(short4 __data, size_t __offset, _ADDR_SPACE short *__p) \
{ _memd8((void*)(__p+(__offset*4))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(uint2 __data, size_t __offset, _ADDR_SPACE uint *__p) \
{ _memd8((void*)(__p+(__offset*2))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(int2 __data, size_t __offset, _ADDR_SPACE int *__p) \
{ _memd8((void*)(__p+(__offset*2))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(float2 __data, size_t __offset, _ADDR_SPACE float *__p) \
{ _memd8((void*)(__p+(__offset*2))) = as_double(__data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2 (ulong2   __data, size_t __offset, _ADDR_SPACE ulong *__p)\
{ __p[__offset<<1] = __data.s0; __p[1+(__offset<<1)] = __data.s1; }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2 (long2    __data, size_t __offset, _ADDR_SPACE long *__p)\
{ __p[__offset<<1] = __data.s0; __p[1+(__offset<<1)] = __data.s1; }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2 (double2  __data, size_t __offset, _ADDR_SPACE double *__p)\
{ __p[__offset<<1] = __data.s0; __p[1+(__offset<<1)] = __data.s1; }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore16(uchar16  __data, size_t __offset, _ADDR_SPACE uchar *__p)\
{ vstore8(__data.lo, __offset*2, __p); vstore8(__data.hi, __offset*2+1, __p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore16(char16   __data, size_t __offset, _ADDR_SPACE char *__p)\
{ vstore8(__data.lo, __offset*2, __p); vstore8(__data.hi, __offset*2+1, __p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore8 (ushort8  __data, size_t __offset, _ADDR_SPACE ushort *__p)\
{ vstore4(__data.lo, __offset*2, __p); vstore4(__data.hi, __offset*2+1, __p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore8 (short8   __data, size_t __offset, _ADDR_SPACE short *__p)\
{ vstore4(__data.lo, __offset*2, __p); vstore4(__data.hi, __offset*2+1, __p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore4 (uint4    __data, size_t __offset, _ADDR_SPACE uint *__p)\
{ vstore2(__data.lo, __offset*2, __p); vstore2(__data.hi, __offset*2+1, __p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore4 (int4     __data, size_t __offset, _ADDR_SPACE int *__p)\
{ vstore2(__data.lo, __offset*2, __p); vstore2(__data.hi, __offset*2+1, __p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore4 (float4   __data, size_t __offset, _ADDR_SPACE float *__p)\
{ vstore2(__data.lo, __offset*2, __p); vstore2(__data.hi, __offset*2+1, __p); } \

_VSTORE_ADDR_SPACES(__private)
_VSTORE_ADDR_SPACES(__global)
_VSTORE_ADDR_SPACES(__local)

#undef _VSTORE_ADDR_SPACES


int printf(__constant char* restrict __format, ...);

extern __constant const uint kernel_config_l2[32];

_CLC_DECL    size_t  get_local_id     (uint __dim);

_CLC_INLINE  uint    get_work_dim     (void)       { return kernel_config_l2[0];      }
_CLC_INLINE  size_t  get_global_size  (uint __dim) { return kernel_config_l2[1+__dim];  }
_CLC_INLINE  size_t  get_local_size   (uint __dim) { return kernel_config_l2[4+__dim];  }
_CLC_INLINE  size_t  get_global_offset(uint __dim) { return kernel_config_l2[7+__dim];  }
_CLC_INLINE  size_t __get_global_first(uint __dim) { return kernel_config_l2[10+__dim]; }
_CLC_INLINE  size_t  get_num_groups   (uint __dim) { return get_global_size(__dim) / get_local_size(__dim); } 
_CLC_INLINE  size_t  get_global_id    (uint __dim) { return __get_global_first(__dim) + get_local_id(__dim); }
_CLC_INLINE  size_t  get_group_id     (uint __dim) 
                     { return (__get_global_first(__dim) - get_global_offset(__dim)) / get_local_size(__dim); }

/*-----------------------------------------------------------------------------
* Non OpenCL extensions
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_INLINE uint dot(uchar4 __a, uchar4 __b) { return _dotpu4 (as_int(__a), as_int(__b)); }
_CLC_OVERLOAD _CLC_INLINE int  dot(char4  __a, uchar4 __b) { return _dotpsu4(as_int(__a), as_int(__b)); }
_CLC_OVERLOAD _CLC_INLINE int  dot(short2 __a, short2 __b) { return _dotp2  (as_int(__a), as_int(__b)); }

/*-----------------------------------------------------------------------------
* RHADD inline functions for uchar
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_INLINE uchar _rhadd(uchar __a, uchar __b)
{ return _avgu4(__a, __b); }

_CLC_OVERLOAD _CLC_INLINE uchar2 rhadd(uchar2 __a, uchar2 __b)
{ return as_uchar4(_avgu4(as_int((uchar4)(__a,__a)), as_int((uchar4)(__b,__b)))).lo; }

_CLC_OVERLOAD _CLC_INLINE uchar4 rhadd(uchar4 __a, uchar4 __b)
{ return as_uchar4(_avgu4 (as_int(__a), as_int(__b))); }

_CLC_OVERLOAD _CLC_INLINE uchar3 rhadd(uchar3 __a, uchar3 __b)
{ return as_uchar3(rhadd(as_uchar4(__a), as_uchar4(__b))); }

_CLC_OVERLOAD _CLC_INLINE uchar8 rhadd(uchar8 __a, uchar8 __b)
{ return as_uchar8(_davgu4(as_long(__a), as_long(__b))); }

_CLC_OVERLOAD _CLC_INLINE uchar16 rhadd(uchar16 __a, uchar16 __b)
{
    uchar16 __x;
    __x.lo = as_uchar8(_davgu4(as_long(__a.lo), as_long(__b.lo)));
    __x.hi = as_uchar8(_davgu4(as_long(__a.hi), as_long(__b.hi)));
    return __x;
}

_CLC_OVERLOAD _CLC_INLINE short _rhadd(short __a, short __b)
{ return _avg2(__a, __b); }

_CLC_OVERLOAD _CLC_INLINE short2 rhadd(short2 __a, short2 __b)
{ return as_short2(_avg2(as_int(__a), as_int(__b))); }

_CLC_OVERLOAD _CLC_INLINE short4 rhadd(short4 __a, short4 __b)
{ return as_short4(_davg2 (as_ulong(__a), as_ulong(__b))); }

_CLC_OVERLOAD _CLC_INLINE short3 rhadd(short3 __a, short3 __b)
{ return as_short3(rhadd(as_short4(__a), as_short4(__b))); }

_CLC_OVERLOAD _CLC_INLINE short8 rhadd(short8 __a, short8 __b)
{
    short8 __x;
    __x.lo = rhadd(__a.lo, __b.lo);
    __x.hi = rhadd(__a.hi, __b.hi);
    return __x;
}

_CLC_OVERLOAD _CLC_INLINE short16 rhadd(short16 __a, short16 __b)
{
    short16 __x;
    __x.lo = rhadd(__a.lo, __b.lo);
    __x.hi = rhadd(__a.hi, __b.hi);
    return __x;
}


#ifndef _OCL_LIB_BUILD
/*-----------------------------------------------------------------------------
* Selected convert_* BIFs to be inlined
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_INLINE short convert_short_sat(int __x)
{
  __x = clamp(__x, (int)SHRT_MIN, (int)SHRT_MAX);
  return (short)__x;
}

_CLC_OVERLOAD _CLC_INLINE uchar convert_uchar_sat(int __x)
{
  __x = clamp(__x, (int)0, (int)UCHAR_MAX);
  return (uchar)__x;
}

_CLC_OVERLOAD _CLC_INLINE uchar2 convert_uchar2_sat(int2 __x)
{
  return (uchar2)(convert_uchar_sat(__x.lo), convert_uchar_sat(__x.hi));
}

_CLC_OVERLOAD _CLC_INLINE uchar3 convert_uchar3_sat(int3 __x)
{
  return (uchar3)(convert_uchar2_sat(__x.s01), convert_uchar_sat(__x.s2));
}

_CLC_OVERLOAD _CLC_INLINE uchar4 convert_uchar4_sat(int4 __x)
{
  return (uchar4)(convert_uchar2_sat(__x.lo), convert_uchar2_sat(__x.hi));
}

_CLC_OVERLOAD _CLC_INLINE int convert_int(uchar __x)
{
  return (int)__x;
}

_CLC_OVERLOAD _CLC_INLINE short convert_short(int __x)
{
  return (short)__x;
}

_CLC_OVERLOAD _CLC_INLINE uchar convert_uchar(uint __x)
{
  return (uchar)__x;
}

_CLC_OVERLOAD _CLC_INLINE float convert_float(float __x)
{
  return __x;
}

_CLC_OVERLOAD _CLC_INLINE float convert_float(int __x)
{
  return (float)__x;
}

_CLC_OVERLOAD _CLC_INLINE float convert_float(uchar __x)
{
  return (float)__x;
}
#endif

/*-----------------------------------------------------------------------------
* Remaining vload/vstore BIFs
*----------------------------------------------------------------------------*/
#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##3 vload3(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##3)(__x[3*__offset] , __x[3*__offset+1], __x[3*__offset+2]); } \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##4 vload4(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##4)(vload2(__offset<<1, __x), vload2((__offset<<1)+1,__x)); } \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##8 vload8(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##8)(vload4(__offset<<1, __x), vload4((__offset<<1)+1,__x)); } \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##16 vload16(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##16)(vload8(__offset<<1, __x), vload8((__offset<<1)+1,__x)); } \

#define VLOAD_ADDR_SPACES(__CLC_SCALAR_GENTYPE) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __private) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __local) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __constant) \
    VLOAD_VECTORIZE(__CLC_SCALAR_GENTYPE, __global) \

VLOAD_ADDR_SPACES(long)
VLOAD_ADDR_SPACES(ulong)
VLOAD_ADDR_SPACES(double)

#undef VLOAD_VECTORIZE
#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##3 vload3(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##3)(__x[3*__offset] , __x[3*__offset+1], __x[3*__offset+2]); } \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##8 vload8(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##8)(vload4(__offset<<1, __x), vload4((__offset<<1)+1,__x)); } \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##16 vload16(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##16)(vload8(__offset<<1, __x), vload8((__offset<<1)+1,__x)); } \

VLOAD_ADDR_SPACES(int)
VLOAD_ADDR_SPACES(uint)
VLOAD_ADDR_SPACES(float)

#undef VLOAD_VECTORIZE
#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##3 vload3(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##3)(__x[3*__offset] , __x[3*__offset+1], __x[3*__offset+2]); } \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##16 vload16(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##16)(vload8(__offset<<1, __x), vload8((__offset<<1)+1,__x)); } \

VLOAD_ADDR_SPACES(short)
VLOAD_ADDR_SPACES(ushort)

#undef VLOAD_VECTORIZE
#undef VLOAD_VECTORIZE
#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##3 vload3(size_t __offset, const ADDR_SPACE PRIM_TYPE *__x) { \
    return (PRIM_TYPE##3)(__x[3*__offset] , __x[3*__offset+1], __x[3*__offset+2]); } \

VLOAD_ADDR_SPACES(char)
VLOAD_ADDR_SPACES(uchar)

#undef VLOAD_VECTORIZE

#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE void vstore3(PRIM_TYPE##3 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    __mem[3*__offset]   = vec.s0; \
    __mem[(3*__offset)+1] = vec.s1; \
    __mem[(3*__offset)+2] = vec.s2; \
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore4(PRIM_TYPE##4 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    vstore2(vec.lo, __offset<<1, __mem); vstore2(vec.hi, (__offset<<1)+1, __mem);\
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore8(PRIM_TYPE##8 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    vstore4(vec.lo, __offset<<1, __mem); vstore4(vec.hi, (__offset<<1)+1, __mem);\
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore16(PRIM_TYPE##16 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    vstore8(vec.lo, __offset<<1, __mem); vstore8(vec.hi, (__offset<<1)+1, __mem);\
  } \

#define VSTORE_ADDR_SPACES(__CLC_SCALAR___CLC_GENTYPE) \
    VSTORE_VECTORIZE(__CLC_SCALAR___CLC_GENTYPE, __private) \
    VSTORE_VECTORIZE(__CLC_SCALAR___CLC_GENTYPE, __local) \
    VSTORE_VECTORIZE(__CLC_SCALAR___CLC_GENTYPE, __global) \


VSTORE_ADDR_SPACES(long)
VSTORE_ADDR_SPACES(ulong)
VSTORE_ADDR_SPACES(double)

#undef VSTORE_VECTORIZE
#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE void vstore3(PRIM_TYPE##3 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    __mem[3*__offset]   = vec.s0; \
    __mem[(3*__offset)+1] = vec.s1; \
    __mem[(3*__offset)+2] = vec.s2; \
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore8(PRIM_TYPE##8 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    vstore4(vec.lo, __offset<<1, __mem); vstore4(vec.hi, (__offset<<1)+1, __mem);\
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore16(PRIM_TYPE##16 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    vstore8(vec.lo, __offset<<1, __mem); vstore8(vec.hi, (__offset<<1)+1, __mem);\
  } \


VSTORE_ADDR_SPACES(int)
VSTORE_ADDR_SPACES(uint)
VSTORE_ADDR_SPACES(float)

#undef VSTORE_VECTORIZE
#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE void vstore3(PRIM_TYPE##3 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    __mem[3*__offset]   = vec.s0; \
    __mem[(3*__offset)+1] = vec.s1; \
    __mem[(3*__offset)+2] = vec.s2; \
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore16(PRIM_TYPE##16 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    vstore8(vec.lo, __offset<<1, __mem); vstore8(vec.hi, (__offset<<1)+1, __mem);\
  } \

VSTORE_ADDR_SPACES(short)
VSTORE_ADDR_SPACES(ushort)

#undef VSTORE_VECTORIZE
#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE void vstore3(PRIM_TYPE##3 vec, size_t __offset, ADDR_SPACE PRIM_TYPE *__mem) { \
    __mem[3*__offset]   = vec.s0; \
    __mem[(3*__offset)+1] = vec.s1; \
    __mem[(3*__offset)+2] = vec.s2; \
  } \

VSTORE_ADDR_SPACES(char)
VSTORE_ADDR_SPACES(uchar)

#undef VSTORE_VECTORIZE
#undef VSTORE_ADDR_SPACES
#undef VLOAD_ADDR_SPACES

#endif //_DSP_CLC_H_
