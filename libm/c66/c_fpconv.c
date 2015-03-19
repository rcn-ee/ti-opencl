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

/* fpconv.c
*  Generic implementation of floating point conversions in C,
*    with respect to rounding modes.
*  Supported conversions:
*    - long(64-bit)/ulong(64-bit) to float
*    - long(64-bit)/ulong(64-bit) to double
*  Supported rounding modes:(in consistence with open_libm/c66/fenv.h)
*    - 0, rte: rounding toward nearest even (default mode)
*    - 1, rtz: rounding toward zero (truncation)
*    - 2, rtp: rounding toward positive infinity
*    - 3, rtn: rounding toward negative infinity
*/

#define FP_TONEAREST  0
#define FP_TOWARDZERO 1
#define FP_UPWARD     2
#define FP_DOWNWARD   3

#if _TMS320C6X
extern unsigned int _norm(int);
#endif

// first set bit in uint, in range of 1 (lsb) to 32 (msb)
static unsigned int find_first_set_uint(unsigned int x)
{
#if _TMS320C6X
  if (x >= 0x80000000)  return 32;
  else                  return 32 - (1 + _norm((int) x));
#else
  unsigned int fsb = 32;  
  unsigned int byte;
  char table[256] = { 0, 1, 2,2, 3,3,3,3, 4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  if      ((byte = x >> 24) != 0) fsb -= (8  - table[byte]);
  else if ((byte = x >> 16) != 0) fsb -= (16 - table[byte]);
  else if ((byte = x >>  8) != 0) fsb -= (24 - table[byte]);
  else if ((byte = x      ) != 0) fsb -= (32 - table[byte]);
  return fsb;
#endif
}

// first set bit in ulong, in range of 1 (lsb) to 64 (msb)
static unsigned int find_first_set_ulong(unsigned long long x)
{
  unsigned int word = (x >> 32);
  if (word != 0)  return 32 + find_first_set_uint(word);
  else            return      find_first_set_uint((unsigned int)x);
}

float fpconv_float_ulong(unsigned long long x, int round)
{
  if (x == 0) return 0.0f;
  unsigned int       fsb   = find_first_set_ulong(x);
  unsigned long long fval  = x & (0xFFFFFF0000000000ULL >> (64 - fsb));
  unsigned long long fhalf =     (0x0000008000000000ULL >> (64 - fsb));
  unsigned long long finc  =     (0x0000010000000000ULL >> (64 - fsb));
  unsigned long long fremain = x - fval;

  // rte(0)/rtp(2) may need to round up, tie-breaking: round half to even
  if ( fremain > 0 &&
       ((round == FP_TONEAREST && fremain > fhalf) ||
        (round == FP_TONEAREST && fremain == fhalf && ((fval & finc) != 0)) ||
        (round == FP_UPWARD)) )
  {
    if (fval == 0xFFFFFF0000000000ULL)  return 0x1p+64;
    fval += finc;
    if ((fval >> fsb) != 0) fsb += 1;
  }

  // mantissa : 23 bits, get rid of leading set/1 bit in fval
  unsigned int fbits = ((fval << (65 - fsb)) >> 41);
  // exponent :  8 bits, biased by 127
  fbits |= ((fsb - 1 + 127) << 23);
  return *((float *) &fbits);
}

float fpconv_float_long(long long x, int round)
{
  if (x == 0x8000000000000000LL)  return -0x1p+63;
  if (x >= 0)  return   fpconv_float_ulong((unsigned long long) x, round);
  else         return - fpconv_float_ulong((unsigned long long) (-x), 
                                  round == FP_UPWARD ? FP_DOWNWARD :
                                  round == FP_DOWNWARD ? FP_UPWARD : round);
}

double fpconv_double_ulong(unsigned long long x, int round)
{
  if (x == 0) return 0.0;
  unsigned int       fsb   = find_first_set_ulong(x);
  unsigned long long fval  = x & (0xFFFFFFFFFFFFF800ULL >> (64 - fsb));
  unsigned long long fhalf =     (0x0000000000000400ULL >> (64 - fsb));
  unsigned long long finc  =     (0x0000000000000800ULL >> (64 - fsb));
  unsigned long long fremain = x - fval;

  // rte(0)/rtp(2) may need to round up, tie-breaking: round half to even
  if ( fremain > 0 &&
       ((round == FP_TONEAREST && fremain > fhalf) ||
        (round == FP_TONEAREST && fremain == fhalf && ((fval & finc) != 0)) ||
        (round == FP_UPWARD)) )
  {
    if (fval == 0xFFFFFFFFFFFFF800ULL)  return 0x1p+64;
    fval += finc;
    if ((fval >> fsb) != 0) fsb += 1;
  }

  // mantissa : 52 bits, get rid of leading set/1 bit in fval
  unsigned long long fbits = ((fval << (65 - fsb)) >> 12);
  // exponent : 11 bits, biased by 1023
  fbits |= (((unsigned long long) (fsb - 1 + 1023)) << 52);
  return *((double *) &fbits);
}

double fpconv_double_long(long long x, int round)
{
  if (x == 0x8000000000000000LL)  return -0x1p+63;
  if (x >= 0)  return   fpconv_double_ulong((unsigned long long) x, round);
  else         return - fpconv_double_ulong((unsigned long long) (-x), 
                                  round == FP_UPWARD ? FP_DOWNWARD :
                                  round == FP_DOWNWARD ? FP_UPWARD : round);
}

