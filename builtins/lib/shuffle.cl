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

#include "clc.h"

#define TEMPLATE2(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##2 shuffle(res_elemt##val_vnum val, mask_elemt##2 mask) \
{ \
    res_elemt##2 result; \
    res_elemt *p = (res_elemt*)&val; \
    result.s0 = p[mask.s0 & vec_step(val)-1]; \
    result.s1 = p[mask.s1 & vec_step(val)-1]; \
    return result; \
}\
_CLC_OVERLOAD _CLC_DEF res_elemt##2 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##2 mask) \
{ \
    res_elemt##2 result; \
    res_elemt *p1 = (res_elemt*)&val1; \
    res_elemt *p2 = (res_elemt*)&val2; \
    result.s0 = mask.s0 & vec_step(val1) ? p2[mask.s0 & vec_step(val1)-1] : \
                                         p1[mask.s0 & vec_step(val1)-1]; \
    result.s1 = mask.s1 & vec_step(val1) ? p2[mask.s1 & vec_step(val1)-1] : \
                                         p1[mask.s1 & vec_step(val1)-1]; \
    return result; \
} 

#define TEMPLATE4(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##4 shuffle(res_elemt##val_vnum val, mask_elemt##4 mask) \
{ \
    res_elemt##4 result; \
    res_elemt *p = (res_elemt*)&val; \
    result.s0 = p[mask.s0 & vec_step(val)-1]; \
    result.s1 = p[mask.s1 & vec_step(val)-1]; \
    result.s2 = p[mask.s2 & vec_step(val)-1]; \
    result.s3 = p[mask.s3 & vec_step(val)-1]; \
    return result; \
} \
_CLC_OVERLOAD _CLC_DEF res_elemt##4 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##4 mask) \
{ \
    res_elemt##4 result; \
    res_elemt *p1= (res_elemt*)&val1; \
    res_elemt *p2 = (res_elemt*)&val2; \
    result.s0 = mask.s0 & vec_step(val1) ? p2[mask.s0 & vec_step(val1)-1] : \
                                         p1[mask.s0 & vec_step(val1)-1]; \
    result.s1 = mask.s1 & vec_step(val1) ? p2[mask.s1 & vec_step(val1)-1] : \
                                         p1[mask.s1 & vec_step(val1)-1]; \
    result.s2 = mask.s2 & vec_step(val1) ? p2[mask.s2 & vec_step(val1)-1] : \
                                         p1[mask.s2 & vec_step(val1)-1]; \
    result.s3 = mask.s3 & vec_step(val1) ? p2[mask.s3 & vec_step(val1)-1] : \
                                         p1[mask.s3 & vec_step(val1)-1]; \
    return result; \
} 

