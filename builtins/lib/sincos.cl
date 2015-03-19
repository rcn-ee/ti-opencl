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

_CLC_OVERLOAD _CLC_DEF float2 sincos(float2 x, global  float2 * cosval) 
{
    float2 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 sincos(float2 x, local   float2 * cosval) 
{
    float2 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 sincos(float2 x, private float2 * cosval) 
{
    float2 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float3 sincos(float3 x, global  float3 * cosval) 
{
    float3 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 sincos(float3 x, local   float3 * cosval) 
{
    float3 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 sincos(float3 x, private float3 * cosval) 
{
    float3 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float4 sincos(float4 x, global  float4 * cosval)
{
    float4 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 sincos(float4 x, local   float4 * cosval)
{
    float4 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 sincos(float4 x, private float4 * cosval)
{
    float4 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 sincos(float8 x, global  float8 * cosval) 
{
    float8 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((float*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((float*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((float*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((float*)cosval)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 sincos(float8 x, local   float8 * cosval) 
{
    float8 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((float*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((float*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((float*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((float*)cosval)[7]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float8 sincos(float8 x, private float8 * cosval) 
{
    float8 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((float*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((float*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((float*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((float*)cosval)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float16 sincos(float16 x, global  float16 * cosval) 
{
    float16 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((float*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((float*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((float*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((float*)cosval)[7]));
    temp.s8 = sincos(x.s8, &(((float*)cosval)[8]));
    temp.s9 = sincos(x.s9, &(((float*)cosval)[9]));
    temp.sa = sincos(x.sa, &(((float*)cosval)[10]));
    temp.sb = sincos(x.sb, &(((float*)cosval)[11]));
    temp.sc = sincos(x.sc, &(((float*)cosval)[12]));
    temp.sd = sincos(x.sd, &(((float*)cosval)[13]));
    temp.se = sincos(x.se, &(((float*)cosval)[14]));
    temp.sf = sincos(x.sf, &(((float*)cosval)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 sincos(float16 x, local   float16 * cosval) 
{
    float16 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((float*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((float*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((float*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((float*)cosval)[7]));
    temp.s8 = sincos(x.s8, &(((float*)cosval)[8]));
    temp.s9 = sincos(x.s9, &(((float*)cosval)[9]));
    temp.sa = sincos(x.sa, &(((float*)cosval)[10]));
    temp.sb = sincos(x.sb, &(((float*)cosval)[11]));
    temp.sc = sincos(x.sc, &(((float*)cosval)[12]));
    temp.sd = sincos(x.sd, &(((float*)cosval)[13]));
    temp.se = sincos(x.se, &(((float*)cosval)[14]));
    temp.sf = sincos(x.sf, &(((float*)cosval)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 sincos(float16 x, private float16 * cosval) 
{
    float16 temp;
    temp.s0 = sincos(x.s0, &(((float*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((float*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((float*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((float*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((float*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((float*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((float*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((float*)cosval)[7]));
    temp.s8 = sincos(x.s8, &(((float*)cosval)[8]));
    temp.s9 = sincos(x.s9, &(((float*)cosval)[9]));
    temp.sa = sincos(x.sa, &(((float*)cosval)[10]));
    temp.sb = sincos(x.sb, &(((float*)cosval)[11]));
    temp.sc = sincos(x.sc, &(((float*)cosval)[12]));
    temp.sd = sincos(x.sd, &(((float*)cosval)[13]));
    temp.se = sincos(x.se, &(((float*)cosval)[14]));
    temp.sf = sincos(x.sf, &(((float*)cosval)[15]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 sincos(double2 x, global  double2 * cosval) 
{
    double2 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 sincos(double2 x, local   double2 * cosval) 
{
    double2 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 sincos(double2 x, private double2 * cosval) 
{
    double2 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double3 sincos(double3 x, global  double3 * cosval) 
{
    double3 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 sincos(double3 x, local   double3 * cosval) 
{
    double3 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 sincos(double3 x, private double3 * cosval) 
{
    double3 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double4 sincos(double4 x, global  double4 * cosval)
{
    double4 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 sincos(double4 x, local   double4 * cosval)
{
    double4 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 sincos(double4 x, private double4 * cosval)
{
    double4 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 sincos(double8 x, global  double8 * cosval) 
{
    double8 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((double*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((double*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((double*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((double*)cosval)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 sincos(double8 x, local   double8 * cosval) 
{
    double8 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((double*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((double*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((double*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((double*)cosval)[7]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double8 sincos(double8 x, private double8 * cosval) 
{
    double8 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((double*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((double*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((double*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((double*)cosval)[7]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double16 sincos(double16 x, global  double16 * cosval) 
{
    double16 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((double*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((double*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((double*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((double*)cosval)[7]));
    temp.s8 = sincos(x.s8, &(((double*)cosval)[8]));
    temp.s9 = sincos(x.s9, &(((double*)cosval)[9]));
    temp.sa = sincos(x.sa, &(((double*)cosval)[10]));
    temp.sb = sincos(x.sb, &(((double*)cosval)[11]));
    temp.sc = sincos(x.sc, &(((double*)cosval)[12]));
    temp.sd = sincos(x.sd, &(((double*)cosval)[13]));
    temp.se = sincos(x.se, &(((double*)cosval)[14]));
    temp.sf = sincos(x.sf, &(((double*)cosval)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 sincos(double16 x, local   double16 * cosval) 
{
    double16 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((double*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((double*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((double*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((double*)cosval)[7]));
    temp.s8 = sincos(x.s8, &(((double*)cosval)[8]));
    temp.s9 = sincos(x.s9, &(((double*)cosval)[9]));
    temp.sa = sincos(x.sa, &(((double*)cosval)[10]));
    temp.sb = sincos(x.sb, &(((double*)cosval)[11]));
    temp.sc = sincos(x.sc, &(((double*)cosval)[12]));
    temp.sd = sincos(x.sd, &(((double*)cosval)[13]));
    temp.se = sincos(x.se, &(((double*)cosval)[14]));
    temp.sf = sincos(x.sf, &(((double*)cosval)[15]));
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 sincos(double16 x, private double16 * cosval) 
{
    double16 temp;
    temp.s0 = sincos(x.s0, &(((double*)cosval)[0]));
    temp.s1 = sincos(x.s1, &(((double*)cosval)[1]));
    temp.s2 = sincos(x.s2, &(((double*)cosval)[2]));
    temp.s3 = sincos(x.s3, &(((double*)cosval)[3]));
    temp.s4 = sincos(x.s4, &(((double*)cosval)[4]));
    temp.s5 = sincos(x.s5, &(((double*)cosval)[5]));
    temp.s6 = sincos(x.s6, &(((double*)cosval)[6]));
    temp.s7 = sincos(x.s7, &(((double*)cosval)[7]));
    temp.s8 = sincos(x.s8, &(((double*)cosval)[8]));
    temp.s9 = sincos(x.s9, &(((double*)cosval)[9]));
    temp.sa = sincos(x.sa, &(((double*)cosval)[10]));
    temp.sb = sincos(x.sb, &(((double*)cosval)[11]));
    temp.sc = sincos(x.sc, &(((double*)cosval)[12]));
    temp.sd = sincos(x.sd, &(((double*)cosval)[13]));
    temp.se = sincos(x.se, &(((double*)cosval)[14]));
    temp.sf = sincos(x.sf, &(((double*)cosval)[15]));
    return temp;
}
