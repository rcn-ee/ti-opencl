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
#include <clc.h>

/*-----------------------------------------------------------------------------
* The C66 intrisic prototype
*----------------------------------------------------------------------------*/
unsigned _lmbd(unsigned, unsigned);

#define EXPAND_SIZES(type) \
_CLC_OVERLOAD _CLC_DEF type##3 clz(type##3 v) \
{ return (type##3)(_lmbd(1, v.s0), _lmbd(1, v.s1), _lmbd(1, v.s2)); } \
_CLC_OVERLOAD _CLC_DEF type##4 clz(type##4 v) \
{ return (type##4)(_lmbd(1, v.s0), _lmbd(1, v.s1), _lmbd(1, v.s2), _lmbd(1, v.s3)); } \
_CLC_OVERLOAD _CLC_DEF type##8 clz(type##8 v) \
{ return (type##8)(_lmbd(1, v.s0), _lmbd(1, v.s1), _lmbd(1, v.s2), _lmbd(1, v.s3), \
                   _lmbd(1, v.s4), _lmbd(1, v.s5), _lmbd(1, v.s6), _lmbd(1, v.s7)); } \
_CLC_OVERLOAD _CLC_DEF type##16 clz(type##16 v) \
{ return (type##16)(_lmbd(1, v.s0), _lmbd(1, v.s1), _lmbd(1, v.s2), _lmbd(1, v.s3), \
                    _lmbd(1, v.s4), _lmbd(1, v.s5), _lmbd(1, v.s6), _lmbd(1, v.s7), \
                    _lmbd(1, v.s8), _lmbd(1, v.s9), _lmbd(1, v.sa), _lmbd(1, v.sb), \
                    _lmbd(1, v.sc), _lmbd(1, v.sd), _lmbd(1, v.se), _lmbd(1, v.sf)); } \

_EXPAND_INTEGER_TYPES()
