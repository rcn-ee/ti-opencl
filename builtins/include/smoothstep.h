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
#ifndef _SMOOTHSTEP_H_
#define _SMOOTHSTEP_H_

#include <clc.h>

#define EXPAND_SIZES(type)                  \
    SCALAR_IMPLEMENTATION(type)             \
    DECLARATION(_VEC_TYPE(type,2), type)  \
    DECLARATION(_VEC_TYPE(type,3), type)  \
    DECLARATION(_VEC_TYPE(type,4), type)  \
    DECLARATION(_VEC_TYPE(type,8), type)  \
    DECLARATION(_VEC_TYPE(type,16), type) \

#define DECLARATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_DECL gentype smoothstep(gentype edge0, gentype edge1, gentype x); \
_CLC_OVERLOAD _CLC_DECL gentype smoothstep(sgentype edge0, sgentype edge1, gentype x); \

#define IMPLEMENTATION(gentype, sgentype) \
_CLC_OVERLOAD _CLC_INLINE gentype smoothstep(gentype edge0, gentype edge1, gentype x)  \
    { gentype t = clamp((x-edge0)/(edge1-edge0), (sgentype)0.0, (sgentype)1.0); \
      return t*t*((gentype)3-(gentype)2*t); } \
_CLC_OVERLOAD _CLC_INLINE gentype smoothstep(sgentype edge0, sgentype edge1, gentype x) \
    { gentype t = clamp((x-(gentype)edge0)/((gentype)edge1-(gentype)edge0), (sgentype)0.0, (sgentype)1.0); \
      return t*t*((gentype)3-(gentype)2*t); } \

#define SCALAR_IMPLEMENTATION(gentype) \
_CLC_OVERLOAD _CLC_INLINE gentype smoothstep(gentype edge0, gentype edge1, gentype x)  \
    { gentype t = clamp((x-edge0)/(edge1-edge0), (gentype)0.0, (gentype)1.0); \
      return t*t*((gentype)3-(gentype)2*t); } \

_EXPAND_FLOAT_TYPES()

#undef EXPAND_SIZES
#undef DECLARATION
#undef IMPLEMENTATION
#undef SCALAR_IMPLEMENTATION

#endif // _SMOOTHSTEP_H_
