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
float maxmagf(float a, float b)
{
    /*-------------------------------------------------------------------------
    * If two subnormals, choose the greater mantissa.  Needed because c66 
    * float compares always returns 0 when both imputs are denorm. This will
    * also catch zero inputs.
    *------------------------------------------------------------------------*/
    if (EXPF(a) == 0 && EXPF(b) == 0)
    {
        if (MANTF(a) > MANTF(b)) return  a;
        if (MANTF(b) > MANTF(a)) return  b;
        return SIGNF(a) ? b : a;
    }

    if (_fabsf(a) > _fabsf(b)) return a;
    if (_fabsf(b) > _fabsf(a)) return b;
    return fmaxf(a,b);
}

DLLEXPORT
double maxmag(double a, double b)
{
    /*-------------------------------------------------------------------------
    * If two subnormals, choose the greater mantissa.  Needed because c66 
    * float compares always returns 0 when both imputs are denorm. This will
    * also catch zero inputs.
    *------------------------------------------------------------------------*/
    if (EXPD(a) == 0 && EXPD(b) == 0)
    {
        if (MANTD(a) > MANTD(b)) return  a;
        if (MANTD(b) > MANTD(a)) return  b;
        return SIGND(a) ? b : a;
    }

    if (_fabs(a) > _fabs(b)) return a;
    if (_fabs(b) > _fabs(a)) return b;
    return fmax(a,b);
}

DLLEXPORT
float minmagf(float a, float b)
{
    /*-------------------------------------------------------------------------
    * If two subnormals, choose the greater mantissa.  Needed because c66 
    * float compares always returns 0 when both imputs are denorm. This will
    * also catch zero inputs.
    *------------------------------------------------------------------------*/
    if (EXPF(a) == 0 && EXPF(b) == 0)
    {
        if (MANTF(a) < MANTF(b)) return  a;
        if (MANTF(b) < MANTF(a)) return  b;
        return SIGNF(a) ? a : b;
    }

    if (_fabsf(a) < _fabsf(b)) return a;
    if (_fabsf(b) < _fabsf(a)) return b;
    return fminf(a,b);
}

DLLEXPORT
double minmag(double a, double b)
{
    /*-------------------------------------------------------------------------
    * If two subnormals, choose the greater mantissa.  Needed because c66 
    * float compares always returns 0 when both imputs are denorm. This will
    * also catch zero inputs.
    *------------------------------------------------------------------------*/
    if (EXPD(a) == 0 && EXPD(b) == 0)
    {
        if (MANTD(a) < MANTD(b)) return  a;
        if (MANTD(b) < MANTD(a)) return  b;
        return SIGND(a) ? a : b;
    }

    if (_fabs(a) < _fabs(b)) return a;
    if (_fabs(b) < _fabs(a)) return b;
    return fmin(a,b);
}
