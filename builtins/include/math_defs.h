/*
 * Copyright (c) 2011, Denis Steckelmacher <steckdenis@yahoo.fr>
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _MATH_DEFS_H_
#define _MATH_DEFS_H_

#define CHAR_BIT        8
#define CHAR_MAX        SCHAR_MAX
#define CHAR_MIN        SCHAR_MIN
#define INT_MAX         2147483647
#define INT_MIN         (-2147483647 - 1)
#define LONG_MAX        0x7fffffffffffffffL
#define LONG_MIN        (-0x7fffffffffffffffL - 1)
#define SCHAR_MAX       127
#define SCHAR_MIN       (-127 - 1)
#define SHRT_MAX        32767
#define SHRT_MIN        (-32767 - 1)
#define UCHAR_MAX       255
#define USHRT_MAX       65535
#define UINT_MAX        0xffffffff
#define ULONG_MAX       0xffffffffffffffffUL

#define FLT_DIG         6
#define FLT_MANT_DIG    24
#define FLT_MAX_10_EXP  +38
#define FLT_MAX_EXP     +128
#define FLT_MIN_10_EXP  -37
#define FLT_MIN_EXP     -125
#define FLT_RADIX       2
#define FLT_MAX         0x1.fffffep127f
#define FLT_MIN         0x1.0p-126f
#define FLT_EPSILON     0x1.0p-23f

#define DBL_DIG         15
#define DBL_MANT_DIG    53
#define DBL_MAX_10_EXP  +308
#define DBL_MAX_EXP     +1024
#define DBL_MIN_10_EXP  -307
#define DBL_MIN_EXP     -1021
#define DBL_RADIX       2
#define DBL_MAX         0x1.fffffffffffffp1023
#define DBL_MIN         0x1.0p-1022
#define DBL_EPSILON     0x1.0p-52

#define M_E             2.7182818284590452354   /* e */
#define M_LOG2E         1.4426950408889634074   /* log_2 e */
#define M_LOG10E        0.43429448190325182765  /* log_10 e */
#define M_LN2           0.69314718055994530942  /* log_e 2 */
#define M_LN10          2.30258509299404568402  /* log_e 10 */
#define M_PI            3.14159265358979323846  /* pi */
#define M_PI_2          1.57079632679489661923  /* pi/2 */
#define M_PI_4          0.78539816339744830962  /* pi/4 */
#define M_1_PI          0.31830988618379067154  /* 1/pi */
#define M_2_PI          0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI      1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2         1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2       0.70710678118654752440  /* 1/sqrt(2) */

#define M_E_F           M_E
#define M_LOG2E_F       M_LOG2E
#define M_LOG10E_F      M_LOG10E
#define M_LN2_F         M_LN2
#define M_LN10_F        M_LN10
#define M_PI_F          M_PI
#define M_PI_2_F        M_PI_2
#define M_PI_4_F        M_PI_4
#define M_1_PI_F        M_1_PI
#define M_2_PI_F        M_2_PI
#define M_2_SQRTPI_F    M_2_SQRTPI
#define M_SQRT2_F       M_SQRT2
#define M_SQRT1_2_F     M_SQRT1_2

#define MAXFLOAT        FLT_MAX
#define HUGE_VALF       __builtin_huge_valf()
#define INFINITY        __builtin_inff()
#define NAN             __builtin_nanf("")

#define HUGE_VAL        __builtin_huge_val()

#define isinf(x)        __builtin_isinf(x)
#define isnormal(x)     __builtin_isnormal(x)

#endif //_MATH_DEFS_H_
