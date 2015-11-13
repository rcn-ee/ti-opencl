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
* The template for non rotl applicable scalar types
*----------------------------------------------------------------------------*/
#define SCALAR(type, utype) \
_CLC_OVERLOAD _CLC_DEF type rotate(type v, type i) \
{\
    uint bits = sizeof(v) << 3;\
    uint mask = bits - 1; \
    i &= mask; \
    if (i == 0) return v; \
    return (v << i) | ((utype)v >> (bits-i)); \
}\

SCALAR(uchar, uchar)
SCALAR(char, uchar)
SCALAR(ushort, ushort)
SCALAR(short, ushort)
SCALAR(ulong, ulong)
SCALAR(long, ulong)

_BINARY_VEC_DEF(char, char,  rotate, rotate)
_BINARY_VEC_DEF(uchar, uchar, rotate, rotate)
_BINARY_VEC_DEF(short, short, rotate, rotate)
_BINARY_VEC_DEF(ushort, ushort,rotate, rotate)
_BINARY_VEC_DEF(int, int,   rotate, _rotl)
_BINARY_VEC_DEF(uint, uint,  rotate, _rotl)
_BINARY_VEC_DEF(long, long,  rotate, rotate)
_BINARY_VEC_DEF(ulong, ulong, rotate, rotate)
