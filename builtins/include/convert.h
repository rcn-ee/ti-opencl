/******************************************************************************
 * Copyright (c) 2011-2013, Peter Collingbourne <peter@pcc.me.uk>
 * Copyright (c) 2013 Victor Oliveira <victormatheus@gmail.com>
 * Copyright (c) 2013 Jesse Towner <jessetowner@lavabit.com>
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
#define _CLC_CONVERT_DECL(FROM_TYPE, TO_TYPE, SUFFIX) \
  _CLC_OVERLOAD _CLC_DECL TO_TYPE convert_##TO_TYPE##SUFFIX(FROM_TYPE x);

#define _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, TO_TYPE, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE, TO_TYPE, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##2, TO_TYPE##2, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##3, TO_TYPE##3, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##4, TO_TYPE##4, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##8, TO_TYPE##8, SUFFIX) \
  _CLC_CONVERT_DECL(FROM_TYPE##16, TO_TYPE##16, SUFFIX)

#define _CLC_VECTOR_CONVERT_FROM1(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, char, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, uchar, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, int, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, uint, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, short, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, ushort, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, long, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, ulong, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, float, SUFFIX)

#ifdef cl_khr_fp64
#define _CLC_VECTOR_CONVERT_FROM(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM1(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_DECL(FROM_TYPE, double, SUFFIX)
#else
#define _CLC_VECTOR_CONVERT_FROM(FROM_TYPE, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM1(FROM_TYPE, SUFFIX)
#endif

#define _CLC_VECTOR_CONVERT_TO1(SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(char, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(uchar, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(int, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(uint, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(short, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(ushort, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(long, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(ulong, SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(float, SUFFIX)

#ifdef cl_khr_fp64
#define _CLC_VECTOR_CONVERT_TO(SUFFIX) \
  _CLC_VECTOR_CONVERT_TO1(SUFFIX) \
  _CLC_VECTOR_CONVERT_FROM(double, SUFFIX)
#else
#define _CLC_VECTOR_CONVERT_TO(SUFFIX) \
  _CLC_VECTOR_CONVERT_TO1(SUFFIX)
#endif

#define _CLC_VECTOR_CONVERT_TO_SUFFIX(ROUND) \
  _CLC_VECTOR_CONVERT_TO(_sat##ROUND) \
  _CLC_VECTOR_CONVERT_TO(ROUND)

_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtn)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rte)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtz)
_CLC_VECTOR_CONVERT_TO_SUFFIX(_rtp)
_CLC_VECTOR_CONVERT_TO_SUFFIX()
