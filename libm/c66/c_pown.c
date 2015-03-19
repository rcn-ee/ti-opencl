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
#include "cdefs-compat.h"
#include <float.h>
#include "fpmath.h"
#include "openlibm.h"
#include "math_private.h"
#include <stdio.h>
#include "c66_helper.h"

DLLEXPORT float pownf(float x, int y)
{
    int y_odd = (y&1);
    int y_neg = (y<0);
    int y_even = !y_odd;

    if (y == 0) return 1.0f;
    if (y == 1) return x;
    if (ISNANF(x)) return x; 

    if (y_even) x = FABSF(x);

    if (ISINFF(x)) 
    {
        if (y_neg && y_even) return 0.0f;
        if (y_neg && y_odd)  return copysign(0.0f, x);
        return x;
    }

    float result;
    /*-------------------------------------------------------------------------
    * dont overflow the mantissa of a float with an int that is too large
    *------------------------------------------------------------------------*/
    if (y>0xffffff || y < -0xffffff)
         result = pow((double)x, (double)y);
    else result = powf(x, (float)y);

    if (ISINFF(result)) result = copysign(result, x);

    return result;
}

DLLEXPORT double pown(double x, int y)
{
    int y_odd = (y&1);
    int y_neg = (y<0);
    int y_even = !y_odd;

    if (y == 0) return 1.0;
    if (y == 1) return x;
    if (ISNAND(x)) return x; 

    if (y_even) x = fabs(x);

    if (ISINFD(x)) 
    {
        if (y_neg && y_even) return 0.0;
        if (y_neg && y_odd)  return copysign(0.0, x);
        return x;
    }

    double result = pow(x, (double)y);

    if (ISINFD(result)) result = copysign(result, x);

    return result;
}
