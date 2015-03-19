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
#ifndef _DSP_CLC_H_
#define _DSP_CLC_H_

#include "clc.h"

#if 0
#define event_t __local int*
#else
struct __ocl_event { int dummy; };
typedef struct __ocl_event* __ocl_event_t;
#define event_t __ocl_event_t
#endif


/*-----------------------------------------------------------------------------
* The extended TI built-in function set
*----------------------------------------------------------------------------*/
event_t  __copy_1D1D(event_t event, void *dst, void *src, uint32_t bytes);
event_t  __copy_2D1D(event_t event, void *dst, void *src, uint32_t bytes, uint32_t num_lines, int32_t pitch); 
event_t  __copy_1D2D(event_t event, void *dst, void *src, uint32_t bytes, uint32_t num_lines, int32_t pitch); 
void     __copy_wait(event_t event);
void     __touch(const __global char *p, uint32_t size);


void __touch (const __global char *p, uint32_t size);
#define PREFETCH_VECTORIZE(PRIM_TYPE) \
   _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE    *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##2 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##3 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##4 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##8 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##16 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \

#define PREFETCH_TYPES()           \
        PREFETCH_VECTORIZE(char)   \
        PREFETCH_VECTORIZE(uchar)  \
        PREFETCH_VECTORIZE(short)  \
        PREFETCH_VECTORIZE(ushort) \
        PREFETCH_VECTORIZE(int)    \
        PREFETCH_VECTORIZE(uint)   \
        PREFETCH_VECTORIZE(long)   \
        PREFETCH_VECTORIZE(ulong)  \
        PREFETCH_VECTORIZE(float)  \
        PREFETCH_VECTORIZE(double) \

PREFETCH_TYPES()
    
_CLC_DECL size_t  get_local_id     (uint dim);

_CLC_OVERLOAD _CLC_NODUP _CLC_DECL void wait_group_events(int num_events, event_t *event_list);

#define VEC_TYPE(type,sz) type##sz

#define CROSS_SIZES(type) \
    TEMPLATE(type)              \
    TEMPLATE(VEC_TYPE(type,2))  \
    TEMPLATE(VEC_TYPE(type,3))  \
    TEMPLATE(VEC_TYPE(type,4))  \
    TEMPLATE(VEC_TYPE(type,8))  \
    TEMPLATE(VEC_TYPE(type,16)) \

#define CROSS_TYPES()   \
    CROSS_SIZES(char)   \
    CROSS_SIZES(uchar)  \
    CROSS_SIZES(short)  \
    CROSS_SIZES(ushort) \
    CROSS_SIZES(int)    \
    CROSS_SIZES(uint)   \
    CROSS_SIZES(long)   \
    CROSS_SIZES(ulong)  \
    CROSS_SIZES(float)  \
    CROSS_SIZES(double) \

#define TEMPLATE(gentype) \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_copy(local gentype *dst, \
              const global gentype *src, size_t num_gentypes, event_t event); \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_copy(global gentype *dst, \
              const local gentype *src, size_t num_gentypes, event_t event); \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_copy(global gentype *dst, \
              const global gentype *src, size_t num_gentypes, event_t event); \

CROSS_TYPES()


#undef TEMPLATE
#define TEMPLATE(gentype) \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_strided_copy(local gentype *dst, \
        const global gentype *src, size_t num_gentypes, size_t src_stride, event_t event); \
_CLC_OVERLOAD _CLC_NODUP _CLC_DECL event_t async_work_group_strided_copy(global gentype *dst, \
        const local gentype *src, size_t num_gentypes, size_t dst_stride, event_t event); \

CROSS_TYPES()

#undef VEC_TYPE
#undef CROSS_SIZES
#undef CROSS_TYPES
#undef TEMPLATE


