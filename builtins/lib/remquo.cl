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

_CLC_OVERLOAD _CLC_DEF float2 remquo(float2 x, float2 y, global  int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 remquo(float2 x, float2 y, local   int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float2 remquo(float2 x, float2 y, private int2 * ptr) 
{
    float2 temp;
    int2 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float3 remquo(float3 x, float3 y, global  int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 remquo(float3 x, float3 y, local   int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float3 remquo(float3 x, float3 y, private int3 * ptr) 
{
    float3 temp;
    int3 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float4 remquo(float4 x, float4 y, global  int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 remquo(float4 x, float4 y, local   int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float4 remquo(float4 x, float4 y, private int4 * ptr)
{
    float4 temp;
    int4 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 remquo(float8 x, float8 y, global  int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquof(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquof(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquof(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquof(x.s7, y.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float8 remquo(float8 x, float8 y, local   int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquof(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquof(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquof(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquof(x.s7, y.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float8 remquo(float8 x, float8 y, private int8 * ptr) 
{
    float8 temp;
    int8 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquof(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquof(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquof(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquof(x.s7, y.s7, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF float16 remquo(float16 x, float16 y, global  int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquof(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquof(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquof(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquof(x.s7, y.s7, &(((int*)&itemp)[7]));
    temp.s8 = remquof(x.s8, y.s8, &(((int*)&itemp)[8]));
    temp.s9 = remquof(x.s9, y.s9, &(((int*)&itemp)[9]));
    temp.sa = remquof(x.sa, y.sa, &(((int*)&itemp)[10]));
    temp.sb = remquof(x.sb, y.sb, &(((int*)&itemp)[11]));
    temp.sc = remquof(x.sc, y.sc, &(((int*)&itemp)[12]));
    temp.sd = remquof(x.sd, y.sd, &(((int*)&itemp)[13]));
    temp.se = remquof(x.se, y.se, &(((int*)&itemp)[14]));
    temp.sf = remquof(x.sf, y.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 remquo(float16 x, float16 y, local   int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquof(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquof(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquof(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquof(x.s7, y.s7, &(((int*)&itemp)[7]));
    temp.s8 = remquof(x.s8, y.s8, &(((int*)&itemp)[8]));
    temp.s9 = remquof(x.s9, y.s9, &(((int*)&itemp)[9]));
    temp.sa = remquof(x.sa, y.sa, &(((int*)&itemp)[10]));
    temp.sb = remquof(x.sb, y.sb, &(((int*)&itemp)[11]));
    temp.sc = remquof(x.sc, y.sc, &(((int*)&itemp)[12]));
    temp.sd = remquof(x.sd, y.sd, &(((int*)&itemp)[13]));
    temp.se = remquof(x.se, y.se, &(((int*)&itemp)[14]));
    temp.sf = remquof(x.sf, y.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF float16 remquo(float16 x, float16 y, private int16 * ptr) 
{
    float16 temp;
    int16 itemp;
    temp.s0 = remquof(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquof(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquof(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquof(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquof(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquof(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquof(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquof(x.s7, y.s7, &(((int*)&itemp)[7]));
    temp.s8 = remquof(x.s8, y.s8, &(((int*)&itemp)[8]));
    temp.s9 = remquof(x.s9, y.s9, &(((int*)&itemp)[9]));
    temp.sa = remquof(x.sa, y.sa, &(((int*)&itemp)[10]));
    temp.sb = remquof(x.sb, y.sb, &(((int*)&itemp)[11]));
    temp.sc = remquof(x.sc, y.sc, &(((int*)&itemp)[12]));
    temp.sd = remquof(x.sd, y.sd, &(((int*)&itemp)[13]));
    temp.se = remquof(x.se, y.se, &(((int*)&itemp)[14]));
    temp.sf = remquof(x.sf, y.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 remquo(double2 x, double2 y, global  int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 remquo(double2 x, double2 y, local   int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double2 remquo(double2 x, double2 y, private int2 * ptr) 
{
    double2 temp;
    int2 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double3 remquo(double3 x, double3 y, global  int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 remquo(double3 x, double3 y, local   int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double3 remquo(double3 x, double3 y, private int3 * ptr) 
{
    double3 temp;
    int3 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double4 remquo(double4 x, double4 y, global  int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 remquo(double4 x, double4 y, local   int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double4 remquo(double4 x, double4 y, private int4 * ptr)
{
    double4 temp;
    int4 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 remquo(double8 x, double8 y, global  int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquod(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquod(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquod(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquod(x.s7, y.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double8 remquo(double8 x, double8 y, local   int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquod(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquod(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquod(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquod(x.s7, y.s7, &(((int*)&itemp)[7]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double8 remquo(double8 x, double8 y, private int8 * ptr) 
{
    double8 temp;
    int8 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquod(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquod(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquod(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquod(x.s7, y.s7, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}

_CLC_OVERLOAD _CLC_DEF double16 remquo(double16 x, double16 y, global  int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquod(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquod(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquod(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquod(x.s7, y.s7, &(((int*)&itemp)[7]));
    temp.s8 = remquod(x.s8, y.s8, &(((int*)&itemp)[8]));
    temp.s9 = remquod(x.s9, y.s9, &(((int*)&itemp)[9]));
    temp.sa = remquod(x.sa, y.sa, &(((int*)&itemp)[10]));
    temp.sb = remquod(x.sb, y.sb, &(((int*)&itemp)[11]));
    temp.sc = remquod(x.sc, y.sc, &(((int*)&itemp)[12]));
    temp.sd = remquod(x.sd, y.sd, &(((int*)&itemp)[13]));
    temp.se = remquod(x.se, y.se, &(((int*)&itemp)[14]));
    temp.sf = remquod(x.sf, y.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 remquo(double16 x, double16 y, local   int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquod(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquod(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquod(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquod(x.s7, y.s7, &(((int*)&itemp)[7]));
    temp.s8 = remquod(x.s8, y.s8, &(((int*)&itemp)[8]));
    temp.s9 = remquod(x.s9, y.s9, &(((int*)&itemp)[9]));
    temp.sa = remquod(x.sa, y.sa, &(((int*)&itemp)[10]));
    temp.sb = remquod(x.sb, y.sb, &(((int*)&itemp)[11]));
    temp.sc = remquod(x.sc, y.sc, &(((int*)&itemp)[12]));
    temp.sd = remquod(x.sd, y.sd, &(((int*)&itemp)[13]));
    temp.se = remquod(x.se, y.se, &(((int*)&itemp)[14]));
    temp.sf = remquod(x.sf, y.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
_CLC_OVERLOAD _CLC_DEF double16 remquo(double16 x, double16 y, private int16 * ptr) 
{
    double16 temp;
    int16 itemp;
    temp.s0 = remquod(x.s0, y.s0, &(((int*)&itemp)[0]));
    temp.s1 = remquod(x.s1, y.s1, &(((int*)&itemp)[1]));
    temp.s2 = remquod(x.s2, y.s2, &(((int*)&itemp)[2]));
    temp.s3 = remquod(x.s3, y.s3, &(((int*)&itemp)[3]));
    temp.s4 = remquod(x.s4, y.s4, &(((int*)&itemp)[4]));
    temp.s5 = remquod(x.s5, y.s5, &(((int*)&itemp)[5]));
    temp.s6 = remquod(x.s6, y.s6, &(((int*)&itemp)[6]));
    temp.s7 = remquod(x.s7, y.s7, &(((int*)&itemp)[7]));
    temp.s8 = remquod(x.s8, y.s8, &(((int*)&itemp)[8]));
    temp.s9 = remquod(x.s9, y.s9, &(((int*)&itemp)[9]));
    temp.sa = remquod(x.sa, y.sa, &(((int*)&itemp)[10]));
    temp.sb = remquod(x.sb, y.sb, &(((int*)&itemp)[11]));
    temp.sc = remquod(x.sc, y.sc, &(((int*)&itemp)[12]));
    temp.sd = remquod(x.sd, y.sd, &(((int*)&itemp)[13]));
    temp.se = remquod(x.se, y.se, &(((int*)&itemp)[14]));
    temp.sf = remquod(x.sf, y.sf, &(((int*)&itemp)[15]));
    *ptr = itemp;
    return temp;
}
