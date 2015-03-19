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

#define _sptruncf(x)  (EXPF(x) < 150 ? (float)(int) x : x)
#define _sptruncd(x)  (EXPD(x) < 1075 ? (double)(long long) x : x)

#define GE2f(x) _extu(_ftoi(x),1,31)
#define GE2d(x) _extu(_hi(x),1,31)

static inline float _fmod2f(float x)
{
    union IEEEf2bits mod; 
    mod.f = x; 
    mod.bits.exp -= 1;  
    mod.f = _sptruncf(mod.f); 
    mod.bits.exp += 1; 
    return x - mod.f;
}

static inline double _fmod2d(double x)
{
    union IEEEd2bits mod; 
    mod.d = x; 
    mod.bits.exp -= 1;  
    mod.d = _sptruncd(mod.d); 
    mod.bits.exp += 1; 
    return x - mod.d;
}


DLLEXPORT
float sinpif(float x) 
{
    if (ISNANF(x)) return x;
    if (ISINFF(x)) return NANF;

    // dont much care about denorms
    if (EXPF(x) == 0) return sinf(x*M_PI);

    float rx;
    if (GE2f(x)) rx = _fmod2f(x);
    else              rx = x;

    float arx = _fabsf(rx);

    if (arx == 0.00f) return x == 0.0f ? x : arx;
    if (arx <  0.25f) return sinf(M_PI*rx);
    if (arx <= 0.75f) return copysign(cosf(M_PI*(0.5f-arx)),rx);
    if (arx <  1.25f) return sinf(M_PI*(copysign(1.0f,rx) - rx));
    if (arx <= 1.75f) return -copysign(cosf(M_PI*(1.5f-arx)),rx);
                      return sinf(M_PI*(rx-copysign(2.0f,rx)));
}

DLLEXPORT
float cospif(float x) 
{
    if (ISNANF(x)) return x;
    if (ISINFF(x)) return NANF;

    x = _fabsf(x);

    if (GE2f(x)) x = _fmod2f(x);

    if (x <= 0.25f) return  cosf(M_PI * x);
    if (x <  0.75f) return  sinf(M_PI * (0.5f-x));
    if (x <= 1.25f) return -cosf(M_PI * (1.0f-x));
    if (x <  1.75f) return  sinf(M_PI * (x-1.5f)); 
                    return  cosf(M_PI * (2.0f-x)); 
}

DLLEXPORT
double cospi(double x) 
{
    if (ISNAND(x)) return x;
    if (ISINFD(x)) return NAND;

    x = _fabs(x);

    if (GE2d(x)) x = _fmod2d(x);

    if (x <= 0.25) return  cos(M_PI * x);
    if (x <  0.75) return  sin(M_PI * (0.5-x));
    if (x <= 1.25) return -cos(M_PI * (1.0-x));
    if (x <  1.75) return  sin(M_PI * (x-1.5)); 
                   return  cos(M_PI * (2.0-x)); 
}

DLLEXPORT
double sinpi(double x) 
{
    if (ISNAND(x)) return x;
    if (ISINFD(x)) return NAND;

    double rx;
    if (GE2d(x)) rx = _fmod2d(x);
    else         rx = x;

    double arx = _fabs(rx);

    if (arx == 0.00) return x == 0.0 ? x : arx;
    if (arx <  0.25) return sin(M_PI*rx);
    if (arx <= 0.75) return copysign(cos(M_PI*(0.5-arx)),rx);
    if (arx <  1.25) return sin(M_PI*(copysign(1.0,rx) - rx));
    if (arx <= 1.75) return -copysign(cos(M_PI*(1.5-arx)),rx);
                     return sin(M_PI*(rx-copysign(2.0,rx)));
}

DLLEXPORT float  tanpif(float x) { return sinpif(x) / cospif(x); }
DLLEXPORT double tanpi(double x) { return sinpi(x) / cospi(x); } 

