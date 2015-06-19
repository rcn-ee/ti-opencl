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

_CLC_PROTECTED float  lgammaf_r(float  x, int * ptr);
_CLC_PROTECTED double lgammad_r(double x, int * ptr);

_CLC_OVERLOAD _CLC_DEF float lgamma_r(float x, global  int * ptr) 
{ 
   float temp;
   int itemp;
   temp = lgammaf_r(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF float lgamma_r(float x, local   int * ptr)
{ 
   float temp;
   int itemp;
   temp = lgammaf_r(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF float lgamma_r(float x, private int * ptr)
{ 
   float temp;
   int itemp;
   temp = lgammaf_r(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 lgamma_r(float2 x, global  int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 lgamma_r(float2 x, local   int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 lgamma_r(float2 x, private int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float3 lgamma_r(float3 x, global  int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 lgamma_r(float3 x, local   int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 lgamma_r(float3 x, private int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float4 lgamma_r(float4 x, global  int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 lgamma_r(float4 x, local   int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 lgamma_r(float4 x, private int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 lgamma_r(float8 x, global  int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammaf_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammaf_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammaf_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammaf_r(x.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 lgamma_r(float8 x, local   int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammaf_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammaf_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammaf_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammaf_r(x.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float8 lgamma_r(float8 x, private int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammaf_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammaf_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammaf_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammaf_r(x.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float16 lgamma_r(float16 x, global  int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammaf_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammaf_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammaf_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammaf_r(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = lgammaf_r(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = lgammaf_r(x.s9, &(((int*)&itemp)[9]));
    temp.sa = lgammaf_r(x.sa, &(((int*)&itemp)[10]));
    temp.sb = lgammaf_r(x.sb, &(((int*)&itemp)[11]));
    temp.sc = lgammaf_r(x.sc, &(((int*)&itemp)[12]));
    temp.sd = lgammaf_r(x.sd, &(((int*)&itemp)[13]));
    temp.se = lgammaf_r(x.se, &(((int*)&itemp)[14]));
    temp.sf = lgammaf_r(x.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 lgamma_r(float16 x, local   int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammaf_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammaf_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammaf_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammaf_r(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = lgammaf_r(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = lgammaf_r(x.s9, &(((int*)&itemp)[9]));
    temp.sa = lgammaf_r(x.sa, &(((int*)&itemp)[10]));
    temp.sb = lgammaf_r(x.sb, &(((int*)&itemp)[11]));
    temp.sc = lgammaf_r(x.sc, &(((int*)&itemp)[12]));
    temp.sd = lgammaf_r(x.sd, &(((int*)&itemp)[13]));
    temp.se = lgammaf_r(x.se, &(((int*)&itemp)[14]));
    temp.sf = lgammaf_r(x.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 lgamma_r(float16 x, private int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = lgammaf_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammaf_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammaf_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammaf_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammaf_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammaf_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammaf_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammaf_r(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = lgammaf_r(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = lgammaf_r(x.s9, &(((int*)&itemp)[9]));
    temp.sa = lgammaf_r(x.sa, &(((int*)&itemp)[10]));
    temp.sb = lgammaf_r(x.sb, &(((int*)&itemp)[11]));
    temp.sc = lgammaf_r(x.sc, &(((int*)&itemp)[12]));
    temp.sd = lgammaf_r(x.sd, &(((int*)&itemp)[13]));
    temp.se = lgammaf_r(x.se, &(((int*)&itemp)[14]));
    temp.sf = lgammaf_r(x.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}




_CLC_OVERLOAD _CLC_DEF double lgamma_r(double x, global  int * ptr)
{ 
   double temp;
   int itemp;
   temp = lgammad_r(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF double lgamma_r(double x, local   int * ptr)
{ 
   double temp;
   int itemp;
   temp = lgammad_r(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF double lgamma_r(double x, private int * ptr)
{ 
   double temp;
   int itemp;
   temp = lgammad_r(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 lgamma_r(double2 x, global  int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 lgamma_r(double2 x, local   int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 lgamma_r(double2 x, private int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double3 lgamma_r(double3 x, global  int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 lgamma_r(double3 x, local   int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 lgamma_r(double3 x, private int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double4 lgamma_r(double4 x, global  int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 lgamma_r(double4 x, local   int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 lgamma_r(double4 x, private int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 lgamma_r(double8 x, global  int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammad_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammad_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammad_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammad_r(x.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 lgamma_r(double8 x, local   int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammad_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammad_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammad_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammad_r(x.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double8 lgamma_r(double8 x, private int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammad_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammad_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammad_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammad_r(x.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double16 lgamma_r(double16 x, global  int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammad_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammad_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammad_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammad_r(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = lgammad_r(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = lgammad_r(x.s9, &(((int*)&itemp)[9]));
    temp.sa = lgammad_r(x.sa, &(((int*)&itemp)[10]));
    temp.sb = lgammad_r(x.sb, &(((int*)&itemp)[11]));
    temp.sc = lgammad_r(x.sc, &(((int*)&itemp)[12]));
    temp.sd = lgammad_r(x.sd, &(((int*)&itemp)[13]));
    temp.se = lgammad_r(x.se, &(((int*)&itemp)[14]));
    temp.sf = lgammad_r(x.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 lgamma_r(double16 x, local   int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammad_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammad_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammad_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammad_r(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = lgammad_r(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = lgammad_r(x.s9, &(((int*)&itemp)[9]));
    temp.sa = lgammad_r(x.sa, &(((int*)&itemp)[10]));
    temp.sb = lgammad_r(x.sb, &(((int*)&itemp)[11]));
    temp.sc = lgammad_r(x.sc, &(((int*)&itemp)[12]));
    temp.sd = lgammad_r(x.sd, &(((int*)&itemp)[13]));
    temp.se = lgammad_r(x.se, &(((int*)&itemp)[14]));
    temp.sf = lgammad_r(x.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 lgamma_r(double16 x, private int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = lgammad_r(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = lgammad_r(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = lgammad_r(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = lgammad_r(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = lgammad_r(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = lgammad_r(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = lgammad_r(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = lgammad_r(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = lgammad_r(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = lgammad_r(x.s9, &(((int*)&itemp)[9]));
    temp.sa = lgammad_r(x.sa, &(((int*)&itemp)[10]));
    temp.sb = lgammad_r(x.sb, &(((int*)&itemp)[11]));
    temp.sc = lgammad_r(x.sc, &(((int*)&itemp)[12]));
    temp.sd = lgammad_r(x.sd, &(((int*)&itemp)[13]));
    temp.se = lgammad_r(x.se, &(((int*)&itemp)[14]));
    temp.sf = lgammad_r(x.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
