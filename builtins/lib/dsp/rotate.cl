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
unsigned _rotl(unsigned, unsigned);

#define EXPAND_SIZES(type) \
_CLC_OVERLOAD _CLC_DEF type##3 rotate(type##3 v, type##3 i) \
{ return (type##3)(_rotl(v.s0,i.s0), _rotl(v.s1,i.s1), _rotl(v.s2,i.s2)); } \
_CLC_OVERLOAD _CLC_DEF type##4 rotate(type##4 v, type##4 i) \
{ return (type##4)(_rotl(v.s0,i.s0), _rotl(v.s1,i.s1), _rotl(v.s2,i.s2), _rotl(v.s3,i.s3)); } \
_CLC_OVERLOAD _CLC_DEF type##8 rotate(type##8 v, type##8 i) \
{ return (type##8)(_rotl(v.s0,i.s0), _rotl(v.s1,i.s1), _rotl(v.s2,i.s2), _rotl(v.s3,i.s3), \
                   _rotl(v.s4,i.s4), _rotl(v.s5,i.s5), _rotl(v.s6,i.s6), _rotl(v.s7,i.s7)); } \
_CLC_OVERLOAD _CLC_DEF type##16 rotate(type##16 v, type##16 i) \
{ return (type##16)(_rotl(v.s0,i.s0), _rotl(v.s1,i.s1), _rotl(v.s2,i.s2), _rotl(v.s3,i.s3), \
                    _rotl(v.s4,i.s4), _rotl(v.s5,i.s5), _rotl(v.s6,i.s6), _rotl(v.s7,i.s7), \
                    _rotl(v.s8,i.s8), _rotl(v.s9,i.s9), _rotl(v.sa,i.sa), _rotl(v.sb,i.sb), \
                    _rotl(v.sc,i.sc), _rotl(v.sd,i.sd), _rotl(v.se,i.se), _rotl(v.sf,i.sf)); } \

_EXPAND_INTEGER_TYPES()
