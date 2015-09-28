/******************************************************************************
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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

_CLC_OVERLOAD _CLC_DEF float  length(float2 p)
{
    float r;
    p = fabs(p);
    if (p.x > p.y)
    {
        r = p.y/p.x;
        return p.x * sqrt(1+r*r);
    }
    else if (p.y != 0)
    {
        r = p.x/p.y;
        return p.y * sqrt(1+r*r);
    }
    return 0.0;
}

_CLC_OVERLOAD _CLC_DEF double  length(double2 p)
{
    double r;
    p = fabs(p);
    if (p.x > p.y)
    {
        r = p.y/p.x;
        return p.x * sqrt(1+r*r);
    }
    else if (p.y != 0)
    {
        r = p.x/p.y;
        return p.y * sqrt(1+r*r);
    }
    return 0.0;
}

_CLC_OVERLOAD _CLC_DEF float  length(float3 p)
{
    p = fabs(p);
    float max_term = max(p.x, max(p.y, p.z));
    if (max_term == 0 || isinf(max_term) ) return max_term;
    if (max_term < 1) return fast_length(p);
    p /= max_term;
    return max_term * sqrt((double) dot(p,p));  // improve precision in sqrt
}

_CLC_OVERLOAD _CLC_DEF double  length(double3 p)
{
    p = fabs(p);
    double max_term = max(p.x, max(p.y, p.z));
    if (max_term == 0 || isinf(max_term) ) return max_term;
    if (max_term < 1) return fast_length(p);
    p /= max_term;
    return max_term * sqrt(dot(p,p));
}

_CLC_OVERLOAD _CLC_DEF float  length(float4 p)
{
    p = fabs(p);
    float max_term = max(max(p.x, p.y), max(p.z, p.w));
    if (max_term == 0 || isinf(max_term) ) return max_term;
    if (max_term < 1) return fast_length(p);
    p /= max_term;
    return max_term * sqrt((double) dot(p,p));  // improve precision in sqrt
}

_CLC_OVERLOAD _CLC_DEF double  length(double4 p)
{
    p = fabs(p);
    double max_term = max(max(p.x, p.y), max(p.z, p.w));
    if (max_term == 0 || isinf(max_term) ) return max_term;
    if (max_term < 1) return fast_length(p);
    p /= max_term;
    return max_term * sqrt(dot(p,p));
}

_CLC_OVERLOAD _CLC_DEF float  fast_length(float2 p)  { return sqrt(dot(p,p));}
_CLC_OVERLOAD _CLC_DEF float  fast_length(float3 p)  { return sqrt(dot(p,p));}
_CLC_OVERLOAD _CLC_DEF float  fast_length(float4 p)  { return sqrt(dot(p,p));}
_CLC_OVERLOAD _CLC_DEF double fast_length(double2 p) { return sqrt(dot(p,p));}
_CLC_OVERLOAD _CLC_DEF double fast_length(double3 p) { return sqrt(dot(p,p));}
_CLC_OVERLOAD _CLC_DEF double fast_length(double4 p) { return sqrt(dot(p,p));}
