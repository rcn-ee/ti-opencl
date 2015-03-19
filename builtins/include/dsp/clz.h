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
#ifndef _CLZ_H_
#define _CLZ_H_

#include <clc.h>

/*-----------------------------------------------------------------------------
* The C66 intrisic prototype
*----------------------------------------------------------------------------*/
unsigned _lmbd(unsigned, unsigned);

#define EXPAND_SIZES(type) \
_CLC_OVERLOAD _CLC_INLINE type clz(type v) { return _lmbd(1, v); } \
_CLC_OVERLOAD _CLC_INLINE type##2 clz(type##2 v) \
{ return (type##2)(_lmbd(1, v.s0), _lmbd(1, v.s1)); } \
_CLC_OVERLOAD _CLC_DECL type##3 clz(type##3 v); \
_CLC_OVERLOAD _CLC_DECL type##4 clz(type##4 v); \
_CLC_OVERLOAD _CLC_DECL type##8 clz(type##8 v); \
_CLC_OVERLOAD _CLC_DECL type##16 clz(type##16 v); \

_EXPAND_INTEGER_TYPES()

#undef EXPAND_SIZES

#endif // _CLZ_H_
