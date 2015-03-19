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

DLLEXPORT
float sqrtf(float a)
{
    const float  Half  = 0.5f;
    const float  OneP5 = 1.5f;
    float  x, y;

    if (_ftoi(a) == 0x7F800000) return a; 
    if (a < 0)                  return _itof(0xFFC00000);

    x = _rsqrsp(a);

    x = x * (OneP5 - (a * x * x * Half));
    x = x * (OneP5 - (a * x * x * Half));
    y = a * x;

    if (a <= 0.0f)   y = 0.0f;
    if (a > FLT_MAX) y = FLT_MAX;

    return (y);
}

DLLEXPORT
double sqrt(double a)
{
    const double  Half  = 0.5;
    const double  OneP5 = 1.5;
    double  x, y;

    if (ISPOSINFD(a)) return a; 
    if (a < 0)        return NEGNAND;

    x = _rsqrdp(a);

    x = x * (OneP5 - (a * x * x * Half));
    x = x * (OneP5 - (a * x * x * Half));
    x = x * (OneP5 - (a * x * x * Half));
    y = a * x;

    if (a <= 0.0)    return 0.0;
    if (a > DBL_MAX) return DBL_MAX;

    return y;
}

DLLEXPORT
float native_sqrtf(float a)
{
    const float  Half  = 0.5f;
    const float  OneP5 = 1.5f;
    float  x, y;

    x = _rsqrsp(a);
    x = x * (OneP5 - (a * x * x * Half));
    x = x * (OneP5 - (a * x * x * Half));
    y = a * x;

    if (a <= 0.0f)   y = 0.0f;
    if (a > FLT_MAX) y = FLT_MAX;

    return (y);
}

DLLEXPORT
double native_sqrt(double a)
{
    const double  Half  = 0.5f;
    const double  OneP5 = 1.5f;
    double  x, y;

    x = _rsqrdp(a);
    x = x * (OneP5 - (a * x * x * Half));
    x = x * (OneP5 - (a * x * x * Half));
    x = x * (OneP5 - (a * x * x * Half));
    y = a * x;

    if (a <= 0.0)    return 0.0;
    if (a > FLT_MAX) return FLT_MAX;

    return y;
}

