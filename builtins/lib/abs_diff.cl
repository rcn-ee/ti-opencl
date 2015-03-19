/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "dsp.h"

#define EXPAND_SIZES(type, utype) \
    TEMPLATE(_VEC_TYPE(type,2), _VEC_TYPE(utype,2))  \
    TEMPLATE(_VEC_TYPE(type,3), _VEC_TYPE(utype,3))  \
    TEMPLATE(_VEC_TYPE(type,4), _VEC_TYPE(utype,4))  \
    TEMPLATE(_VEC_TYPE(type,8), _VEC_TYPE(utype,8))  \
    TEMPLATE(_VEC_TYPE(type,16), _VEC_TYPE(utype,16)) \

#define TEMPLATE(gentype, ugentype) \
    _CLC_OVERLOAD _CLC_DEF ugentype abs_diff(gentype x, gentype y) \
    { return __builtin_astype(x > y ? x-y : y-x, ugentype); }

EXPAND_SIZES(uchar, uchar)
EXPAND_SIZES(char,  uchar)
EXPAND_SIZES(ushort, ushort)
EXPAND_SIZES(short,  ushort)
EXPAND_SIZES(uint,   uint)
EXPAND_SIZES(ulong,  ulong)

#undef TEMPLATE

#define TEMPLATE(gentype, ugentype, shiftval) \
_CLC_OVERLOAD _CLC_DEF ugentype abs_diff(gentype x,  gentype y) \
{ \
     gentype signs_differ = (x^y) >> (gentype)shiftval; \
     return (signs_differ) ? abs(x) + abs(y) : \
           __builtin_astype(x > y ? x-y : y-x, ugentype); \
} 

TEMPLATE(int,  uint, 31)
TEMPLATE(_VEC_TYPE(int,2), _VEC_TYPE(uint,2), 31)
TEMPLATE(_VEC_TYPE(int,3), _VEC_TYPE(uint,3), 31)
TEMPLATE(_VEC_TYPE(int,4), _VEC_TYPE(uint,4), 31)
TEMPLATE(_VEC_TYPE(int,8), _VEC_TYPE(uint,8), 31)
TEMPLATE(_VEC_TYPE(int,16), _VEC_TYPE(uint,16), 31)

TEMPLATE(long, ulong, 63)
TEMPLATE(_VEC_TYPE(long,2), _VEC_TYPE(ulong,2), 63)
TEMPLATE(_VEC_TYPE(long,3), _VEC_TYPE(ulong,3), 63)
TEMPLATE(_VEC_TYPE(long,4), _VEC_TYPE(ulong,4), 63)
TEMPLATE(_VEC_TYPE(long,8), _VEC_TYPE(ulong,8), 63)
TEMPLATE(_VEC_TYPE(long,16), _VEC_TYPE(ulong,16), 63)

#undef TEMPLATE
