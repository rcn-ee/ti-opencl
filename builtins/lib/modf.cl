/******************************************************************************
 * Copyright (c) 2011-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "clc.h"

_CLC_PROTECTED float  modff(float  x, float  * iptr);
_CLC_PROTECTED double modfd(double x, double * iptr);

_CLC_OVERLOAD _CLC_DEF float modf(float x, global  float * iptr) { return modff(x, (float*)iptr); }
_CLC_OVERLOAD _CLC_DEF float modf(float x, local   float * iptr) { return modff(x, (float*)iptr); }
_CLC_OVERLOAD _CLC_DEF float modf(float x, private float * iptr) { return modff(x, (float*)iptr); }

_CLC_OVERLOAD _CLC_DEF float2 modf(float2 x, global  float2 * ptr) 
{
    float2 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 modf(float2 x, local   float2 * ptr) 
{
    float2 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 modf(float2 x, private float2 * ptr) 
{
    float2 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float3 modf(float3 x, global  float3 * ptr) 
{
    float3 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 modf(float3 x, local   float3 * ptr) 
{
    float3 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 modf(float3 x, private float3 * ptr) 
{
    float3 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float4 modf(float4 x, global  float4 * ptr)
{
    float4 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 modf(float4 x, local   float4 * ptr)
{
    float4 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 modf(float4 x, private float4 * ptr)
{
    float4 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 modf(float8 x, global  float8 * ptr) 
{
    float8 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    temp.s4 = modff(x.s4, &(((float*)ptr)[4]));
    temp.s5 = modff(x.s5, &(((float*)ptr)[5]));
    temp.s6 = modff(x.s6, &(((float*)ptr)[6]));
    temp.s7 = modff(x.s7, &(((float*)ptr)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 modf(float8 x, local   float8 * ptr) 
{
    float8 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    temp.s4 = modff(x.s4, &(((float*)ptr)[4]));
    temp.s5 = modff(x.s5, &(((float*)ptr)[5]));
    temp.s6 = modff(x.s6, &(((float*)ptr)[6]));
    temp.s7 = modff(x.s7, &(((float*)ptr)[7]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float8 modf(float8 x, private float8 * ptr) 
{
    float8 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    temp.s4 = modff(x.s4, &(((float*)ptr)[4]));
    temp.s5 = modff(x.s5, &(((float*)ptr)[5]));
    temp.s6 = modff(x.s6, &(((float*)ptr)[6]));
    temp.s7 = modff(x.s7, &(((float*)ptr)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float16 modf(float16 x, global  float16 * ptr) 
{
    float16 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    temp.s4 = modff(x.s4, &(((float*)ptr)[4]));
    temp.s5 = modff(x.s5, &(((float*)ptr)[5]));
    temp.s6 = modff(x.s6, &(((float*)ptr)[6]));
    temp.s7 = modff(x.s7, &(((float*)ptr)[7]));
    temp.s8 = modff(x.s8, &(((float*)ptr)[8]));
    temp.s9 = modff(x.s9, &(((float*)ptr)[9]));
    temp.sa = modff(x.sa, &(((float*)ptr)[10]));
    temp.sb = modff(x.sb, &(((float*)ptr)[11]));
    temp.sc = modff(x.sc, &(((float*)ptr)[12]));
    temp.sd = modff(x.sd, &(((float*)ptr)[13]));
    temp.se = modff(x.se, &(((float*)ptr)[14]));
    temp.sf = modff(x.sf, &(((float*)ptr)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 modf(float16 x, local   float16 * ptr) 
{
    float16 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    temp.s4 = modff(x.s4, &(((float*)ptr)[4]));
    temp.s5 = modff(x.s5, &(((float*)ptr)[5]));
    temp.s6 = modff(x.s6, &(((float*)ptr)[6]));
    temp.s7 = modff(x.s7, &(((float*)ptr)[7]));
    temp.s8 = modff(x.s8, &(((float*)ptr)[8]));
    temp.s9 = modff(x.s9, &(((float*)ptr)[9]));
    temp.sa = modff(x.sa, &(((float*)ptr)[10]));
    temp.sb = modff(x.sb, &(((float*)ptr)[11]));
    temp.sc = modff(x.sc, &(((float*)ptr)[12]));
    temp.sd = modff(x.sd, &(((float*)ptr)[13]));
    temp.se = modff(x.se, &(((float*)ptr)[14]));
    temp.sf = modff(x.sf, &(((float*)ptr)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 modf(float16 x, private float16 * ptr) 
{
    float16 temp;
    temp.s0 = modff(x.s0, &(((float*)ptr)[0]));
    temp.s1 = modff(x.s1, &(((float*)ptr)[1]));
    temp.s2 = modff(x.s2, &(((float*)ptr)[2]));
    temp.s3 = modff(x.s3, &(((float*)ptr)[3]));
    temp.s4 = modff(x.s4, &(((float*)ptr)[4]));
    temp.s5 = modff(x.s5, &(((float*)ptr)[5]));
    temp.s6 = modff(x.s6, &(((float*)ptr)[6]));
    temp.s7 = modff(x.s7, &(((float*)ptr)[7]));
    temp.s8 = modff(x.s8, &(((float*)ptr)[8]));
    temp.s9 = modff(x.s9, &(((float*)ptr)[9]));
    temp.sa = modff(x.sa, &(((float*)ptr)[10]));
    temp.sb = modff(x.sb, &(((float*)ptr)[11]));
    temp.sc = modff(x.sc, &(((float*)ptr)[12]));
    temp.sd = modff(x.sd, &(((float*)ptr)[13]));
    temp.se = modff(x.se, &(((float*)ptr)[14]));
    temp.sf = modff(x.sf, &(((float*)ptr)[15]));
    return temp;
}




_CLC_OVERLOAD _CLC_DEF double modf(double x, global  double * ptr) { return modfd(x, (double*)ptr); }
_CLC_OVERLOAD _CLC_DEF double modf(double x, local   double * ptr) { return modfd(x, (double*)ptr); }
_CLC_OVERLOAD _CLC_DEF double modf(double x, private double * ptr) { return modfd(x, (double*)ptr); }

_CLC_OVERLOAD _CLC_DEF double2 modf(double2 x, global  double2 * ptr) 
{
    double2 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 modf(double2 x, local   double2 * ptr) 
{
    double2 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 modf(double2 x, private double2 * ptr) 
{
    double2 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double3 modf(double3 x, global  double3 * ptr) 
{
    double3 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 modf(double3 x, local   double3 * ptr) 
{
    double3 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 modf(double3 x, private double3 * ptr) 
{
    double3 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double4 modf(double4 x, global  double4 * ptr)
{
    double4 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 modf(double4 x, local   double4 * ptr)
{
    double4 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 modf(double4 x, private double4 * ptr)
{
    double4 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 modf(double8 x, global  double8 * ptr) 
{
    double8 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    temp.s4 = modfd(x.s4, &(((double*)ptr)[4]));
    temp.s5 = modfd(x.s5, &(((double*)ptr)[5]));
    temp.s6 = modfd(x.s6, &(((double*)ptr)[6]));
    temp.s7 = modfd(x.s7, &(((double*)ptr)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 modf(double8 x, local   double8 * ptr) 
{
    double8 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    temp.s4 = modfd(x.s4, &(((double*)ptr)[4]));
    temp.s5 = modfd(x.s5, &(((double*)ptr)[5]));
    temp.s6 = modfd(x.s6, &(((double*)ptr)[6]));
    temp.s7 = modfd(x.s7, &(((double*)ptr)[7]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double8 modf(double8 x, private double8 * ptr) 
{
    double8 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    temp.s4 = modfd(x.s4, &(((double*)ptr)[4]));
    temp.s5 = modfd(x.s5, &(((double*)ptr)[5]));
    temp.s6 = modfd(x.s6, &(((double*)ptr)[6]));
    temp.s7 = modfd(x.s7, &(((double*)ptr)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double16 modf(double16 x, global  double16 * ptr) 
{
    double16 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    temp.s4 = modfd(x.s4, &(((double*)ptr)[4]));
    temp.s5 = modfd(x.s5, &(((double*)ptr)[5]));
    temp.s6 = modfd(x.s6, &(((double*)ptr)[6]));
    temp.s7 = modfd(x.s7, &(((double*)ptr)[7]));
    temp.s8 = modfd(x.s8, &(((double*)ptr)[8]));
    temp.s9 = modfd(x.s9, &(((double*)ptr)[9]));
    temp.sa = modfd(x.sa, &(((double*)ptr)[10]));
    temp.sb = modfd(x.sb, &(((double*)ptr)[11]));
    temp.sc = modfd(x.sc, &(((double*)ptr)[12]));
    temp.sd = modfd(x.sd, &(((double*)ptr)[13]));
    temp.se = modfd(x.se, &(((double*)ptr)[14]));
    temp.sf = modfd(x.sf, &(((double*)ptr)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 modf(double16 x, local   double16 * ptr) 
{
    double16 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    temp.s4 = modfd(x.s4, &(((double*)ptr)[4]));
    temp.s5 = modfd(x.s5, &(((double*)ptr)[5]));
    temp.s6 = modfd(x.s6, &(((double*)ptr)[6]));
    temp.s7 = modfd(x.s7, &(((double*)ptr)[7]));
    temp.s8 = modfd(x.s8, &(((double*)ptr)[8]));
    temp.s9 = modfd(x.s9, &(((double*)ptr)[9]));
    temp.sa = modfd(x.sa, &(((double*)ptr)[10]));
    temp.sb = modfd(x.sb, &(((double*)ptr)[11]));
    temp.sc = modfd(x.sc, &(((double*)ptr)[12]));
    temp.sd = modfd(x.sd, &(((double*)ptr)[13]));
    temp.se = modfd(x.se, &(((double*)ptr)[14]));
    temp.sf = modfd(x.sf, &(((double*)ptr)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 modf(double16 x, private double16 * ptr) 
{
    double16 temp;
    temp.s0 = modfd(x.s0, &(((double*)ptr)[0]));
    temp.s1 = modfd(x.s1, &(((double*)ptr)[1]));
    temp.s2 = modfd(x.s2, &(((double*)ptr)[2]));
    temp.s3 = modfd(x.s3, &(((double*)ptr)[3]));
    temp.s4 = modfd(x.s4, &(((double*)ptr)[4]));
    temp.s5 = modfd(x.s5, &(((double*)ptr)[5]));
    temp.s6 = modfd(x.s6, &(((double*)ptr)[6]));
    temp.s7 = modfd(x.s7, &(((double*)ptr)[7]));
    temp.s8 = modfd(x.s8, &(((double*)ptr)[8]));
    temp.s9 = modfd(x.s9, &(((double*)ptr)[9]));
    temp.sa = modfd(x.sa, &(((double*)ptr)[10]));
    temp.sb = modfd(x.sb, &(((double*)ptr)[11]));
    temp.sc = modfd(x.sc, &(((double*)ptr)[12]));
    temp.sd = modfd(x.sd, &(((double*)ptr)[13]));
    temp.se = modfd(x.se, &(((double*)ptr)[14]));
    temp.sf = modfd(x.sf, &(((double*)ptr)[15]));
    return temp;
}
