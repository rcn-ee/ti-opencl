/******************************************************************************
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

/*-----------------------------------------------------------------------------
* Define helper macros for the builtin functions
*----------------------------------------------------------------------------*/
#define UNARY_SCALAR(function, type, suffix) \
type function##suffix(type x); \
_CLC_OVERLOAD _CLC_DEF type function(type x) \
{ \
        type result;\
        result = function##suffix(x); \
        return result;\
} \

#define UNARY_VEC2(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##2 function(type##2 x) \
{ \
        type##2 result;\
        result.s0 = function##suffix(x.s0); \
        result.s1 = function##suffix(x.s1); \
        return result;\
} \

#define UNARY_VEC3(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##3 function(type##3 x) \
{ \
        type##3 result;\
        result.s0 = function##suffix(x.s0); \
        result.s1 = function##suffix(x.s1); \
        result.s2 = function##suffix(x.s2); \
        return result;\
} \

#define UNARY_VEC4(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##4 function(type##4 x) \
{ \
        type##4 result;\
        result.s0 = function##suffix(x.s0); \
        result.s1 = function##suffix(x.s1); \
        result.s2 = function##suffix(x.s2); \
        result.s3 = function##suffix(x.s3); \
        return result;\
} \

#define UNARY_VEC8(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##8 function(type##8 x) \
{ \
        type##8 result;\
        result.s0 = function##suffix(x.s0); \
        result.s1 = function##suffix(x.s1); \
        result.s2 = function##suffix(x.s2); \
        result.s3 = function##suffix(x.s3); \
        result.s4 = function##suffix(x.s4); \
        result.s5 = function##suffix(x.s5); \
        result.s6 = function##suffix(x.s6); \
        result.s7 = function##suffix(x.s7); \
        return result;\
} \

#define UNARY_VEC16(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##16 function(type##16 x) \
{ \
        type##16 result;\
        result.s0 = function##suffix(x.s0); \
        result.s1 = function##suffix(x.s1); \
        result.s2 = function##suffix(x.s2); \
        result.s3 = function##suffix(x.s3); \
        result.s4 = function##suffix(x.s4); \
        result.s5 = function##suffix(x.s5); \
        result.s6 = function##suffix(x.s6); \
        result.s7 = function##suffix(x.s7); \
        result.s8 = function##suffix(x.s8); \
        result.s9 = function##suffix(x.s9); \
        result.sa = function##suffix(x.sa); \
        result.sb = function##suffix(x.sb); \
        result.sc = function##suffix(x.sc); \
        result.sd = function##suffix(x.sd); \
        result.se = function##suffix(x.se); \
        result.sf = function##suffix(x.sf); \
        return result;\
} \

#define UNARY_OPERATION(function, type, suffix) \
    UNARY_SCALAR(function, type, suffix) \
    UNARY_VEC2  (function, type, suffix) \
    UNARY_VEC3  (function, type, suffix) \
    UNARY_VEC4  (function, type, suffix) \
    UNARY_VEC8  (function, type, suffix) \
    UNARY_VEC16 (function, type, suffix) \

#define UNARY_TYPE(function) \
    UNARY_OPERATION(function, float, f) \
    UNARY_OPERATION(function, double, d) \


#define BINARY_OPERATION(function, type, suffix) \
    BINARY_SCALAR(function, type, suffix) \
    BINARY_VEC2  (function, type, suffix) \
    BINARY_VEC3  (function, type, suffix) \
    BINARY_VEC4  (function, type, suffix) \
    BINARY_VEC8  (function, type, suffix) \
    BINARY_VEC16 (function, type, suffix) \

#define BINARY_SCALAR(function, type, suffix) \
type function##suffix(type x, type y); \
_CLC_OVERLOAD _CLC_DEF type function(type x, type y) \
{ \
        type result;\
        result = function##suffix(x, y); \
        return result;\
} \

#define BINARY_VEC2(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##2 function(type##2 x, type##2 y) \
{ \
        type##2 result;\
        result.s0 = function##suffix(x.s0, y.s0); \
        result.s1 = function##suffix(x.s1, y.s0); \
        return result;\
} \

#define BINARY_VEC3(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##3 function(type##3 x, type##3 y) \
{ \
        type##3 result;\
        result.s0 = function##suffix(x.s0, y.s0); \
        result.s1 = function##suffix(x.s1, y.s0); \
        result.s2 = function##suffix(x.s2, y.s0); \
        return result;\
} \

#define BINARY_VEC4(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##4 function(type##4 x, type##4 y) \
{ \
        type##4 result;\
        result.s0 = function##suffix(x.s0, y.s0); \
        result.s1 = function##suffix(x.s1, y.s0); \
        result.s2 = function##suffix(x.s2, y.s0); \
        result.s3 = function##suffix(x.s3, y.s0); \
        return result;\
} \

#define BINARY_VEC8(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##8 function(type##8 x, type##8 y) \
{ \
        type##8 result;\
        result.s0 = function##suffix(x.s0, y.s0); \
        result.s1 = function##suffix(x.s1, y.s0); \
        result.s2 = function##suffix(x.s2, y.s0); \
        result.s3 = function##suffix(x.s3, y.s0); \
        result.s4 = function##suffix(x.s4, y.s0); \
        result.s5 = function##suffix(x.s5, y.s0); \
        result.s6 = function##suffix(x.s6, y.s0); \
        result.s7 = function##suffix(x.s7, y.s0); \
        return result;\
} \

#define BINARY_VEC16(function, type, suffix) \
_CLC_OVERLOAD _CLC_DEF type##16 function(type##16 x, type##16 y) \
{ \
        type##16 result;\
        result.s0 = function##suffix(x.s0, y.s0); \
        result.s1 = function##suffix(x.s1, y.s1); \
        result.s2 = function##suffix(x.s2, y.s2); \
        result.s3 = function##suffix(x.s3, y.s3); \
        result.s4 = function##suffix(x.s4, y.s4); \
        result.s5 = function##suffix(x.s5, y.s5); \
        result.s6 = function##suffix(x.s6, y.s6); \
        result.s7 = function##suffix(x.s7, y.s7); \
        result.s8 = function##suffix(x.s8, y.s8); \
        result.s9 = function##suffix(x.s9, y.s9); \
        result.sa = function##suffix(x.sa, y.sa); \
        result.sb = function##suffix(x.sb, y.sb); \
        result.sc = function##suffix(x.sc, y.sc); \
        result.sd = function##suffix(x.sd, y.sd); \
        result.se = function##suffix(x.se, y.se); \
        result.sf = function##suffix(x.sf, y.sf); \
        return result;\
} \

#define BINARY_TYPE(function) \
    BINARY_OPERATION(function, float, f) \
    BINARY_OPERATION(function, double, d) \

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

BINARY_TYPE (atan2)
BINARY_TYPE (fmod)
BINARY_TYPE (pow)

#undef UNARY_SCALAR
#undef UNARY_VEC2
#undef UNARY_VEC3
#undef UNARY_VEC4
#undef UNARY_VEC8
#undef UNARY_VEC16
#undef UNARY_OPERATION
#undef UNARY_TYPE

#undef BINARY_SCALAR
#undef BINARY_VEC2
#undef BINARY_VEC3
#undef BINARY_VEC4
#undef BINARY_VEC8
#undef BINARY_VEC16
#undef BINARY_OPERATION
#undef BINARY_TYPE
