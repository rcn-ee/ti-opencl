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
#include <openlibm.h>
#include "math_private.h"
#include "fpmath.h"
#include "c66_helper.h"

DLLEXPORT float fmaxf(float x, float y)
{
    if (ISNANF(x))           return y;
    if (ISNANF(y))           return x;

    if (EXPF(x) == 0 && EXPF(y) == 0)  
    {
        if (SIGNF(x) ^ SIGNF(y)) return SIGNF(x) ? y : x;
        if (SIGNF(x)) return _ftoi(x) > _ftoi(y) ? y : x;
        else          return _ftoi(x) > _ftoi(y) ? x : y;
    }

    return (x > y ? x : y);
}

DLLEXPORT double fmax(double x, double y)
{
    if (ISNAND(x))           return y;
    if (ISNAND(y))           return x;

    if (EXPD(x) == 0 && EXPD(y) == 0)  
    {
        if (SIGND(x) ^ SIGND(y)) return SIGND(x) ? y : x;
        if (SIGND(x)) return _dtoll(x) > _dtoll(y) ? y : x;
        else          return _dtoll(x) > _dtoll(y) ? x : y;
    }

    return (x > y ? x : y);
}

DLLEXPORT float fminf(float x, float y)
{
    if (ISNANF(x))           return y;
    if (ISNANF(y))           return x;

    if (EXPF(x) == 0 && EXPF(y) == 0)  
    {
        if (SIGNF(x) ^ SIGNF(y)) return SIGNF(x) ? x : y;
        if (SIGNF(x)) return _ftoi(x) > _ftoi(y) ? x : y;
        else          return _ftoi(x) > _ftoi(y) ? y : x;
    }

    return (x < y ? x : y);
}

DLLEXPORT double fmin(double x, double y)
{
    if (ISNAND(x)) return y;
    if (ISNAND(y)) return x;

    if (EXPD(x) == 0 && EXPD(y) == 0)  
    {
        if (SIGND(x) ^ SIGND(y)) return SIGND(x) ? x : y;
        if (SIGND(x)) return _dtoll(x) > _dtoll(y) ? x : y;
        else          return _dtoll(x) > _dtoll(y) ? y : x;
    }

    return (x < y ? x : y);
}
