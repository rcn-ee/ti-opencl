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

_CLC_PROTECTED float  frexpf(float  x, int * ptr);
_CLC_PROTECTED double frexpd(double x, int * ptr);

_CLC_OVERLOAD _CLC_DEF float frexp(float x, global  int * ptr) 
{ 
   float temp;
   int itemp;
   temp = frexpf(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF float frexp(float x, local   int * ptr) 
{ 
   float temp;
   int itemp;
   temp = frexpf(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF float frexp(float x, private int * ptr) 
{ 
   float temp;
   int itemp;
   temp = frexpf(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 frexp(float2 x, global  int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    *ptr=itemp;
    return temp;
}


_CLC_OVERLOAD _CLC_DEF float2 frexp(float2 x, local   int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 frexp(float2 x, private int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float3 frexp(float3 x, global  int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 frexp(float3 x, local   int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 frexp(float3 x, private int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float4 frexp(float4 x, global  int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 frexp(float4 x, local   int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 frexp(float4 x, private int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 frexp(float8 x, global  int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpf(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpf(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpf(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpf(x.s7, &(((int*)&itemp)[7]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 frexp(float8 x, local   int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpf(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpf(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpf(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpf(x.s7, &(((int*)&itemp)[7]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float8 frexp(float8 x, private int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpf(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpf(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpf(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpf(x.s7, &(((int*)&itemp)[7]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float16 frexp(float16 x, global  int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpf(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpf(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpf(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpf(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = frexpf(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = frexpf(x.s9, &(((int*)&itemp)[9]));
    temp.sa = frexpf(x.sa, &(((int*)&itemp)[10]));
    temp.sb = frexpf(x.sb, &(((int*)&itemp)[11]));
    temp.sc = frexpf(x.sc, &(((int*)&itemp)[12]));
    temp.sd = frexpf(x.sd, &(((int*)&itemp)[13]));
    temp.se = frexpf(x.se, &(((int*)&itemp)[14]));
    temp.sf = frexpf(x.sf, &(((int*)&itemp)[15]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 frexp(float16 x, local   int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpf(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpf(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpf(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpf(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = frexpf(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = frexpf(x.s9, &(((int*)&itemp)[9]));
    temp.sa = frexpf(x.sa, &(((int*)&itemp)[10]));
    temp.sb = frexpf(x.sb, &(((int*)&itemp)[11]));
    temp.sc = frexpf(x.sc, &(((int*)&itemp)[12]));
    temp.sd = frexpf(x.sd, &(((int*)&itemp)[13]));
    temp.se = frexpf(x.se, &(((int*)&itemp)[14]));
    temp.sf = frexpf(x.sf, &(((int*)&itemp)[15]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 frexp(float16 x, private int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = frexpf(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpf(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpf(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpf(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpf(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpf(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpf(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpf(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = frexpf(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = frexpf(x.s9, &(((int*)&itemp)[9]));
    temp.sa = frexpf(x.sa, &(((int*)&itemp)[10]));
    temp.sb = frexpf(x.sb, &(((int*)&itemp)[11]));
    temp.sc = frexpf(x.sc, &(((int*)&itemp)[12]));
    temp.sd = frexpf(x.sd, &(((int*)&itemp)[13]));
    temp.se = frexpf(x.se, &(((int*)&itemp)[14]));
    temp.sf = frexpf(x.sf, &(((int*)&itemp)[15]));
    *ptr=itemp;
    return temp;
}




_CLC_OVERLOAD _CLC_DEF double frexp(double x, global  int * ptr) 
{ 
   float temp;
   int itemp;
   temp = frexpd(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF double frexp(double x, local   int * ptr)
{ 
   float temp;
   int itemp;
   temp = frexpd(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}
_CLC_OVERLOAD _CLC_DEF double frexp(double x, private int * ptr)
{ 
   float temp;
   int itemp;
   temp = frexpd(x, (int*)&itemp); 
   *ptr = itemp;
   return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 frexp(double2 x, global  int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 frexp(double2 x, local   int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 frexp(double2 x, private int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double3 frexp(double3 x, global  int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 frexp(double3 x, local   int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 frexp(double3 x, private int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double4 frexp(double4 x, global  int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 frexp(double4 x, local   int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 frexp(double4 x, private int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 frexp(double8 x, global  int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpd(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpd(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpd(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpd(x.s7, &(((int*)&itemp)[7]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 frexp(double8 x, local   int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpd(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpd(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpd(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpd(x.s7, &(((int*)&itemp)[7]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double8 frexp(double8 x, private int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpd(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpd(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpd(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpd(x.s7, &(((int*)&itemp)[7]));
    *ptr=itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double16 frexp(double16 x, global  int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpd(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpd(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpd(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpd(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = frexpd(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = frexpd(x.s9, &(((int*)&itemp)[9]));
    temp.sa = frexpd(x.sa, &(((int*)&itemp)[10]));
    temp.sb = frexpd(x.sb, &(((int*)&itemp)[11]));
    temp.sc = frexpd(x.sc, &(((int*)&itemp)[12]));
    temp.sd = frexpd(x.sd, &(((int*)&itemp)[13]));
    temp.se = frexpd(x.se, &(((int*)&itemp)[14]));
    temp.sf = frexpd(x.sf, &(((int*)&itemp)[15]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 frexp(double16 x, local   int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpd(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpd(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpd(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpd(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = frexpd(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = frexpd(x.s9, &(((int*)&itemp)[9]));
    temp.sa = frexpd(x.sa, &(((int*)&itemp)[10]));
    temp.sb = frexpd(x.sb, &(((int*)&itemp)[11]));
    temp.sc = frexpd(x.sc, &(((int*)&itemp)[12]));
    temp.sd = frexpd(x.sd, &(((int*)&itemp)[13]));
    temp.se = frexpd(x.se, &(((int*)&itemp)[14]));
    temp.sf = frexpd(x.sf, &(((int*)&itemp)[15]));
    *ptr=itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 frexp(double16 x, private int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = frexpd(x.s0, &(((int*)&itemp)[0]));
    temp.s1 = frexpd(x.s1, &(((int*)&itemp)[1]));
    temp.s2 = frexpd(x.s2, &(((int*)&itemp)[2]));
    temp.s3 = frexpd(x.s3, &(((int*)&itemp)[3]));
    temp.s4 = frexpd(x.s4, &(((int*)&itemp)[4]));
    temp.s5 = frexpd(x.s5, &(((int*)&itemp)[5]));
    temp.s6 = frexpd(x.s6, &(((int*)&itemp)[6]));
    temp.s7 = frexpd(x.s7, &(((int*)&itemp)[7]));
    temp.s8 = frexpd(x.s8, &(((int*)&itemp)[8]));
    temp.s9 = frexpd(x.s9, &(((int*)&itemp)[9]));
    temp.sa = frexpd(x.sa, &(((int*)&itemp)[10]));
    temp.sb = frexpd(x.sb, &(((int*)&itemp)[11]));
    temp.sc = frexpd(x.sc, &(((int*)&itemp)[12]));
    temp.sd = frexpd(x.sd, &(((int*)&itemp)[13]));
    temp.se = frexpd(x.se, &(((int*)&itemp)[14]));
    temp.sf = frexpd(x.sf, &(((int*)&itemp)[15]));
    *ptr=itemp;
    return temp;
}