#define TEMPLATE8(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##8 shuffle(res_elemt##val_vnum val, mask_elemt##8 mask) \
{ \
    res_elemt##8 result; \
    res_elemt *p = (res_elemt*)&val; \
    result.s0 = p[mask.s0 & vec_step(val)-1]; \
    result.s1 = p[mask.s1 & vec_step(val)-1]; \
    result.s2 = p[mask.s2 & vec_step(val)-1]; \
    result.s3 = p[mask.s3 & vec_step(val)-1]; \
    result.s4 = p[mask.s4 & vec_step(val)-1]; \
    result.s5 = p[mask.s5 & vec_step(val)-1]; \
    result.s6 = p[mask.s6 & vec_step(val)-1]; \
    result.s7 = p[mask.s7 & vec_step(val)-1]; \
    return result; \
} \
_CLC_OVERLOAD _CLC_DEF res_elemt##8 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##8 mask) \
{ \
    res_elemt##8 result; \
    res_elemt *p1= (res_elemt*)&val1; \
    res_elemt *p2 = (res_elemt*)&val2; \
    result.s0 = mask.s0 & vec_step(val1) ? p2[mask.s0 & vec_step(val1)-1] : \
                                         p1[mask.s0 & vec_step(val1)-1]; \
    result.s1 = mask.s1 & vec_step(val1) ? p2[mask.s1 & vec_step(val1)-1] : \
                                         p1[mask.s1 & vec_step(val1)-1]; \
    result.s2 = mask.s2 & vec_step(val1) ? p2[mask.s2 & vec_step(val1)-1] : \
                                         p1[mask.s2 & vec_step(val1)-1]; \
    result.s3 = mask.s3 & vec_step(val1) ? p2[mask.s3 & vec_step(val1)-1] : \
                                         p1[mask.s3 & vec_step(val1)-1]; \
    result.s4 = mask.s4 & vec_step(val1) ? p2[mask.s4 & vec_step(val1)-1] : \
                                         p1[mask.s4 & vec_step(val1)-1]; \
    result.s5 = mask.s5 & vec_step(val1) ? p2[mask.s5 & vec_step(val1)-1] : \
                                         p1[mask.s5 & vec_step(val1)-1]; \
    result.s6 = mask.s6 & vec_step(val1) ? p2[mask.s6 & vec_step(val1)-1] : \
                                         p1[mask.s6 & vec_step(val1)-1]; \
    result.s7 = mask.s7 & vec_step(val1) ? p2[mask.s7 & vec_step(val1)-1] : \
                                         p1[mask.s7 & vec_step(val1)-1]; \
    return result; \
} 

#define TEMPLATE16(res_elemt, val_vnum, mask_elemt) \
_CLC_OVERLOAD _CLC_DEF res_elemt##16 shuffle(res_elemt##val_vnum val, mask_elemt##16 mask) \
{ \
    res_elemt##16 result; \
    res_elemt *p = (res_elemt*)&val; \
    result.s0 = p[mask.s0 & vec_step(val)-1]; \
    result.s1 = p[mask.s1 & vec_step(val)-1]; \
    result.s2 = p[mask.s2 & vec_step(val)-1]; \
    result.s3 = p[mask.s3 & vec_step(val)-1]; \
    result.s4 = p[mask.s4 & vec_step(val)-1]; \
    result.s5 = p[mask.s5 & vec_step(val)-1]; \
    result.s6 = p[mask.s6 & vec_step(val)-1]; \
    result.s7 = p[mask.s7 & vec_step(val)-1]; \
    result.s8 = p[mask.s8 & vec_step(val)-1]; \
    result.s9 = p[mask.s9 & vec_step(val)-1]; \
    result.sa = p[mask.sa & vec_step(val)-1]; \
    result.sb = p[mask.sb & vec_step(val)-1]; \
    result.sc = p[mask.sc & vec_step(val)-1]; \
    result.sd = p[mask.sd & vec_step(val)-1]; \
    result.se = p[mask.se & vec_step(val)-1]; \
    result.sf = p[mask.sf & vec_step(val)-1]; \
    return result; \
} \
_CLC_OVERLOAD _CLC_DEF res_elemt##16 shuffle2(res_elemt##val_vnum val1, res_elemt##val_vnum val2, mask_elemt##16 mask) \
{ \
    res_elemt##16 result; \
    res_elemt *p1= (res_elemt*)&val1; \
    res_elemt *p2 = (res_elemt*)&val2; \
    result.s0 = mask.s0 & vec_step(val1) ? p2[mask.s0 & vec_step(val1)-1] : \
                                         p1[mask.s0 & vec_step(val1)-1]; \
    result.s1 = mask.s1 & vec_step(val1) ? p2[mask.s1 & vec_step(val1)-1] : \
                                         p1[mask.s1 & vec_step(val1)-1]; \
    result.s2 = mask.s2 & vec_step(val1) ? p2[mask.s2 & vec_step(val1)-1] : \
                                         p1[mask.s2 & vec_step(val1)-1]; \
    result.s3 = mask.s3 & vec_step(val1) ? p2[mask.s3 & vec_step(val1)-1] : \
                                         p1[mask.s3 & vec_step(val1)-1]; \
    result.s4 = mask.s4 & vec_step(val1) ? p2[mask.s4 & vec_step(val1)-1] : \
                                         p1[mask.s4 & vec_step(val1)-1]; \
    result.s5 = mask.s5 & vec_step(val1) ? p2[mask.s5 & vec_step(val1)-1] : \
                                         p1[mask.s5 & vec_step(val1)-1]; \
    result.s6 = mask.s6 & vec_step(val1) ? p2[mask.s6 & vec_step(val1)-1] : \
                                         p1[mask.s6 & vec_step(val1)-1]; \
    result.s7 = mask.s7 & vec_step(val1) ? p2[mask.s7 & vec_step(val1)-1] : \
                                         p1[mask.s7 & vec_step(val1)-1]; \
    result.s8 = mask.s8 & vec_step(val1) ? p2[mask.s8 & vec_step(val1)-1] : \
                                         p1[mask.s8 & vec_step(val1)-1]; \
    result.s9 = mask.s9 & vec_step(val1) ? p2[mask.s9 & vec_step(val1)-1] : \
                                         p1[mask.s9 & vec_step(val1)-1]; \
    result.sa = mask.sa & vec_step(val1) ? p2[mask.sa & vec_step(val1)-1] : \
                                         p1[mask.sa & vec_step(val1)-1]; \
    result.sb = mask.sb & vec_step(val1) ? p2[mask.sb & vec_step(val1)-1] : \
                                         p1[mask.sb & vec_step(val1)-1]; \
    result.sc = mask.sc & vec_step(val1) ? p2[mask.sc & vec_step(val1)-1] : \
                                         p1[mask.sc & vec_step(val1)-1]; \
    result.sd = mask.sd & vec_step(val1) ? p2[mask.sd & vec_step(val1)-1] : \
                                         p1[mask.sd & vec_step(val1)-1]; \
    result.se = mask.se & vec_step(val1) ? p2[mask.se & vec_step(val1)-1] : \
                                         p1[mask.se & vec_step(val1)-1]; \
    result.sf = mask.sf & vec_step(val1) ? p2[mask.sf & vec_step(val1)-1] : \
                                         p1[mask.sf & vec_step(val1)-1]; \
    return result; \
} 


