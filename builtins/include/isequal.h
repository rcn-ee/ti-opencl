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
#ifndef _ISEQUAL_H_
#define _ISEQUAL_H_

#include <clc.h>

#define BLD_CONVERT(name, type) name##type

#define EXPAND_SIZES(type)                  \
    SCALAR_IMPLEMENTATION(type, int)        \
    IMPLEMENTATION(_VEC_TYPE(type,2), _VEC_TYPE(int,2))  \
    IMPLEMENTATION(_VEC_TYPE(type,3), _VEC_TYPE(int,3))  \
    IMPLEMENTATION(_VEC_TYPE(type,4), _VEC_TYPE(int,4))  \
    IMPLEMENTATION(_VEC_TYPE(type,8), _VEC_TYPE(int,8))  \
    IMPLEMENTATION(_VEC_TYPE(type,16), _VEC_TYPE(int,16)) \

#define DECLARATION(gentype, rettype) \
_CLC_OVERLOAD _CLC_DECL rettype isequal(gentype x, gentype y); \

#define IMPLEMENTATION(gentype, rettype) \
_CLC_OVERLOAD _CLC_INLINE rettype isequal(gentype x, gentype y)  \
    { return BLD_CONVERT(convert_, rettype)(x == y); } \

#define SCALAR_IMPLEMENTATION(gentype, rettype) \
_CLC_OVERLOAD _CLC_INLINE rettype isequal(gentype x, gentype y)  \
    { return BLD_CONVERT(convert_, rettype)(x == y); } \

_EXPAND_INTEGER_TYPES()
EXPAND_SIZES(float)
SCALAR_IMPLEMENTATION(double, int)        \
IMPLEMENTATION(_VEC_TYPE(double,2), _VEC_TYPE(long,2))  \
IMPLEMENTATION(_VEC_TYPE(double,3), _VEC_TYPE(long,3))  \
IMPLEMENTATION(_VEC_TYPE(double,4), _VEC_TYPE(long,4))  \
IMPLEMENTATION(_VEC_TYPE(double,8), _VEC_TYPE(long,8))  \
IMPLEMENTATION(_VEC_TYPE(double,16), _VEC_TYPE(long,16)) \

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION
#undef SCALAR_IMPLEMENTATION

#endif // _ISEQUAL_H_
