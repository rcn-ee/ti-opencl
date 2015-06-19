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

_CLC_OVERLOAD _CLC_DEF float modf(float x, global  float * iptr) 
{ 
   float temp;
   float itemp;
   temp = modff(x, (float*)&itemp); 
   *iptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF float modf(float x, local   float * iptr)
{ 
   float temp;
   float itemp;
   temp = modff(x, (float*)&itemp); 
   *iptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF float modf(float x, private float * iptr)
{ 
   float temp;
   float itemp;
   temp = modff(x, (float*)&itemp); 
   *iptr = itemp;
   return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 modf(float2 x, global  float2 * ptr) 
{
    float2 temp;
    float2 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 modf(float2 x, local   float2 * ptr) 
{
    float2 temp;
    float2 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 modf(float2 x, private float2 * ptr) 
{
    float2 temp;
    float2 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float3 modf(float3 x, global  float3 * ptr) 
{
    float3 temp;
    float3 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 modf(float3 x, local   float3 * ptr) 
{
    float3 temp;
    float3 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 modf(float3 x, private float3 * ptr) 
{
    float3 temp;
    float3 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float4 modf(float4 x, global  float4 * ptr)
{
    float4 temp;
    float4 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 modf(float4 x, local   float4 * ptr)
{
    float4 temp;
    float4 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 modf(float4 x, private float4 * ptr)
{
    float4 temp;
    float4 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 modf(float8 x, global  float8 * ptr) 
{
    float8 temp;
    float8 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    temp.s4 = modff(x.s4, &(((float*)&itemp)[4]));
    temp.s5 = modff(x.s5, &(((float*)&itemp)[5]));
    temp.s6 = modff(x.s6, &(((float*)&itemp)[6]));
    temp.s7 = modff(x.s7, &(((float*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 modf(float8 x, local   float8 * ptr) 
{
    float8 temp;
    float8 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    temp.s4 = modff(x.s4, &(((float*)&itemp)[4]));
    temp.s5 = modff(x.s5, &(((float*)&itemp)[5]));
    temp.s6 = modff(x.s6, &(((float*)&itemp)[6]));
    temp.s7 = modff(x.s7, &(((float*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float8 modf(float8 x, private float8 * ptr) 
{
    float8 temp;
    float8 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    temp.s4 = modff(x.s4, &(((float*)&itemp)[4]));
    temp.s5 = modff(x.s5, &(((float*)&itemp)[5]));
    temp.s6 = modff(x.s6, &(((float*)&itemp)[6]));
    temp.s7 = modff(x.s7, &(((float*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float16 modf(float16 x, global  float16 * ptr) 
{
    float16 temp;
    float16 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    temp.s4 = modff(x.s4, &(((float*)&itemp)[4]));
    temp.s5 = modff(x.s5, &(((float*)&itemp)[5]));
    temp.s6 = modff(x.s6, &(((float*)&itemp)[6]));
    temp.s7 = modff(x.s7, &(((float*)&itemp)[7]));
    temp.s8 = modff(x.s8, &(((float*)&itemp)[8]));
    temp.s9 = modff(x.s9, &(((float*)&itemp)[9]));
    temp.sa = modff(x.sa, &(((float*)&itemp)[10]));
    temp.sb = modff(x.sb, &(((float*)&itemp)[11]));
    temp.sc = modff(x.sc, &(((float*)&itemp)[12]));
    temp.sd = modff(x.sd, &(((float*)&itemp)[13]));
    temp.se = modff(x.se, &(((float*)&itemp)[14]));
    temp.sf = modff(x.sf, &(((float*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 modf(float16 x, local   float16 * ptr) 
{
    float16 temp;
    float16 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    temp.s4 = modff(x.s4, &(((float*)&itemp)[4]));
    temp.s5 = modff(x.s5, &(((float*)&itemp)[5]));
    temp.s6 = modff(x.s6, &(((float*)&itemp)[6]));
    temp.s7 = modff(x.s7, &(((float*)&itemp)[7]));
    temp.s8 = modff(x.s8, &(((float*)&itemp)[8]));
    temp.s9 = modff(x.s9, &(((float*)&itemp)[9]));
    temp.sa = modff(x.sa, &(((float*)&itemp)[10]));
    temp.sb = modff(x.sb, &(((float*)&itemp)[11]));
    temp.sc = modff(x.sc, &(((float*)&itemp)[12]));
    temp.sd = modff(x.sd, &(((float*)&itemp)[13]));
    temp.se = modff(x.se, &(((float*)&itemp)[14]));
    temp.sf = modff(x.sf, &(((float*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 modf(float16 x, private float16 * ptr) 
{
    float16 temp;
    float16 itemp;
    temp.s0 = modff(x.s0, &(((float*)&itemp)[0]));
    temp.s1 = modff(x.s1, &(((float*)&itemp)[1]));
    temp.s2 = modff(x.s2, &(((float*)&itemp)[2]));
    temp.s3 = modff(x.s3, &(((float*)&itemp)[3]));
    temp.s4 = modff(x.s4, &(((float*)&itemp)[4]));
    temp.s5 = modff(x.s5, &(((float*)&itemp)[5]));
    temp.s6 = modff(x.s6, &(((float*)&itemp)[6]));
    temp.s7 = modff(x.s7, &(((float*)&itemp)[7]));
    temp.s8 = modff(x.s8, &(((float*)&itemp)[8]));
    temp.s9 = modff(x.s9, &(((float*)&itemp)[9]));
    temp.sa = modff(x.sa, &(((float*)&itemp)[10]));
    temp.sb = modff(x.sb, &(((float*)&itemp)[11]));
    temp.sc = modff(x.sc, &(((float*)&itemp)[12]));
    temp.sd = modff(x.sd, &(((float*)&itemp)[13]));
    temp.se = modff(x.se, &(((float*)&itemp)[14]));
    temp.sf = modff(x.sf, &(((float*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}




_CLC_OVERLOAD _CLC_DEF double modf(double x, global  double * ptr)
{ 
   double temp;
   double itemp;
   temp = modfd(x, (double*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF double modf(double x, local   double * ptr)
{ 
   double temp;
   double itemp;
   temp = modfd(x, (double*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF double modf(double x, private double * ptr)
{ 
   double temp;
   double itemp;
   temp = modfd(x, (double*)&itemp); 
   *ptr = itemp;
   return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 modf(double2 x, global  double2 * ptr) 
{
    double2 temp;
    double2 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 modf(double2 x, local   double2 * ptr) 
{
    double2 temp;
    double2 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 modf(double2 x, private double2 * ptr) 
{
    double2 temp;
    double2 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double3 modf(double3 x, global  double3 * ptr) 
{
    double3 temp;
    double3 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 modf(double3 x, local   double3 * ptr) 
{
    double3 temp;
    double3 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 modf(double3 x, private double3 * ptr) 
{
    double3 temp;
    double3 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double4 modf(double4 x, global  double4 * ptr)
{
    double4 temp;
    double4 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 modf(double4 x, local   double4 * ptr)
{
    double4 temp;
    double4 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 modf(double4 x, private double4 * ptr)
{
    double4 temp;
    double4 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 modf(double8 x, global  double8 * ptr) 
{
    double8 temp;
    double8 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    temp.s4 = modfd(x.s4, &(((double*)&itemp)[4]));
    temp.s5 = modfd(x.s5, &(((double*)&itemp)[5]));
    temp.s6 = modfd(x.s6, &(((double*)&itemp)[6]));
    temp.s7 = modfd(x.s7, &(((double*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 modf(double8 x, local   double8 * ptr) 
{
    double8 temp;
    double8 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    temp.s4 = modfd(x.s4, &(((double*)&itemp)[4]));
    temp.s5 = modfd(x.s5, &(((double*)&itemp)[5]));
    temp.s6 = modfd(x.s6, &(((double*)&itemp)[6]));
    temp.s7 = modfd(x.s7, &(((double*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double8 modf(double8 x, private double8 * ptr) 
{
    double8 temp;
    double8 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    temp.s4 = modfd(x.s4, &(((double*)&itemp)[4]));
    temp.s5 = modfd(x.s5, &(((double*)&itemp)[5]));
    temp.s6 = modfd(x.s6, &(((double*)&itemp)[6]));
    temp.s7 = modfd(x.s7, &(((double*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double16 modf(double16 x, global  double16 * ptr) 
{
    double16 temp;
    double16 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    temp.s4 = modfd(x.s4, &(((double*)&itemp)[4]));
    temp.s5 = modfd(x.s5, &(((double*)&itemp)[5]));
    temp.s6 = modfd(x.s6, &(((double*)&itemp)[6]));
    temp.s7 = modfd(x.s7, &(((double*)&itemp)[7]));
    temp.s8 = modfd(x.s8, &(((double*)&itemp)[8]));
    temp.s9 = modfd(x.s9, &(((double*)&itemp)[9]));
    temp.sa = modfd(x.sa, &(((double*)&itemp)[10]));
    temp.sb = modfd(x.sb, &(((double*)&itemp)[11]));
    temp.sc = modfd(x.sc, &(((double*)&itemp)[12]));
    temp.sd = modfd(x.sd, &(((double*)&itemp)[13]));
    temp.se = modfd(x.se, &(((double*)&itemp)[14]));
    temp.sf = modfd(x.sf, &(((double*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 modf(double16 x, local   double16 * ptr) 
{
    double16 temp;
    double16 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    temp.s4 = modfd(x.s4, &(((double*)&itemp)[4]));
    temp.s5 = modfd(x.s5, &(((double*)&itemp)[5]));
    temp.s6 = modfd(x.s6, &(((double*)&itemp)[6]));
    temp.s7 = modfd(x.s7, &(((double*)&itemp)[7]));
    temp.s8 = modfd(x.s8, &(((double*)&itemp)[8]));
    temp.s9 = modfd(x.s9, &(((double*)&itemp)[9]));
    temp.sa = modfd(x.sa, &(((double*)&itemp)[10]));
    temp.sb = modfd(x.sb, &(((double*)&itemp)[11]));
    temp.sc = modfd(x.sc, &(((double*)&itemp)[12]));
    temp.sd = modfd(x.sd, &(((double*)&itemp)[13]));
    temp.se = modfd(x.se, &(((double*)&itemp)[14]));
    temp.sf = modfd(x.sf, &(((double*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 modf(double16 x, private double16 * ptr) 
{
    double16 temp;
    double16 itemp;
    temp.s0 = modfd(x.s0, &(((double*)&itemp)[0]));
    temp.s1 = modfd(x.s1, &(((double*)&itemp)[1]));
    temp.s2 = modfd(x.s2, &(((double*)&itemp)[2]));
    temp.s3 = modfd(x.s3, &(((double*)&itemp)[3]));
    temp.s4 = modfd(x.s4, &(((double*)&itemp)[4]));
    temp.s5 = modfd(x.s5, &(((double*)&itemp)[5]));
    temp.s6 = modfd(x.s6, &(((double*)&itemp)[6]));
    temp.s7 = modfd(x.s7, &(((double*)&itemp)[7]));
    temp.s8 = modfd(x.s8, &(((double*)&itemp)[8]));
    temp.s9 = modfd(x.s9, &(((double*)&itemp)[9]));
    temp.sa = modfd(x.sa, &(((double*)&itemp)[10]));
    temp.sb = modfd(x.sb, &(((double*)&itemp)[11]));
    temp.sc = modfd(x.sc, &(((double*)&itemp)[12]));
    temp.sd = modfd(x.sd, &(((double*)&itemp)[13]));
    temp.se = modfd(x.se, &(((double*)&itemp)[14]));
    temp.sf = modfd(x.sf, &(((double*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