#define CROSS_SIZE(type1, type2) \
TEMPLATE2(type1, 2, type2) \
TEMPLATE2(type1, 4, type2) \
TEMPLATE2(type1, 8, type2) \
TEMPLATE2(type1, 16, type2) \
TEMPLATE4(type1, 2, type2) \
TEMPLATE4(type1, 4, type2) \
TEMPLATE4(type1, 8, type2) \
TEMPLATE4(type1, 16, type2) \
TEMPLATE8(type1, 2, type2) \
TEMPLATE8(type1, 4, type2) \
TEMPLATE8(type1, 8, type2) \
TEMPLATE8(type1, 16, type2) \
TEMPLATE16(type1, 2, type2) \
TEMPLATE16(type1, 4, type2) \
TEMPLATE16(type1, 8, type2) \
TEMPLATE16(type1, 16, type2) \

#define CROSS_MASKTYPE(type) \
CROSS_SIZE(type, uchar) \
CROSS_SIZE(type, ushort) \
CROSS_SIZE(type, uint) \
CROSS_SIZE(type, ulong) \

CROSS_MASKTYPE(char)
CROSS_MASKTYPE(uchar)
CROSS_MASKTYPE(short)
CROSS_MASKTYPE(ushort)
CROSS_MASKTYPE(int)
CROSS_MASKTYPE(uint)
CROSS_MASKTYPE(long)
CROSS_MASKTYPE(ulong)
CROSS_MASKTYPE(float)
CROSS_MASKTYPE(double)
