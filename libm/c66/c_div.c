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
float __c6xabi_divf(float arg1, float arg2)
{
    /*-------------------------------------------------------------------------
    * input NaNs have out NaNs
    *------------------------------------------------------------------------*/
    if (ISNANF(arg1)) return arg1;
    if (ISNANF(arg2)) return arg2;


    /*-------------------------------------------------------------------------
    * inf / inf or 0 / 0 generates new NaN
    *------------------------------------------------------------------------*/
    if ((arg1 == 0.0f && arg2 == 0.0f) || (EXPF(arg1) == 255 && EXPF(arg2) == 255)) 
        return NEGNANF;

    /*-------------------------------------------------------------------------
    * xor of input signs determines output sign
    *------------------------------------------------------------------------*/
    unsigned neg_result= (SIGNF(arg1) ^ SIGNF(arg2)) << 31;
    int      exp_delta = (int)EXPF(arg1) - (int)EXPF(arg2);

    /*-------------------------------------------------------------------------
    * Inf / anything or anything / zero is inf, also inf if / overflows
    *------------------------------------------------------------------------*/
    if (EXPF(arg1) == 255 || arg2 == 0.0f || exp_delta > 128) 
        return _itof(0x7F800000 | neg_result);

    /*-------------------------------------------------------------------------
    * anything / inf is zero, also 0 if / underflows
    *------------------------------------------------------------------------*/
    if (EXPF(arg2) == 255 || exp_delta < -127) return _itof(neg_result);


    /*-------------------------------------------------------------------------
    * If the C6x divisor recip will underflow, we divide dividend and divisor 
    * by 2*4 to prevent the underflow.  Previous early returns have ensured 
    * that this division is ok for the dividend.
    *------------------------------------------------------------------------*/
    if (EXPF(arg2) > 251 )
    {
        union IEEEf2bits u1; u1.f = arg1;
        union IEEEf2bits u2; u2.f = arg2;
        u2.bits.exp -= 4;
        u1.bits.exp -= 4;
        arg2 = u2.f;
        arg1 = u1.f;
    }

    float X0 = _rcpsp(arg2);
    float X1 = X0 *  (2.0f - arg2 * X0);
               X1 *= (2.0f - arg2 * X1);  // sufficient for 2.9 ULP
               X1 *= (2.0f - arg2 * X1);  // needed to get below 2.5 ULP for OpenCL

    return arg1 * X1;
}

DLLEXPORT
double __c6xabi_divd(double arg1, double arg2)
{
    /*-------------------------------------------------------------------------
    * input NaNs have out NaNs
    *------------------------------------------------------------------------*/
    if (ISNAND(arg1)) return arg1;
    if (ISNAND(arg2)) return arg2;

    /*-------------------------------------------------------------------------
    * inf / inf or 0 / 0 generates new NaN
    *------------------------------------------------------------------------*/
    if ((arg1 == 0.0 && arg2 == 0.0) || (EXPD(arg1) == 2047 && EXPD(arg2) == 2047)) 
        return NEGNAND;

    /*-------------------------------------------------------------------------
    * xor of input signs determines output sign
    *------------------------------------------------------------------------*/
    unsigned neg_result= (SIGND(arg1) ^ SIGND(arg2));
    int      exp_delta = (int)EXPD(arg1) - (int)EXPD(arg2);

    /*-------------------------------------------------------------------------
    * Inf / anything or anything / zero is inf, also inf if / overflows
    *------------------------------------------------------------------------*/
    if (EXPD(arg1) == 2047 || arg2 == 0.0 || exp_delta > 1024) 
        return neg_result ? NEGINFD : INFD;

    /*-------------------------------------------------------------------------
    * anything / inf is zero, also 0 if / underflows
    *------------------------------------------------------------------------*/
    if (EXPD(arg2) == 2047 || exp_delta < -1023) 
        return neg_result ? NEGZEROD : 0.0;

    /*-------------------------------------------------------------------------
    * If the C6x divisor recip will underflow, we divide dividend and divisor 
    * by 2*4 to prevent the underflow.  Previous early returns have ensured 
    * that this division is ok for the dividend.
    *------------------------------------------------------------------------*/
    if (EXPD(arg2) > 2043 )
    {
        union IEEEd2bits u1; u1.d = arg1;
        union IEEEd2bits u2; u2.d = arg2;
        u2.bits.exp -= 4;
        u1.bits.exp -= 4;
        arg2 = u2.d;
        arg1 = u1.d;
    }

    double X0 = _rcpdp(arg2);
    double X1 = X0 *  (2.0 - arg2 * X0);
                X1 *= (2.0 - arg2 * X1);
                X1 *= (2.0 - arg2 * X1);
                X1 *= (2.0 - arg2 * X1);

    return arg1 * X1;
}

DLLEXPORT
float native_dividef(float arg1, float arg2)
{
    float X0 = _rcpsp(arg2);
    float X1 = X0 *  (2.0f - arg2 * X0);
               X1 *= (2.0f - arg2 * X1);  // sufficient for 2.9 ULP
    return arg1 * X1;
}

DLLEXPORT
float native_divide(double arg1, double arg2)
{
    float X0 = _rcpdp(arg2);
    float X1 = X0 *  (2.0 - arg2 * X0);
               X1 *= (2.0 - arg2 * X1);  
               X1 *= (2.0 - arg2 * X1);  
    return arg1 * X1;
}
