/******************************************************************************
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
#ifndef _DSP_MATH_H
#define _DSP_MATH_H

/*-----------------------------------------------------------------------------
* Define helper macros for the builtin functions
*----------------------------------------------------------------------------*/
#define UNARY_VEC(function, type, len) \
       _CLC_OVERLOAD _CLC_DECL type##len function(type##len x)

#define UNARY_SET(function, type) \
    _CLC_OVERLOAD _CLC_DECL type function(type x); \
    UNARY_VEC(function, type, 2); \
    UNARY_VEC(function, type, 3); \
    UNARY_VEC(function, type, 4); \
    UNARY_VEC(function, type, 8); \
    UNARY_VEC(function, type, 16); \

#define UNARY_TYPE(function) \
    UNARY_SET(function, float) \
    UNARY_SET(function, double) \

#define BINARY_VEC(function, type, len) \
       _CLC_OVERLOAD _CLC_DECL type##len function(type##len x, type##len y)

#define BINARY_SET(function, type) \
    _CLC_OVERLOAD _CLC_DECL type function(type x, type y); \
    BINARY_VEC(function, type, 2); \
    BINARY_VEC(function, type, 3); \
    BINARY_VEC(function, type, 4); \
    BINARY_VEC(function, type, 8); \
    BINARY_VEC(function, type, 16); \

#define BINARY_TYPE(function) \
    BINARY_SET(function, float) \
    BINARY_SET(function, double) \

/*-------------------------------------------------------------------------
* Prototypes for the math builtins 
*------------------------------------------------------------------------*/
UNARY_TYPE  (acos)
UNARY_TYPE  (acosh)
UNARY_TYPE  (asin)
UNARY_TYPE  (asinh)
UNARY_TYPE  (atan)
UNARY_TYPE  (atanh)
UNARY_TYPE  (ceil)
UNARY_TYPE  (cos)
UNARY_TYPE  (cosh)
UNARY_TYPE  (exp)
UNARY_TYPE  (exp2)
UNARY_TYPE  (exp10)
UNARY_TYPE  (fabs)
UNARY_TYPE  (floor)
UNARY_TYPE  (log)
UNARY_TYPE  (log2)
UNARY_TYPE  (log10)
UNARY_TYPE  (rsqrt)
UNARY_TYPE  (sin)
UNARY_TYPE  (sinh)
UNARY_TYPE  (sqrt)
UNARY_TYPE  (tan)
UNARY_TYPE  (tanh)

BINARY_TYPE  (atan2)
BINARY_TYPE  (fmod)
BINARY_TYPE  (pow)

BINARY_TYPE  (nextafter)

#undef UNARY_VEC
#undef UNARY_SET
#undef UNARY_TYPE
#undef BINARY_VEC
#undef BINARY_SET
#undef BINARY_TYPE

#endif // _DSP_MATH_H
