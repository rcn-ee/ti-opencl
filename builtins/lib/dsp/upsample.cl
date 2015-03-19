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
#include "dsp.h"

/*-----------------------------------------------------------------------------
* Expand vector type implementations
*----------------------------------------------------------------------------*/
#define TEMPLATE(xtype,ytype,restype) \
_CLC_OVERLOAD _CLC_DEF restype upsample(xtype x, ytype y) \
{ return (restype)(upsample(x.lo,y.lo), upsample(x.hi,y.hi)); } 

#define TEMPLATE3(xtype,ytype,restype) \
_CLC_OVERLOAD _CLC_DEF restype upsample(xtype x, ytype y) \
{ return (restype)(upsample(x.s0,y.s0), upsample(x.s1,y.s1), upsample(x.s2,y.s2)); } 

#define EXPAND_SIZES(xtype, ytype, restype)\
    TEMPLATE(_VEC_TYPE(xtype,2), _VEC_TYPE(ytype,2), _VEC_TYPE(restype,2))\
    TEMPLATE3(_VEC_TYPE(xtype,3), _VEC_TYPE(ytype,3), _VEC_TYPE(restype,3))\
    TEMPLATE(_VEC_TYPE(xtype,4), _VEC_TYPE(ytype,4), _VEC_TYPE(restype,4))\
    TEMPLATE(_VEC_TYPE(xtype,8), _VEC_TYPE(ytype,8), _VEC_TYPE(restype,8))\
    TEMPLATE(_VEC_TYPE(xtype,16), _VEC_TYPE(ytype,16), _VEC_TYPE(restype,16))\

#define _EXPAND_UPSAMPLE_TYPES()   \
    EXPAND_SIZES(char, uchar, short)   \
    EXPAND_SIZES(uchar, uchar, ushort) \
    EXPAND_SIZES(short, ushort, int)   \
    EXPAND_SIZES(ushort, ushort, uint) \
    EXPAND_SIZES(int, uint, long)      \
    EXPAND_SIZES(uint, uint, ulong)    \

_EXPAND_UPSAMPLE_TYPES()