_CLC_OVERLOAD _CLC_DECL char  rotate(char v, char i);
_CLC_OVERLOAD _CLC_DECL uchar rotate(uchar v, uchar i);
_CLC_OVERLOAD _CLC_DECL short  rotate(short v, short i);
_CLC_OVERLOAD _CLC_DECL ushort rotate(ushort v, ushort i);
_CLC_OVERLOAD _CLC_INLINE int  rotate(int v, int i)        { return _rotl(v,i); }
_CLC_OVERLOAD _CLC_INLINE uint rotate(uint v, uint i)      { return _rotl(v,i); }
_CLC_OVERLOAD _CLC_DECL long  rotate(long v, long i);
_CLC_OVERLOAD _CLC_DECL ulong rotate(ulong v, ulong i);

BINARY_VEC_DECL(char, char, rotate)
BINARY_VEC_DECL(uchar, uchar, rotate)
BINARY_VEC_DECL(short, short, rotate)
BINARY_VEC_DECL(ushort, ushort, rotate)
BINARY_VEC_DECL(int, int, rotate)
BINARY_VEC_DECL(uint, uint, rotate)
BINARY_VEC_DECL(long, long, rotate)
BINARY_VEC_DECL(ulong, ulong, rotate)

_CLC_OVERLOAD _CLC_INLINE char   clz(char   v) { return v<0?0: _lmbd(1,v)-24; } 
_CLC_OVERLOAD _CLC_INLINE uchar  clz(uchar  v) { return _lmbd(1, v) - 24; } 
_CLC_OVERLOAD _CLC_INLINE short  clz(short  v) { return v<0?0: _lmbd(1,v)-16; } 
_CLC_OVERLOAD _CLC_INLINE ushort clz(ushort v) { return _lmbd(1, v) - 16; } 
_CLC_OVERLOAD _CLC_INLINE int    clz(int    v) { return _lmbd(1, v); } 
_CLC_OVERLOAD _CLC_INLINE uint   clz(uint   v) { return _lmbd(1, v); } 

_CLC_OVERLOAD _CLC_INLINE long clz(long   v) 
{
    uint2 tmp  = as_uint2(v);
    return tmp.hi ? _lmbd(1, tmp.hi) : _lmbd(1, tmp.lo) + 32;
}

_CLC_OVERLOAD _CLC_INLINE ulong clz(ulong  v)
{
    uint2 tmp  = as_uint2(v);
    return tmp.hi ? _lmbd(1, tmp.hi) : _lmbd(1, tmp.lo) + 32;
}

UNARY_VEC_DECL(char, char, clz)
UNARY_VEC_DECL(uchar, uchar, clz)
UNARY_VEC_DECL(short, short, clz)
UNARY_VEC_DECL(ushort, ushort, clz)
UNARY_VEC_DECL(int, int, clz)
UNARY_VEC_DECL(uint, uint, clz)
UNARY_VEC_DECL(long, long, clz)
UNARY_VEC_DECL(ulong, ulong, clz)


_CLC_OVERLOAD _CLC_INLINE uchar  abs(char x)    { return _abs(x); }
_CLC_OVERLOAD _CLC_INLINE ushort abs(short x)   { return _abs(x); }
_CLC_OVERLOAD _CLC_INLINE uint   abs(int x)     { return _abs(x); }
_CLC_OVERLOAD _CLC_INLINE ulong  abs(long x)    { if (x < 0) x = -x; return x; }

_CLC_OVERLOAD _CLC_INLINE uchar  abs(uchar x)   { return x; }
_CLC_OVERLOAD _CLC_INLINE ushort abs(ushort x)  { return x; }
_CLC_OVERLOAD _CLC_INLINE uint   abs(uint x)    { return x; }
_CLC_OVERLOAD _CLC_INLINE ulong  abs(ulong x)   { return x; }

UNARY_VEC_DECL(char,  uchar,  abs)
UNARY_VEC_DECL(short, ushort, abs)
UNARY_VEC_DECL(int,   uint,   abs)
UNARY_VEC_DECL(long,  ulong,  abs)

