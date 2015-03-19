/******************************************************************************
 * Copyright (c) 2012-2013 Christian Rau <rauy@users.sourceforge.net>
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*******************************************************************************/
#include "clc.h"

static constant uint16_t base_table[512] = { 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 
 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 
 0x0200, 0x0400, 0x0800, 0x0C00, 0x1000, 0x1400, 0x1800, 0x1C00, 
 0x2000, 0x2400, 0x2800, 0x2C00, 0x3000, 0x3400, 0x3800, 0x3C00, 
 0x4000, 0x4400, 0x4800, 0x4C00, 0x5000, 0x5400, 0x5800, 0x5C00, 
 0x6000, 0x6400, 0x6800, 0x6C00, 0x7000, 0x7400, 0x7800, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 
 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8001, 
 0x8002, 0x8004, 0x8008, 0x8010, 0x8020, 0x8040, 0x8080, 0x8100, 
 0x8200, 0x8400, 0x8800, 0x8C00, 0x9000, 0x9400, 0x9800, 0x9C00, 
 0xA000, 0xA400, 0xA800, 0xAC00, 0xB000, 0xB400, 0xB800, 0xBC00, 
 0xC000, 0xC400, 0xC800, 0xCC00, 0xD000, 0xD400, 0xD800, 0xDC00, 
 0xE000, 0xE400, 0xE800, 0xEC00, 0xF000, 0xF400, 0xF800, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 
 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00, 0xFC00 };

static constant unsigned char shift_table[512] = { 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 13, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 
 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 13 };


#define SGNEXP (bits >> 23)  // Sign and exponent bits
#define MANT_SHIFT (shift_table[SGNEXP])
#define MANTISSA (bits & 0x7FFFFF)

static uint16_t __ocl_float_to_half(float fval)
{
   uint32_t bits = *(uint32_t *)&fval;
   uint16_t hbits = base_table[SGNEXP] + (uint16_t)(MANTISSA >> MANT_SHIFT);

   // Make Nan's non signaling
   if (fval != fval) hbits |= 0x0200;

   return hbits;
}

// Round to nearest (tiebreak to even)
static uint16_t __ocl_float_to_half_rte(float fval) 
{
   uint32_t bits = *(uint32_t *)&fval;
   uint16_t hbits = __ocl_float_to_half(fval);
   uint32_t exp = SGNEXP & 0xFF;

   hbits += ((MANTISSA >> (MANT_SHIFT - 1)) | (exp == 102)) & 
            ((hbits & 0x7C00) != 0x7C00) & 
            ((((((uint32_t)(1) << (MANT_SHIFT - 1))- 1) & bits) != 0) | hbits);

   return hbits;
}

// Round to positive infinity
static uint16_t __ocl_float_to_half_rtp(float fval) 
{
   uint32_t bits = *(uint32_t *)&fval;
   uint16_t hbits = __ocl_float_to_half(fval);

   // Expression below includes an awkward subexpression that equates to
   // 'hbits == 0xFC00'. This to workaround a C6x translation tool issue.
   hbits += ((((MANTISSA & (((uint32_t)(1) << (MANT_SHIFT)) - 1)) != 0) | 
	      ((SGNEXP <= 102) & (SGNEXP != 0)) ) & (hbits < 0x7C00)) - 
      (((hbits & 0x8000)!= 0) & ((hbits & 0x7C00) == 0x7C00) & (SGNEXP != 511));

   return hbits;
}

// Round to negative infinity
static uint16_t __ocl_float_to_half_rtn(float f) 
{
   uint32_t bits = *(uint32_t *)&f;
   uint16_t hbits = __ocl_float_to_half(f);

   hbits += ( (((MANTISSA & (((uint32_t)(1) << (MANT_SHIFT)) - 1)) != 0) |
	     ((SGNEXP <= 358) & (SGNEXP != 256))) & 
	     (hbits < 0xFC00) & (hbits >> 15)) - 
             ((hbits == 0x7C00) & ((bits >> 23) != 255));

   return hbits;
}

// Round to zero
static uint16_t __ocl_float_to_half_rtz(float fval) 
{
   uint32_t bits = *(uint32_t *)&fval;
   uint16_t hbits = __ocl_float_to_half(fval);

   hbits -= ((hbits & 0x7FFF) == 0x7C00) & ~MANT_SHIFT;

   return hbits;
}

// Convert 16-bit half float to 32-bit float value
static float __ocl_half_to_float(uint16_t hfval)
{
   union{ uint32_t u; float f;} bits;

   // Position sign
   bits.u = ((uint32_t)hfval & 0x8000) << 16;

   // Extract and position mantissa 
   uint32_t mantissa = ((uint32_t)hfval & 0x03ff) << 13;

   // Extract exponent to check for subnormals/Inf/NaN
   uint32_t  exponent = ((uint32_t)hfval & 0x7c00) >> 10;     
   
   // If value is Inf/NaN then map to corresponding float Inf/Nan
   if( exponent == 31)
   {
      exponent = 0xFF;
      bits.u |= exponent << 23 | mantissa | (mantissa ? 1 << 22 : 0);
      return bits.f;
   }

   // Check for subnormals which can be represented as normalized float
   if( exponent == 0 )
   {
      if( mantissa == 0 ) return bits.f;
      
      int shiftval;
      for (shiftval = -1; (mantissa & 0x800000) == 0; shiftval++)
	 mantissa <<= 1;
      mantissa &= 0x007fffff;
      exponent -= shiftval;
   }
    
   // Re-bias exponent for float
   exponent += 112;
    
   bits.u |= exponent << 23 | mantissa;
    
   return bits.f;
}


