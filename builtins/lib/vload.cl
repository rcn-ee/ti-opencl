/******************************************************************************
 * Copyright (c) 2011-2014, Peter Collingbourne <peter@pcc.me.uk>
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
#include "clc.h"

#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##3 vload3(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##3)(x[3*offset] , x[3*offset+1], x[3*offset+2]); } \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##4 vload4(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##4)(vload2(offset<<1, x), vload2((offset<<1)+1,x)); } \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##8 vload8(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##8)(vload4(offset<<1, x), vload4((offset<<1)+1,x)); } \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##16 vload16(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##16)(vload8(offset<<1, x), vload8((offset<<1)+1,x)); } \

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
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##3 vload3(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##3)(x[3*offset] , x[3*offset+1], x[3*offset+2]); } \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##8 vload8(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##8)(vload4(offset<<1, x), vload4((offset<<1)+1,x)); } \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##16 vload16(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##16)(vload8(offset<<1, x), vload8((offset<<1)+1,x)); } \

VLOAD_ADDR_SPACES(int)
VLOAD_ADDR_SPACES(uint)
VLOAD_ADDR_SPACES(float)

#undef VLOAD_VECTORIZE
#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##3 vload3(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##3)(x[3*offset] , x[3*offset+1], x[3*offset+2]); } \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##16 vload16(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##16)(vload8(offset<<1, x), vload8((offset<<1)+1,x)); } \

VLOAD_ADDR_SPACES(short)
VLOAD_ADDR_SPACES(ushort)

#undef VLOAD_VECTORIZE
#undef VLOAD_VECTORIZE
#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_DEF PRIM_TYPE##3 vload3(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##3)(x[3*offset] , x[3*offset+1], x[3*offset+2]); } \

VLOAD_ADDR_SPACES(char)
VLOAD_ADDR_SPACES(uchar)

#undef VLOAD_VECTORIZE

#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_DEF void vstore3(PRIM_TYPE##3 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    mem[3*offset]   = vec.s0; \
    mem[(3*offset)+1] = vec.s1; \
    mem[(3*offset)+2] = vec.s2; \
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore4(PRIM_TYPE##4 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore2(vec.lo, offset<<1, mem); vstore2(vec.hi, (offset<<1)+1, mem);\
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore8(PRIM_TYPE##8 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore4(vec.lo, offset<<1, mem); vstore4(vec.hi, (offset<<1)+1, mem);\
  } \
  _CLC_OVERLOAD _CLC_DEF void vstore16(PRIM_TYPE##16 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore8(vec.lo, offset<<1, mem); vstore8(vec.hi, (offset<<1)+1, mem);\
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
  _CLC_OVERLOAD _CLC_DEF void vstore3(PRIM_TYPE##3 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    mem[3*offset]   = vec.s0; \
    mem[(3*offset)+1] = vec.s1; \
    mem[(3*offset)+2] = vec.s2; \
  } \
  _CLC_OVERLOAD _CLC_INLINE void vstore8(PRIM_TYPE##8 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore4(vec.lo, offset<<1, mem); vstore4(vec.hi, (offset<<1)+1, mem);\
  } \
  _CLC_OVERLOAD _CLC_DEF void vstore16(PRIM_TYPE##16 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore8(vec.lo, offset<<1, mem); vstore8(vec.hi, (offset<<1)+1, mem);\
  } \


VSTORE_ADDR_SPACES(int)
VSTORE_ADDR_SPACES(uint)
VSTORE_ADDR_SPACES(float)

#undef VSTORE_VECTORIZE
#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_DEF void vstore3(PRIM_TYPE##3 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    mem[3*offset]   = vec.s0; \
    mem[(3*offset)+1] = vec.s1; \
    mem[(3*offset)+2] = vec.s2; \
  } \
  _CLC_OVERLOAD _CLC_DEF void vstore16(PRIM_TYPE##16 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore8(vec.lo, offset<<1, mem); vstore8(vec.hi, (offset<<1)+1, mem);\
  } \

VSTORE_ADDR_SPACES(short)
VSTORE_ADDR_SPACES(ushort)

#undef VSTORE_VECTORIZE
#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_DEF void vstore3(PRIM_TYPE##3 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    mem[3*offset]   = vec.s0; \
    mem[(3*offset)+1] = vec.s1; \
    mem[(3*offset)+2] = vec.s2; \
  } \

VSTORE_ADDR_SPACES(char)
VSTORE_ADDR_SPACES(uchar)

#undef VSTORE_VECTORIZE
#undef VSTORE_ADDR_SPACES
#undef VLOAD_ADDR_SPACES