/*-----------------------------------------------------------------------------
* ABS for unsigned types is straightforward
*----------------------------------------------------------------------------*/
#define DEFINE(type, utype) \
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(utype,2)  abs(_VEC_TYPE(utype,2) x)  {return x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(utype,3)  abs(_VEC_TYPE(utype,3) x)  {return x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(utype,4)  abs(_VEC_TYPE(utype,4) x)  {return x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(utype,8)  abs(_VEC_TYPE(utype,8) x)  {return x;}\
    _CLC_OVERLOAD _CLC_INLINE _VEC_TYPE(utype,16) abs(_VEC_TYPE(utype,16) x) {return x;}\

DEFINE(uchar, uchar)
DEFINE(ushort, ushort)
DEFINE(uint, uint)
DEFINE(ulong, ulong)

#undef DEFINE

_CLC_OVERLOAD _CLC_DECL long   mul_hi(long x,   long y);
_CLC_OVERLOAD _CLC_DECL ulong  mul_hi(ulong x,   ulong y);

_CLC_OVERLOAD _CLC_INLINE char  mul_hi(char x,  char y)        
{ return _mpy(x,y)  >> 8; }

_CLC_OVERLOAD _CLC_INLINE uchar mul_hi(uchar x, uchar y)       
{ return _mpyu(x,y) >> 8; }

_CLC_OVERLOAD _CLC_INLINE short  mul_hi(short x,  short y)     
{ return _mpy(x,y)  >> 16; }

_CLC_OVERLOAD _CLC_INLINE ushort mul_hi(ushort x, ushort y)    
{ return _mpyu(x,y) >> 16; }

_CLC_OVERLOAD _CLC_INLINE int    mul_hi(int x,    int y)       
{ return ((long)x * (long)y)   >> 32; }

_CLC_OVERLOAD _CLC_INLINE uint   mul_hi(uint x,   uint y)      
{ return ((ulong)x * (ulong)y) >> 32; }

BINARY_VEC_DECL(char, char, mul_hi)
BINARY_VEC_DECL(uchar, uchar, mul_hi)
BINARY_VEC_DECL(short, short, mul_hi)
BINARY_VEC_DECL(ushort, ushort, mul_hi)
BINARY_VEC_DECL(int, int, mul_hi)
BINARY_VEC_DECL(uint, uint, mul_hi)
BINARY_VEC_DECL(long, long, mul_hi)
BINARY_VEC_DECL(ulong, ulong, mul_hi)

_CLC_OVERLOAD _CLC_INLINE char   add_sat(char x, char y)   
{ return _sadd(x<<24, y<<24)>>24; }

_CLC_OVERLOAD _CLC_INLINE uchar  add_sat(uchar x, uchar y) 
{ return _saddu4(x,y); }

_CLC_OVERLOAD _CLC_INLINE short  add_sat(short x, short y) 
{ return _sadd2(x,y); }

_CLC_OVERLOAD _CLC_INLINE ushort add_sat(ushort x, ushort y) 
{ 
    int tmp = x + y;
    if (tmp >> 16) return USHRT_MAX;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE int    add_sat(int x, int y)     
{ return _sadd(x,y); }

_CLC_OVERLOAD _CLC_INLINE uint   add_sat(uint x, uint y)   
{ 
    ulong tmp = (ulong)x + (ulong)y;
    if (tmp >> 32) return UINT_MAX;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE long   add_sat(long x, long y)   
{ 
    if (x > 0 && y > (LONG_MAX-x)) return LONG_MAX;
    if (x < 0 && y < (LONG_MIN-x)) return LONG_MIN;
    return x + y;
}

_CLC_OVERLOAD _CLC_INLINE ulong   add_sat(ulong x, ulong y)   
{ 
    if (y > (ULONG_MAX-x)) return ULONG_MAX;
    return x + y;
}

BINARY_VEC_DECL(char, char, add_sat)
BINARY_VEC_DECL(uchar, uchar, add_sat)
BINARY_VEC_DECL(short, short, add_sat)
BINARY_VEC_DECL(ushort, ushort, add_sat)
BINARY_VEC_DECL(int, int, add_sat)
BINARY_VEC_DECL(uint, uint, add_sat)
BINARY_VEC_DECL(long, long, add_sat)
BINARY_VEC_DECL(ulong, ulong, add_sat)


_CLC_OVERLOAD _CLC_INLINE char   sub_sat(char x, char y)   
{ return _ssub(x<<24, y<<24)>>24; }

_CLC_OVERLOAD _CLC_INLINE uchar  sub_sat(uchar x, uchar y) 
{ 
    if (y > x) return 0;
    return x-y; 
}

_CLC_OVERLOAD _CLC_INLINE short  sub_sat(short x, short y) 
{ return _ssub2(x,y); }

_CLC_OVERLOAD _CLC_INLINE ushort sub_sat(ushort x, ushort y) 
{ 
    if (y > x) return 0;
    return x-y; 
}

_CLC_OVERLOAD _CLC_INLINE int    sub_sat(int x, int y)     
{ return _ssub(x,y); }

_CLC_OVERLOAD _CLC_INLINE uint   sub_sat(uint x, uint y)   
{ 
    if (y > x) return 0;
    return x-y; 
}

_CLC_OVERLOAD _CLC_INLINE long   sub_sat(long x, long y)   
{ 
    if (x > 0 && -y > (LONG_MAX-x)) return LONG_MAX;
    if (x < 0 && -y < (LONG_MIN-x)) return LONG_MIN;

    return x - y;
}

_CLC_OVERLOAD _CLC_INLINE ulong   sub_sat(ulong x, ulong y)   
{ 
    if (y > x) return 0;
    return x-y; 
}

BINARY_VEC_DECL(char, char, sub_sat)
BINARY_VEC_DECL(uchar, uchar, sub_sat)
BINARY_VEC_DECL(short, short, sub_sat)
BINARY_VEC_DECL(ushort, ushort, sub_sat)
BINARY_VEC_DECL(int, int, sub_sat)
BINARY_VEC_DECL(uint, uint, sub_sat)
BINARY_VEC_DECL(long, long, sub_sat)
BINARY_VEC_DECL(ulong, ulong, sub_sat)


_CLC_OVERLOAD _CLC_INLINE short   upsample(char x, uchar y)   
{ return (short)x << 8 | y; }

_CLC_OVERLOAD _CLC_INLINE ushort  upsample(uchar x, uchar y) 
{ return (ushort)x << 8 | y; } 

_CLC_OVERLOAD _CLC_INLINE int     upsample(short x, ushort y) 
{ return (int) _pack2(x,y); }

_CLC_OVERLOAD _CLC_INLINE uint    upsample(ushort x, ushort y) 
{ return (uint) _pack2(x,y); }

_CLC_OVERLOAD _CLC_INLINE long     upsample(int x, uint y) 
{ return (long) _itoll(x,y); }

_CLC_OVERLOAD _CLC_INLINE ulong    upsample(uint x, uint y) 
{ return (ulong) _itoll(x,y); }

BINARY_VEC_DECL_ALT(char,  short,  uchar, upsample)
BINARY_VEC_DECL_ALT(uchar, ushort, uchar, upsample)
BINARY_VEC_DECL_ALT(short,  int,  ushort, upsample)
BINARY_VEC_DECL_ALT(ushort, uint, ushort, upsample)
BINARY_VEC_DECL_ALT(int,  long,  uint, upsample)
BINARY_VEC_DECL_ALT(uint, ulong, uint, upsample)


_CLC_OVERLOAD _CLC_INLINE char mad_sat(char a, char b, char c)
{
    int tmp = _mpy32(a,b);
    tmp += c;

    if (tmp > (int)CHAR_MAX) return CHAR_MAX;
    if (tmp < (int)CHAR_MIN) return CHAR_MIN;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE uchar mad_sat(uchar a, uchar b, uchar c)
{
    uint tmp = _mpy32u(a,b);
    tmp += c;

    if (tmp > (uint)UCHAR_MAX) return UCHAR_MAX;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE short mad_sat(short a, short b, short c)
{
    int tmp = _mpy32(a,b);
    tmp += c;

    if (tmp > (int)SHRT_MAX) return SHRT_MAX;
    if (tmp < (int)SHRT_MIN) return SHRT_MIN;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE ushort mad_sat(ushort a, ushort b, ushort c)
{
    uint tmp = _mpy32u(a,b);
    tmp += c;

    if (tmp > (uint)USHRT_MAX) return USHRT_MAX;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE int mad_sat(int a, int b, int c)
{
    long tmp = (long)a * (long)b + (long)c;
    if (tmp > (long)INT_MAX) return INT_MAX;
    if (tmp < (long)INT_MIN) return INT_MIN;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE uint mad_sat(uint a, uint b, uint c)
{
    ulong tmp = _mpy32u(a,b);
    tmp += c;

    if (tmp > (ulong)UINT_MAX) return UINT_MAX;
    return tmp;
}

_CLC_OVERLOAD _CLC_INLINE long mad_sat(long a, long b, long c)
{
    if (a > 0 && b > 0 && a > (LONG_MAX/b)) return LONG_MAX;
    if (a > 0 && b < 0 && b < (LONG_MIN/a)) return LONG_MIN;
    if (a < 0 && b > 0 && a < (LONG_MIN/b)) return LONG_MIN;
    if (a < 0 && b < 0 && b < (LONG_MAX/a)) return LONG_MAX;

    return add_sat(a*b, c);
}

_CLC_OVERLOAD _CLC_INLINE ulong mad_sat(ulong a, ulong b, ulong c)
{
    if (a > (ULONG_MAX/b)) return ULONG_MAX;
    return add_sat(a*b, c);
}

TERNARY_VEC_DECL(char, char, mad_sat)
TERNARY_VEC_DECL(uchar, uchar, mad_sat)
TERNARY_VEC_DECL(short, short, mad_sat)
TERNARY_VEC_DECL(ushort, ushort, mad_sat)
TERNARY_VEC_DECL(int, int, mad_sat)
TERNARY_VEC_DECL(uint, uint, mad_sat)
TERNARY_VEC_DECL(long, long, mad_sat)
TERNARY_VEC_DECL(ulong, ulong, mad_sat)


int printf(constant char* restrict _format, ...);

extern constant const uint kernel_config_l2[32];

_CLC_DECL    size_t  get_local_id     (uint dim);

_CLC_INLINE  uint    get_work_dim     (void)     { return kernel_config_l2[0];      }
_CLC_INLINE  size_t  get_global_size  (uint dim) { return kernel_config_l2[1+dim];  }
_CLC_INLINE  size_t  get_local_size   (uint dim) { return kernel_config_l2[4+dim];  }
_CLC_INLINE  size_t  get_global_offset(uint dim) { return kernel_config_l2[7+dim];  }
_CLC_INLINE  size_t __get_global_first(uint dim) { return kernel_config_l2[10+dim]; }
_CLC_INLINE  size_t  get_num_groups   (uint dim) { return get_global_size(dim) / get_local_size(dim); } 
_CLC_INLINE  size_t  get_global_id    (uint dim) { return __get_global_first(dim) + get_local_id(dim); }
_CLC_INLINE  size_t  get_group_id     (uint dim) 
                     { return (__get_global_first(dim) - get_global_offset(dim)) / get_local_size(dim); }

#endif //_DSP_CLC_H_

