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
#include "clc.h"

#define DECLARATION(type, itype, utype) \
_CLC_OVERLOAD _CLC_DEF type select(type a, type b, itype c) { return c ? b : a; }\
_CLC_OVERLOAD _CLC_DEF type select(type a, type b, utype c) { return c ? b : a; }

#define SELECT_EXPAND_SIZES(type,itype,utype) \
    DECLARATION(_VEC_TYPE(type,2), _VEC_TYPE(itype,2), _VEC_TYPE(utype,2))  \
    DECLARATION(_VEC_TYPE(type,3), _VEC_TYPE(itype,3), _VEC_TYPE(utype,3))  \
    DECLARATION(_VEC_TYPE(type,4), _VEC_TYPE(itype,4), _VEC_TYPE(utype,4))  \
    DECLARATION(_VEC_TYPE(type,8), _VEC_TYPE(itype,8), _VEC_TYPE(utype,8))  \
    DECLARATION(_VEC_TYPE(type,16), _VEC_TYPE(itype,16), _VEC_TYPE(utype,16))  \

#define SELECT_EXPAND_TYPES   \
    SELECT_EXPAND_SIZES(char, char, uchar)   \
    SELECT_EXPAND_SIZES(uchar, char, uchar)  \
    SELECT_EXPAND_SIZES(short, short, ushort)  \
    SELECT_EXPAND_SIZES(ushort, short, ushort) \
    SELECT_EXPAND_SIZES(int, int, uint)    \
    SELECT_EXPAND_SIZES(uint, int, uint)   \
    SELECT_EXPAND_SIZES(long, long, ulong)   \
    SELECT_EXPAND_SIZES(ulong, long, ulong)  \
    SELECT_EXPAND_SIZES(float, int, uint)  \
    SELECT_EXPAND_SIZES(double, long, ulong)

SELECT_EXPAND_TYPES
