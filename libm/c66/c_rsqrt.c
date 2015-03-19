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
float rsqrtf(float a)
{
    float Half  =  0.5;
    float OneP5 =  1.5;
    float Small =  0x1.0p-126f;
    float X0, X1, X2;

    if (_ftoi(a) == 0x80000000) return _itof(0xFF800000);

    X0 = _rsqrsp(a);

    if (_ftoi(X0) == 0 || _ftoi(X0) > 0x7F800000) return X0;

    X1 = X0 * (OneP5 - (a * X0 * X0 * Half));
    X2 = X1 * (OneP5 - (a * X1 * X1 * Half));
    X2 = X2 * (OneP5 - (a * X2 * X2 * Half));

    if (a < Small) return INFF;
    return X2;
}

DLLEXPORT
double rsqrt(double a)
{
    double Half  =  0.5;
    double OneP5 =  1.5;
    double Small =  0x1.0p-1022;
    double X0, X1, X2, X3;

    /*-------------------------------------------------------------------------
    * Does not handle denorms per OpenCL spec
    *------------------------------------------------------------------------*/
    if (a == 0.0)      return copysign(INFD,a);

    X0 = _rsqrdp(a);

    if (X0 == 0 || ISNAND(X0)) return X0;

    X1 = X0 * (OneP5 - (a * X0 * X0 * Half));
    X2 = X1 * (OneP5 - (a * X1 * X1 * Half));
    X3 = X2 * (OneP5 - (a * X2 * X2 * Half));

    if (a < Small) return INFD;
    return X3;
}

DLLEXPORT
float native_rsqrtf(float a)
{
    float Half  =  0.5;
    float OneP5 =  1.5;
    float Small =  1.17549435e-38;
    float X0, X1, X2;

    X0 = _rsqrsp(a);
    X1 = X0 * (OneP5 - (a * X0 * X0 * Half));
    X2 = X1 * (OneP5 - (a * X1 * X1 * Half));

    if (a < Small) return INFF;
    return X2;
}

DLLEXPORT
double native_rsqrt(double a)
{
    double Half  =  0.5;
    double OneP5 =  1.5;
    double Small =  0x1.0p-1022;
    double X0, X1, X2, X3;

    X0 = _rsqrdp(a);
    X1 = X0 * (OneP5 - (a * X0 * X0 * Half));
    X2 = X1 * (OneP5 - (a * X1 * X1 * Half));
    X3 = X2 * (OneP5 - (a * X2 * X2 * Half));

    if (a < Small) return INFD;
    return X3;
}
