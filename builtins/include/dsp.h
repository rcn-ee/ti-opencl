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

/******************************************************************************
* VLOAD
* char, uchar, short, ushort, int, uint, long, ulong, float, double
* n = 2,3,4,8,16
******************************************************************************/
#define VLOAD_ADDR_SPACES(ADDR_SPACE) \
_CLC_OVERLOAD _CLC_INLINE uchar2 vload2(size_t offset, ADDR_SPACE const uchar *p) \
{ return as_uchar2(_mem2((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE uchar4 vload4(size_t offset, ADDR_SPACE const uchar *p) \
{ return as_uchar4(_mem4((void*)(p+(offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE uchar8 vload8(size_t offset, ADDR_SPACE const uchar *p) \
{ return as_uchar8(_memd8((void*)(p+(offset*8)))); }\
\
_CLC_OVERLOAD _CLC_INLINE char2 vload2(size_t offset, ADDR_SPACE const char *p) \
{ return as_char2(_mem2((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE char4 vload4(size_t offset, ADDR_SPACE const char *p) \
{ return as_char4(_mem4((void*)(p+(offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE char8 vload8(size_t offset, ADDR_SPACE const char *p) \
{ return as_char8(_memd8((void*)(p+(offset*8)))); }\
\
_CLC_OVERLOAD _CLC_INLINE ushort2 vload2(size_t offset, ADDR_SPACE const ushort *p) \
{ return as_ushort2(_mem4((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE ushort4 vload4(size_t offset, ADDR_SPACE const ushort *p) \
{ return as_ushort4(_memd8((void*)(p+(offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE short2 vload2(size_t offset, ADDR_SPACE const short *p) \
{ return as_short2(_mem4((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE short4 vload4(size_t offset, ADDR_SPACE const short *p) \
{ return as_short4(_memd8((void*)(p+(offset*4)))); }\
\
_CLC_OVERLOAD _CLC_INLINE uint2 vload2(size_t offset, ADDR_SPACE const uint *p) \
{ return as_uint2(_memd8((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE int2 vload2(size_t offset, ADDR_SPACE const int *p) \
{ return as_int2(_memd8((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE float2 vload2(size_t offset, ADDR_SPACE const float *p) \
{ return as_float2(_memd8((void*)(p+(offset*2)))); }\
\
_CLC_OVERLOAD _CLC_INLINE ulong2   vload2 (size_t offset, ADDR_SPACE const ulong *p) \
{ return (ulong2)( as_ulong(_memd8((void*)(p+(offset*2)))), as_ulong(_memd8((void*)(p+1+(offset*2))))); }\
\
_CLC_OVERLOAD _CLC_INLINE long2    vload2 (size_t offset, ADDR_SPACE const long *p) \
{ return (long2)( as_long(_memd8((void*)(p+(offset*2)))), as_long(_memd8((void*)(p+1+(offset*2))))); }\
\
_CLC_OVERLOAD _CLC_INLINE double2  vload2 (size_t offset, ADDR_SPACE const double *p) \
{ return (double2)( as_double(_memd8((void*)(p+(offset*2)))), as_double(_memd8((void*)(p+1+(offset*2))))); }\
\
_CLC_OVERLOAD _CLC_INLINE uchar16  vload16(size_t offset, ADDR_SPACE const uchar *p)\
{ return (uchar16)(vload8(offset*2, p), vload8(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_INLINE char16   vload16(size_t offset, ADDR_SPACE const char *p)\
{ return (char16)(vload8(offset*2, p), vload8(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_INLINE ushort8  vload8 (size_t offset, ADDR_SPACE const ushort *p)\
{ return (ushort8)(vload4(offset*2, p), vload4(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_INLINE short8   vload8 (size_t offset, ADDR_SPACE const short *p)\
{ return (short8)(vload4(offset*2, p), vload4(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_INLINE uint4    vload4 (size_t offset, ADDR_SPACE const uint *p)\
{ return (uint4)(vload2(offset*2, p), vload2(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_INLINE int4     vload4 (size_t offset, ADDR_SPACE const int *p)\
{ return (int4)(vload2(offset*2, p), vload2(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_INLINE float4   vload4 (size_t offset, ADDR_SPACE const float *p)\
{ return (float4)(vload2(offset*2, p), vload2(offset*2+1, p)); }\
\
_CLC_OVERLOAD _CLC_DECL ushort16 vload16(size_t offset, ADDR_SPACE const ushort *p);\
_CLC_OVERLOAD _CLC_DECL short16  vload16(size_t offset, ADDR_SPACE const short *p);\
_CLC_OVERLOAD _CLC_DECL uint8    vload8 (size_t offset, ADDR_SPACE const uint *p);\
_CLC_OVERLOAD _CLC_DECL uint16   vload16(size_t offset, ADDR_SPACE const uint *p);\
_CLC_OVERLOAD _CLC_DECL int8     vload8 (size_t offset, ADDR_SPACE const int *p);\
_CLC_OVERLOAD _CLC_DECL int16    vload16(size_t offset, ADDR_SPACE const int *p);\
_CLC_OVERLOAD _CLC_DECL float8   vload8 (size_t offset, ADDR_SPACE const float *p);\
_CLC_OVERLOAD _CLC_DECL float16  vload16(size_t offset, ADDR_SPACE const float *p);\
_CLC_OVERLOAD _CLC_DECL ulong4   vload4 (size_t offset, ADDR_SPACE const ulong *p);\
_CLC_OVERLOAD _CLC_DECL ulong8   vload8 (size_t offset, ADDR_SPACE const ulong *p);\
_CLC_OVERLOAD _CLC_DECL ulong16  vload16(size_t offset, ADDR_SPACE const ulong *p);\
_CLC_OVERLOAD _CLC_DECL long4    vload4 (size_t offset, ADDR_SPACE const long *p);\
_CLC_OVERLOAD _CLC_DECL long8    vload8 (size_t offset, ADDR_SPACE const long *p);\
_CLC_OVERLOAD _CLC_DECL long16   vload16(size_t offset, ADDR_SPACE const long *p);\
_CLC_OVERLOAD _CLC_DECL double4  vload4 (size_t offset, ADDR_SPACE const double *p);\
_CLC_OVERLOAD _CLC_DECL double8  vload8 (size_t offset, ADDR_SPACE const double *p);\
_CLC_OVERLOAD _CLC_DECL double16 vload16(size_t offset, ADDR_SPACE const double *p);\
\
_CLC_OVERLOAD _CLC_DECL uchar3   vload3 (size_t offset, ADDR_SPACE const uchar *p);\
_CLC_OVERLOAD _CLC_DECL char3    vload3 (size_t offset, ADDR_SPACE const char *p);\
_CLC_OVERLOAD _CLC_DECL ushort3  vload3 (size_t offset, ADDR_SPACE const ushort *p);\
_CLC_OVERLOAD _CLC_DECL short3   vload3 (size_t offset, ADDR_SPACE const short *p);\
_CLC_OVERLOAD _CLC_DECL uint3    vload3 (size_t offset, ADDR_SPACE const uint *p);\
_CLC_OVERLOAD _CLC_DECL int3     vload3 (size_t offset, ADDR_SPACE const int *p);\
_CLC_OVERLOAD _CLC_DECL float3   vload3 (size_t offset, ADDR_SPACE const float *p);\
_CLC_OVERLOAD _CLC_DECL ulong3   vload3 (size_t offset, ADDR_SPACE const ulong *p);\
_CLC_OVERLOAD _CLC_DECL long3    vload3 (size_t offset, ADDR_SPACE const long *p);\
_CLC_OVERLOAD _CLC_DECL double3  vload3 (size_t offset, ADDR_SPACE const double *p);\

VLOAD_ADDR_SPACES(private)
VLOAD_ADDR_SPACES(constant)
VLOAD_ADDR_SPACES(global)
VLOAD_ADDR_SPACES(local)


#undef VLOAD_ADDR_SPACES

/******************************************************************************
* VSTORE
* char, uchar, short, ushort, int, uint, long, ulong, float, double
* n = 2,3,4,8,16
******************************************************************************/
#define VSTORE_ADDR_SPACES(ADDR_SPACE) \
_CLC_OVERLOAD _CLC_INLINE void vstore2(uchar2 data, size_t offset, ADDR_SPACE uchar *p) \
{ _mem2((void*)(p+(offset*2))) = as_ushort(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(uchar4 data, size_t offset, ADDR_SPACE uchar *p) \
{ _mem4((void*)(p+(offset*4))) = as_uint(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore8(uchar8 data, size_t offset, ADDR_SPACE uchar *p) \
{ _memd8((void*)(p+(offset*8))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(char2 data, size_t offset, ADDR_SPACE char *p) \
{ _mem2((void*)(p+(offset*2))) = as_ushort(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(char4 data, size_t offset, ADDR_SPACE char *p) \
{ _mem4((void*)(p+(offset*4))) = as_uint(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore8(char8 data, size_t offset, ADDR_SPACE char *p) \
{ _memd8((void*)(p+(offset*8))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(ushort2 data, size_t offset, ADDR_SPACE ushort *p) \
{ _mem4((void*)(p+(offset*2))) = as_uint(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(ushort4 data, size_t offset, ADDR_SPACE ushort *p) \
{ _memd8((void*)(p+(offset*4))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(short2 data, size_t offset, ADDR_SPACE short *p) \
{ _mem4((void*)(p+(offset*2))) = as_uint(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore4(short4 data, size_t offset, ADDR_SPACE short *p) \
{ _memd8((void*)(p+(offset*4))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(uint2 data, size_t offset, ADDR_SPACE uint *p) \
{ _memd8((void*)(p+(offset*2))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(int2 data, size_t offset, ADDR_SPACE int *p) \
{ _memd8((void*)(p+(offset*2))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2(float2 data, size_t offset, ADDR_SPACE float *p) \
{ _memd8((void*)(p+(offset*2))) = as_double(data); }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2 (ulong2   data, size_t offset, ADDR_SPACE ulong *p)\
{ p[offset<<1] = data.s0; p[1+(offset<<1)] = data.s1; }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2 (long2    data, size_t offset, ADDR_SPACE long *p)\
{ p[offset<<1] = data.s0; p[1+(offset<<1)] = data.s1; }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore2 (double2  data, size_t offset, ADDR_SPACE double *p)\
{ p[offset<<1] = data.s0; p[1+(offset<<1)] = data.s1; }\
\
_CLC_OVERLOAD _CLC_INLINE void vstore16(uchar16  data, size_t offset, ADDR_SPACE uchar *p)\
{ vstore8(data.lo, offset*2, p); vstore8(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore16(char16   data, size_t offset, ADDR_SPACE char *p)\
{ vstore8(data.lo, offset*2, p); vstore8(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore8 (ushort8  data, size_t offset, ADDR_SPACE ushort *p)\
{ vstore4(data.lo, offset*2, p); vstore4(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore8 (short8   data, size_t offset, ADDR_SPACE short *p)\
{ vstore4(data.lo, offset*2, p); vstore4(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore4 (uint4    data, size_t offset, ADDR_SPACE uint *p)\
{ vstore2(data.lo, offset*2, p); vstore2(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore4 (int4     data, size_t offset, ADDR_SPACE int *p)\
{ vstore2(data.lo, offset*2, p); vstore2(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_INLINE void vstore4 (float4   data, size_t offset, ADDR_SPACE float *p)\
{ vstore2(data.lo, offset*2, p); vstore2(data.hi, offset*2+1, p); } \
\
_CLC_OVERLOAD _CLC_DECL void vstore16(ushort16 data, size_t offset, ADDR_SPACE ushort *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(short16  data, size_t offset, ADDR_SPACE short *p);\
_CLC_OVERLOAD _CLC_DECL void vstore8 (uint8    data, size_t offset, ADDR_SPACE uint *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(uint16   data, size_t offset, ADDR_SPACE uint *p);\
_CLC_OVERLOAD _CLC_DECL void vstore8 (int8     data, size_t offset, ADDR_SPACE int *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(int16    data, size_t offset, ADDR_SPACE int *p);\
_CLC_OVERLOAD _CLC_DECL void vstore8 (float8   data, size_t offset, ADDR_SPACE float *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(float16  data, size_t offset, ADDR_SPACE float *p);\
_CLC_OVERLOAD _CLC_DECL void vstore4 (ulong4   data, size_t offset, ADDR_SPACE ulong *p);\
_CLC_OVERLOAD _CLC_DECL void vstore8 (ulong8   data, size_t offset, ADDR_SPACE ulong *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(ulong16  data, size_t offset, ADDR_SPACE ulong *p);\
_CLC_OVERLOAD _CLC_DECL void vstore4 (long4    data, size_t offset, ADDR_SPACE long *p);\
_CLC_OVERLOAD _CLC_DECL void vstore8 (long8    data, size_t offset, ADDR_SPACE long *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(long16   data, size_t offset, ADDR_SPACE long *p);\
_CLC_OVERLOAD _CLC_DECL void vstore4 (double4  data, size_t offset, ADDR_SPACE double *p);\
_CLC_OVERLOAD _CLC_DECL void vstore8 (double8  data, size_t offset, ADDR_SPACE double *p);\
_CLC_OVERLOAD _CLC_DECL void vstore16(double16 data, size_t offset, ADDR_SPACE double *p);\
\
_CLC_OVERLOAD _CLC_DECL void vstore3 (uchar3   data, size_t offset, ADDR_SPACE uchar *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (char3    data, size_t offset, ADDR_SPACE char *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (ushort3  data, size_t offset, ADDR_SPACE ushort *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (short3   data, size_t offset, ADDR_SPACE short *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (uint3    data, size_t offset, ADDR_SPACE uint *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (int3     data, size_t offset, ADDR_SPACE int *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (float3   data, size_t offset, ADDR_SPACE float *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (ulong3   data, size_t offset, ADDR_SPACE ulong *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (long3    data, size_t offset, ADDR_SPACE long *p);\
_CLC_OVERLOAD _CLC_DECL void vstore3 (double3  data, size_t offset, ADDR_SPACE double *p);\

VSTORE_ADDR_SPACES(private)
VSTORE_ADDR_SPACES(global)
VSTORE_ADDR_SPACES(local)

#undef VSTORE_ADDR_SPACES


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

/*-----------------------------------------------------------------------------
* Non OpenCL extensions
*----------------------------------------------------------------------------*/
_CLC_OVERLOAD _CLC_INLINE uint dot(uchar4 a, uchar4 b) { return _dotpu4 (as_int(a), as_int(b)); }
_CLC_OVERLOAD _CLC_INLINE int  dot(char4  a, uchar4 b) { return _dotpsu4(as_int(a), as_int(b)); }
_CLC_OVERLOAD _CLC_INLINE int  dot(short2 a, short2 b) { return _dotp2  (as_int(a), as_int(b)); }

#endif //_DSP_CLC_H_
