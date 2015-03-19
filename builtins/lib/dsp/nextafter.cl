/******************************************************************************
 * Copyright (c) 2011-2013, Peter Collingbourne <peter@pcc.me.uk>
 * Copyright (c) 2011-2013, Texas Instruments Incorporated - http://www.ti.com/
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
#include <clc.h>

// This file provides OpenCL C implementations of nextafter for targets that
// don't support the clang builtin.

#define FLT_NAN 0.0f/0.0f
#define DBL_NAN 0.0/0.0


#define NEXTAFTER(FLOAT_TYPE, UINT_TYPE, NAN, ZERO, NEXTAFTER_ZERO) \
_CLC_OVERLOAD _CLC_DEF FLOAT_TYPE nextafter(FLOAT_TYPE x, FLOAT_TYPE y) { \
  union { FLOAT_TYPE f; UINT_TYPE i; } next; \
  if (isnan(x) || isnan(y)) return NAN; \
  if (x == -MAXFLOAT && y == -INFINITY) return -INFINITY; \
  if (x == MAXFLOAT && y == INFINITY)   return INFINITY; \
  if (x == y) return y;                 \
  next.f = x;                 \
  if (x < y) next.i++;                 \
  else \
  {                    \
    if (next.f == ZERO) next.i = NEXTAFTER_ZERO;  \
    else next.i--;               \
  }                           \
  return next.f;              \
}

NEXTAFTER(float, uint, FLT_NAN, 0.0f, 0x80000001)
_CLC_BINARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, float, nextafter, float, float)

NEXTAFTER(double, ulong, DBL_NAN, 0.0, 0x8000000000000001)
_CLC_BINARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, double, nextafter, double, double)
