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

DLLEXPORT float powrf(float x, float y)
{
    if (ISNANF(y))                              return NANF;
    if (ISNANF(x))                              return NANF;
    if (ISANYZEROF(x) && ISANYZEROF(y))         return NANF;
    if (ISPOSINFF(x)  && ISANYZEROF(y))         return NANF;
    if (x == 1.0f && ISINFF(y))                 return NANF;

    if (((int)_ftoi(x)) > 0 && ISANYZEROF(y))   return 1.0f;
    if (ISANYZEROF(x) && ((int)_ftoi(y)) < 0)   return INFF;
    if (ISANYZEROF(x) && ISNEGINFF(y))          return INFF;
    if (ISANYZEROF(x) && ((int)_ftoi(y)) > 0)   return 0.0f;
    if (x == 1.0f && !ISINFF(y))                return 1.0f;
    if (((int)_ftoi(x)) < 0)                    return NANF;

    return powf(x,y);
}

DLLEXPORT double powr(double x, double y)
{

    if (ISNAND(y))                              return NAND;
    if (ISNAND(x))                              return NAND;
    if (ISANYZEROD(x) && ISANYZEROD(y))         return NAND;
    if (ISPOSINFD(x)  && ISANYZEROD(y))         return NAND;
    if (x == 1.0 && ISINFD(y))                  return NAND;

    if (((long long) _dtoll(x)) > 0 && ISANYZEROD(y))   return 1.0;
    if (ISANYZEROD(x) && ((long long) _dtoll(y < 0)))   return INFD;
    if (ISANYZEROD(x) && ISNEGINFD(y))          return INFD;
    if (ISANYZEROD(x) && ((long long) _dtoll(y > 0)))   return 0.0;
    if (x == 1.0 && !ISINFD(y))                return 1.0;
    if (((long long) _dtoll(x)) < 0)                    return NAND;

    return pow(x,y);
}
