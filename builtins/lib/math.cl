/******************************************************************************
 * Copyright (c) 2011-2014, Texas Instruments Incorporated - http://www.ti.com/
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

#define UNARY(function) \
_UNARY_VEC_DEF(float,  float,  function, function) \
_UNARY_VEC_DEF(double, double, function, function) \

#define BINARY(function) \
_BINARY_VEC_DEF(float,  float,  function, function) \
_BINARY_VEC_DEF(double, double, function, function) \

#define TERNARY(function) \
_TERNARY_VEC_DEF(float,  float,  function, function) \
_TERNARY_VEC_DEF(double, double, function, function) \

/*-------------------------------------------------------------------------
* Prototypes for the math builtins 
*------------------------------------------------------------------------*/
UNARY(acos)
UNARY(acosh)
UNARY(acospi)
UNARY(asin)
UNARY(asinh)
UNARY(asinpi) 
UNARY(atan)
BINARY(atan2pi)
UNARY(atanh)
UNARY(atanpi)
BINARY(atan2)
UNARY(cbrt)
UNARY(ceil)
UNARY(cos)
BINARY(copysign)
UNARY(cosh)
UNARY(cospi)
UNARY(erf)
UNARY(erfc)
UNARY(exp)
UNARY(exp2)
UNARY(exp10)
UNARY(expm1)
UNARY(fabs)
BINARY(fdim)
UNARY(floor)
TERNARY(fma)
BINARY(fmax)
BINARY(fmin)
BINARY(fmod)
BINARY(hypot)

_UNARY_VEC_DEF(float,  int, ilogb, ilogbf)
_UNARY_VEC_DEF(double, int, ilogb, ilogbd)

_BINARY_VEC_DEF_ALT(float,  float,  int, ldexp, ldexpf)
_BINARY_VEC_DEF_ALT(double, double, int, ldexp, ldexpd)

UNARY(lgamma)
UNARY(log)
UNARY(log2)
UNARY(log10)
UNARY(log1p)
UNARY(logb)
TERNARY(mad)
BINARY(maxmag)
BINARY(minmag)

_UNARY_VEC_DEF(uint,  float,  nan, nan)
_UNARY_VEC_DEF(ulong, double, nan, nan)

BINARY(nextafter)
BINARY(pow)

_BINARY_VEC_DEF_ALT(float,  float,  int, pown, pownf)
_BINARY_VEC_DEF_ALT(double, double, int, pown, pownd)

BINARY(powr)
BINARY(remainder)
UNARY(rint)

_BINARY_VEC_DEF_ALT(float,  float,  int, rootn, rootnf)
_BINARY_VEC_DEF_ALT(double, double, int, rootn, rootnd)

UNARY(round)
UNARY(rsqrt)
UNARY(sin)
UNARY(sinh)
UNARY(sinpi)
UNARY(sqrt)
UNARY(tan)
UNARY(tanh)
UNARY(tanpi)
UNARY(tgamma)
UNARY(trunc)

// lgamma_r
// modf

/*-----------------------------------------------------------------------------
* Native versions
*----------------------------------------------------------------------------*/
//#define native_sin(x)
//#define native_cos(x)

UNARY(reciprocal)
UNARY(native_recip)
UNARY(native_rsqrt)
UNARY(native_divide)

//#define native_powr(x,y)
//#define native_exp(x)
//#define native_exp2(x)
//#define native_exp10(x)
//#define native_log2(x)
//#define native_log10(x)
//#define native_sqrt(x)
//#define native_tan(x)

