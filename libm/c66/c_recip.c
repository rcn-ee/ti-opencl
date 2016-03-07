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

float  __fmpy_by_0x1p25(float);
double __dmpy_by_0x1p54(double);

DLLEXPORT
float native_recipf(float arg)
{
    float X0 = _rcpsp(arg);
    float X1 = X0 *  (2.0f - arg * X0);
               X1 *= (2.0f - arg * X1);  // sufficient for 2.9 ULP
               X1 *= (2.0f - arg * X1);  // needed to get below 2.5 ULP for OpenCL

    return X1;
}

DLLEXPORT
double native_recip(double arg)
{
    double X0 = _rcpdp(arg);
    double X1 = X0 *  (2.0 - arg * X0);
                X1 *= (2.0 - arg * X1);
                X1 *= (2.0 - arg * X1);
                X1 *= (2.0 - arg * X1);

    return X1;
}

DLLEXPORT
float reciprocalf(float arg)
{
    if (ISNANF(arg))     return arg;
    if (ISANYZEROF(arg)) return copysign(INFF, arg);
    if (ISINFF(arg))     return 0.0;

    int underflow = (FABSF(arg) > 0x1p125);
    int subnormal = ISDENORMF(arg);

    if (subnormal) arg = __fmpy_by_0x1p25(arg);
    if (underflow) arg *= 0x1p-2;

                   arg = native_recipf(arg);

    if (subnormal) arg *= 0x1p25;
    if (underflow) arg *= 0x1p-2;

    return arg;
}

DLLEXPORT
double reciprocald(double arg)
{
    if (ISNAND(arg))     return arg;
    if (ISANYZEROD(arg)) return copysign(INFD, arg);
    if (ISINFD(arg))     return 0.0;

    int underflow = (fabs(arg) > 0x1p1020);
    int subnormal = ISDENORMD(arg);

    if (subnormal) arg = __dmpy_by_0x1p54(arg);
    if (underflow) arg *= 0x1p-4;

                   arg = native_recip(arg);

    if (subnormal) arg *= 0x1p54;
    if (underflow) arg *= 0x1p-4;

    return arg;
}

