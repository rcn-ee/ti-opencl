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
#ifndef _VLOAD_H_
#define _VLOAD_H_

#define VLOAD_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##2 vload2(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##2)(x[2*offset] , x[2*offset+1]); \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##3 vload3(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##3)(x[3*offset] , x[3*offset+1], x[3*offset+2]); \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##4 vload4(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##4)(x[4*offset], x[4*offset+1], x[4*offset+2], x[4*offset+3]); \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##8 vload8(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##8)(vload4(0, &x[8*offset]), vload4(1, &x[8*offset])); \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE PRIM_TYPE##16 vload16(size_t offset, const ADDR_SPACE PRIM_TYPE *x) { \
    return (PRIM_TYPE##16)(vload8(0, &x[16*offset]), vload8(1, &x[16*offset])); \
  } \

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

#define VSTORE_VECTORIZE(PRIM_TYPE, ADDR_SPACE) \
  _CLC_OVERLOAD _CLC_INLINE void vstore2(PRIM_TYPE##2 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    mem[2*offset] = vec.s0; \
    mem[2*offset+1] = vec.s1; \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE void vstore3(PRIM_TYPE##3 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    mem[3*offset] = vec.s0; \
    mem[3*offset+1] = vec.s1; \
    mem[3*offset+2] = vec.s2; \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE void vstore4(PRIM_TYPE##4 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore2(vec.lo, 0, &mem[offset*4]); \
    vstore2(vec.hi, 1, &mem[offset*4]); \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE void vstore8(PRIM_TYPE##8 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore4(vec.lo, 0, &mem[offset*8]); \
    vstore4(vec.hi, 1, &mem[offset*8]); \
  } \
\
  _CLC_OVERLOAD _CLC_INLINE void vstore16(PRIM_TYPE##16 vec, size_t offset, ADDR_SPACE PRIM_TYPE *mem) { \
    vstore8(vec.lo, 0, &mem[offset*16]); \
    vstore8(vec.hi, 1, &mem[offset*16]); \
  } \

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

#endif // _VLOAD_H_
