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

_UNARY_VEC_DEF(float,  int,  isnan, -isnan)
_UNARY_VEC_DEF(double, long, isnan, -isnan)

_UNARY_VEC_DEF(float,  int,  isfinite, -isfinite)
_UNARY_VEC_DEF(double, long, isfinite, -isfinite)

_UNARY_VEC_DEF(float,  int,  isinf, -isinf)
_UNARY_VEC_DEF(double, long, isinf, -isinf)

_UNARY_VEC_DEF(float,  int,  isnormal, -isnormal)
_UNARY_VEC_DEF(double, long, isnormal, -isnormal)

_UNARY_VEC_DEF(float,  int,  signbit, -signbit)
_UNARY_VEC_DEF(double, long, signbit, -signbit)

_BINARY_VEC_DEF(float,  int,  isequal, -isequal)
_BINARY_VEC_DEF(double, long, isequal, -isequal)

_BINARY_VEC_DEF(float,  int,  isnotequal, -isnotequal)
_BINARY_VEC_DEF(double, long, isnotequal, -isnotequal)

_BINARY_VEC_DEF(float,  int,  isless, -isless)
_BINARY_VEC_DEF(double, long, isless, -isless)

_BINARY_VEC_DEF(float,  int,  islessequal, -islessequal)
_BINARY_VEC_DEF(double, long, islessequal, -islessequal)

_BINARY_VEC_DEF(float,  int,  isgreater, -isgreater)
_BINARY_VEC_DEF(double, long, isgreater, -isgreater)

_BINARY_VEC_DEF(float,  int,  isgreaterequal, -isgreaterequal)
_BINARY_VEC_DEF(double, long, isgreaterequal, -isgreaterequal)

_BINARY_VEC_DEF(float,  int,  islessgreater, -islessgreater)
_BINARY_VEC_DEF(double, long, islessgreater, -islessgreater)
