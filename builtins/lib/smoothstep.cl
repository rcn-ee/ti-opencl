/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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

_CLC_OVERLOAD _CLC_DEF float smoothstep(float edge0, float edge1, float x)
{ 
    float t = clamp((float)((x-edge0)/(edge1-edge0)), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f*t); 
}

_CLC_OVERLOAD _CLC_DEF double smoothstep(double edge0, double edge1, double x)
{ 
    double t = clamp((double)((x-edge0)/(edge1-edge0)), 0.0, 1.0);
    return t * t * (3.0 - 2.0*t); 
}

#define FLOAT_TEMPLATE(N) \
_CLC_OVERLOAD _CLC_DEF float##N smoothstep(float##N edge0, float##N edge1, float##N x) \
{\
    float##N t = clamp((x-edge0)/(edge1-edge0), 0.0f, 1.0f); \
    return t*t*(3.0f - 2.0f * t); \
}\
_CLC_OVERLOAD _CLC_DEF float##N smoothstep(float edge0, float edge1, float##N x) \
{\
    float##N t = clamp((x-edge0)/(edge1-edge0), 0.0f, 1.0f); \
    return t*t*(3.0f - 2.0f * t);\
}\


#define DOUBLE_TEMPLATE(N) \
_CLC_OVERLOAD _CLC_DEF double##N smoothstep(double##N edge0, double##N edge1, double##N x) \
{\
    double##N t = clamp((x-edge0)/(edge1-edge0), 0.0, 1.0); \
    return t*t*(3.0 - 2.0 * t);\
}\
_CLC_OVERLOAD _CLC_DEF double##N smoothstep(double edge0, double edge1, double##N x) \
{\
    double##N t = clamp((x-edge0)/(edge1-edge0), 0.0, 1.0); \
    return t*t*(3.0 - 2.0 * t);\
}

FLOAT_TEMPLATE(2) 
FLOAT_TEMPLATE(3) 
FLOAT_TEMPLATE(4) 
FLOAT_TEMPLATE(8) 
FLOAT_TEMPLATE(16) 

DOUBLE_TEMPLATE(2) 
DOUBLE_TEMPLATE(3) 
DOUBLE_TEMPLATE(4) 
DOUBLE_TEMPLATE(8) 
DOUBLE_TEMPLATE(16) 
