/******************************************************************************
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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

/* hypotf.c
*  Generic implementation of hypotf(float x, float y) = sqrt(x^2 + y^2)
*    without undue overflow and underflow
*  The implementation performs the computation in double precision, then
*    converts the result back to single precision.
*/

#include "math_private.h"
#include "c66_helper.h"

extern double sqrt(double);

float hypotf(float x, float y)
{
  /* if x or y is +/- Inf, return +Inf */
  /* if x or y is +/- Nan, return +Nan */
  int32_t ha, hb;

  GET_FLOAT_WORD(ha,x);
  ha &= 0x7fffffff;
  GET_FLOAT_WORD(hb,y);
  hb &= 0x7fffffff;
  if (ha == 0x7F800000 || hb == 0x7F800000)  return INFF;
  if (ha >  0x7F800000 || hb >  0x7F800000)  return NANF;

  double t1 = (double) x * x + (double) y * y;
  double t2 = sqrt(t1);
  return (float)t2;
}

