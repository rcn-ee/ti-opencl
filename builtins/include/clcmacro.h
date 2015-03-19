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
#ifndef _CLC_MACRO_H_
#define _CLC_MACRO_H_

#define _CLC_UNARY_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE##2 x) { \
    return (RET_TYPE##2)(FUNCTION(x.x), FUNCTION(x.y)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE##3 x) { \
    return (RET_TYPE##3)(FUNCTION(x.x), FUNCTION(x.y), FUNCTION(x.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE##4 x) { \
    return (RET_TYPE##4)(FUNCTION(x.lo), FUNCTION(x.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE##8 x) { \
    return (RET_TYPE##8)(FUNCTION(x.lo), FUNCTION(x.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE##16 x) { \
    return (RET_TYPE##16)(FUNCTION(x.lo), FUNCTION(x.hi)); \
  }

#define _CLC_BINARY_VECTORIZE(DECLSPEC, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE) \
  DECLSPEC RET_TYPE##2 FUNCTION(ARG1_TYPE##2 x, ARG2_TYPE##2 y) { \
    return (RET_TYPE##2)(FUNCTION(x.x, y.x), FUNCTION(x.y, y.y)); \
  } \
\
  DECLSPEC RET_TYPE##3 FUNCTION(ARG1_TYPE##3 x, ARG2_TYPE##3 y) { \
    return (RET_TYPE##3)(FUNCTION(x.x, y.x), FUNCTION(x.y, y.y), \
                         FUNCTION(x.z, y.z)); \
  } \
\
  DECLSPEC RET_TYPE##4 FUNCTION(ARG1_TYPE##4 x, ARG2_TYPE##4 y) { \
    return (RET_TYPE##4)(FUNCTION(x.lo, y.lo), FUNCTION(x.hi, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##8 FUNCTION(ARG1_TYPE##8 x, ARG2_TYPE##8 y) { \
    return (RET_TYPE##8)(FUNCTION(x.lo, y.lo), FUNCTION(x.hi, y.hi)); \
  } \
\
  DECLSPEC RET_TYPE##16 FUNCTION(ARG1_TYPE##16 x, ARG2_TYPE##16 y) { \
    return (RET_TYPE##16)(FUNCTION(x.lo, y.lo), FUNCTION(x.hi, y.hi)); \
  }

#define _CLC_DEFINE_BINARY_BUILTIN(RET_TYPE, FUNCTION, BUILTIN, ARG1_TYPE, ARG2_TYPE) \
_CLC_DEF _CLC_OVERLOAD RET_TYPE FUNCTION(ARG1_TYPE x, ARG2_TYPE y) { \
  return BUILTIN(x, y); \
} \
_CLC_BINARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, RET_TYPE, FUNCTION, ARG1_TYPE, ARG2_TYPE)

#define _CLC_DEFINE_UNARY_BUILTIN(RET_TYPE, FUNCTION, BUILTIN, ARG1_TYPE) \
_CLC_DEF _CLC_OVERLOAD RET_TYPE FUNCTION(ARG1_TYPE x) { \
  return BUILTIN(x); \
} \
_CLC_UNARY_VECTORIZE(_CLC_OVERLOAD _CLC_DEF, RET_TYPE, FUNCTION, ARG1_TYPE)

#define _VEC_TYPE(type,sz) type##sz

#define _EXPAND_TYPES()   \
    EXPAND_SIZES(char)   \
    EXPAND_SIZES(uchar)  \
    EXPAND_SIZES(short)  \
    EXPAND_SIZES(ushort) \
    EXPAND_SIZES(int)    \
    EXPAND_SIZES(uint)   \
    EXPAND_SIZES(long)   \
    EXPAND_SIZES(ulong)  \
    EXPAND_SIZES(float)  \
    EXPAND_SIZES(double) 

#define _EXPAND_INTEGER_TYPES()   \
    EXPAND_SIZES(char)   \
    EXPAND_SIZES(uchar)  \
    EXPAND_SIZES(short)  \
    EXPAND_SIZES(ushort) \
    EXPAND_SIZES(int)    \
    EXPAND_SIZES(uint)   \
    EXPAND_SIZES(long)   \
    EXPAND_SIZES(ulong)  

#define _EXPAND_FLOAT_TYPES()   \
    EXPAND_SIZES(float)  \
    EXPAND_SIZES(double) 

#endif // _CLC_MACRO_H_