#define DEF_VLOAD_HALF_VECTORIZE(SPACE) \
_CLC_OVERLOAD _CLC_DEF float vload_half(size_t offset, const SPACE half *p) { \
   return __ocl_half_to_float(*(SPACE unsigned short *)(p + offset));	      \
} \
_CLC_OVERLOAD _CLC_DEF float2 vload_half2(size_t offset, const SPACE half *p) { \
  return (float2)(vload_half(offset*2, p), \
                  vload_half(offset*2 + 1, p)); \
} \
_CLC_OVERLOAD _CLC_DEF float3 vload_half3(size_t offset, const SPACE half *p) { \
  return (float3)(vload_half(offset*3, p), \
                  vload_half(offset*3 + 1, p), \
                  vload_half(offset*3 + 2, p)); \
} \
_CLC_OVERLOAD _CLC_DEF float3 vloada_half3(size_t offset, const SPACE half *p) { \
  return (float3)(vload_half(offset*4, p), \
                  vload_half(offset*4 + 1, p), \
                  vload_half(offset*4 + 2, p)); \
} \
_CLC_OVERLOAD _CLC_DEF float4 vload_half4(size_t offset, const SPACE half *p) { \
  return (float4)(vload_half2(offset*2, p), \
                  vload_half2(offset*2 + 1, p)); \
} \
_CLC_OVERLOAD _CLC_DEF float8 vload_half8(size_t offset, const SPACE half *p) { \
  return (float8)(vload_half4(offset*2, p), \
                  vload_half4(offset*2 + 1, p)); \
} \
_CLC_OVERLOAD _CLC_DEF float16 vload_half16(size_t offset, const SPACE half *p) { \
  return (float16)(vload_half8(offset*2, p), \
                   vload_half8(offset*2 + 1, p)); \
}

DEF_VLOAD_HALF_VECTORIZE(__global)
DEF_VLOAD_HALF_VECTORIZE(__local)
DEF_VLOAD_HALF_VECTORIZE(__constant)
DEF_VLOAD_HALF_VECTORIZE(__private)

#undef DEF_VLOAD_HALF_VECTORIZE

#define DEF_VSTORE_HALF_SPACE_ROUND(SPACE, ROUND, FUNC) \
_CLC_OVERLOAD _CLC_DEF void vstore_half##ROUND(float data, size_t offset, SPACE half *p) { \
  *(SPACE short *)(p + offset) = FUNC(data); \
} \
_CLC_OVERLOAD _CLC_DEF void vstorea_half##ROUND(float data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data, offset, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstore_half2##ROUND(float2 data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data.lo, offset*2, p); \
  vstore_half##ROUND(data.hi, offset*2 + 1, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstorea_half2##ROUND(float2 data, size_t offset, SPACE half *p) { \
  vstore_half2##ROUND(data, offset, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstore_half3##ROUND(float3 data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data.s0, offset*3, p); \
  vstore_half##ROUND(data.s1, offset*3 + 1, p); \
  vstore_half##ROUND(data.s2, offset*3 + 2, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstorea_half3##ROUND(float3 data, size_t offset, SPACE half *p) { \
  vstore_half##ROUND(data.s0, offset*4, p); \
  vstore_half##ROUND(data.s1, offset*4 + 1, p); \
  vstore_half##ROUND(data.s2, offset*4 + 2, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstore_half4##ROUND(float4 data, size_t offset, SPACE half *p) { \
  vstore_half2##ROUND(data.lo, offset*2, p); \
  vstore_half2##ROUND(data.hi, offset*2 + 1, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstorea_half4##ROUND(float4 data, size_t offset, SPACE half *p) { \
  vstore_half4##ROUND(data, offset, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstore_half8##ROUND(float8 data, size_t offset, SPACE half *p) { \
  vstore_half4##ROUND(data.lo, offset*2, p); \
  vstore_half4##ROUND(data.hi, offset*2 + 1, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstorea_half8##ROUND(float8 data, size_t offset, SPACE half *p) { \
  vstore_half8##ROUND(data, offset, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstore_half16##ROUND(float16 data, size_t offset, SPACE half *p) { \
  vstore_half8##ROUND(data.lo, offset*2, p); \
  vstore_half8##ROUND(data.hi, offset*2 + 1, p); \
} \
_CLC_OVERLOAD _CLC_DEF void vstorea_half16##ROUND(float16 data, size_t offset, SPACE half *p) { \
  vstore_half16##ROUND(data, offset, p); \
}

#define DEF_VSTORE_HALF_SPACE(SPACE) \
  DEF_VSTORE_HALF_SPACE_ROUND(SPACE,     , __ocl_float_to_half_rte) \
  DEF_VSTORE_HALF_SPACE_ROUND(SPACE, _rte, __ocl_float_to_half_rte) \
  DEF_VSTORE_HALF_SPACE_ROUND(SPACE, _rtz, __ocl_float_to_half_rtz) \
  DEF_VSTORE_HALF_SPACE_ROUND(SPACE, _rtp, __ocl_float_to_half_rtp) \
  DEF_VSTORE_HALF_SPACE_ROUND(SPACE, _rtn, __ocl_float_to_half_rtn) 

DEF_VSTORE_HALF_SPACE(__global)
DEF_VSTORE_HALF_SPACE(__local)
DEF_VSTORE_HALF_SPACE(__private)

#undef DEF_VSTORE_HALF_SPACE
#undef DEF_VSTORE_HALF_SPACE_ROUND

