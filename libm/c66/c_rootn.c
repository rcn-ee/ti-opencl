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

float  reciprocalf(float);
double reciprocald(double);

DLLEXPORT float rootnf(float x, int y)
{
    int y_odd  = (y&1);
    int y_even = !y_odd;
    int y_neg  = (y<0);
    int x_neg  = (x<0);

    if (y == 0) return NANF;
    if (EXPF(x) == 0)  /* Flush denormalized input to zero */
        return (y > 0) ? 0.0f : (y_even ? INFF : copysign(INFF, x));
    if (y == 1) return x;
    if (ISNEGINFF(x) && y_even) return NANF;
    if (ISINFF(x)) return y_neg ? copysign(0.0f, x) : x;
    if (ISANYZEROF(x) && y_odd && y_neg) return copysign(INFF, x);

    int negresult = 0;
    if (x_neg && y_odd) { negresult = 1; x = FABSF(x); }

    float result =  powf(x, reciprocalf(y));
    if (negresult) result = -result;
    return result;
}

DLLEXPORT double rootn(double x, int y)
{
    int y_odd  = (y&1);
    int y_even = !y_odd;
    int y_neg  = (y<0);
    int x_neg  = (x<0);

    if (y == 0) return NAND;
    if (y == 1) return x;
    if (ISNEGINFD(x) && y_even) return NAND;

    if (ISINFD(x)) return y_neg ? copysign(0.0, x) : x;
    if (ISANYZEROD(x) && y_odd && y_neg) return copysign(INFD, x);

    int negresult = 0;
    if (x_neg && y_odd) { negresult = 1; x = fabs(x); }
    double result =  pow(x, reciprocald(y));
    if (negresult) result = -result;
    return result;
}
