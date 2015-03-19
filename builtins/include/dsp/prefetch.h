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
#ifndef _DSP_PREFETCH_
#define _DSP_PREFETCH_

void __touch(const __global char *p, uint32_t size);

#define PREFETCH_VECTORIZE(PRIM_TYPE) \
   _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE    *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##2 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##3 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##4 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##8 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \
    _CLC_OVERLOAD _CLC_INLINE void prefetch(const __global PRIM_TYPE##16 *p, size_t num_gentypes) \
    { __touch((const __global char*)p, (uint32_t)(num_gentypes * sizeof(*p))); } \

#define PREFETCH_TYPES()           \
        PREFETCH_VECTORIZE(char)   \
        PREFETCH_VECTORIZE(uchar)  \
        PREFETCH_VECTORIZE(short)  \
        PREFETCH_VECTORIZE(ushort) \
        PREFETCH_VECTORIZE(int)    \
        PREFETCH_VECTORIZE(uint)   \
        PREFETCH_VECTORIZE(long)   \
        PREFETCH_VECTORIZE(ulong)  \
        PREFETCH_VECTORIZE(float)  \
        PREFETCH_VECTORIZE(double) \

PREFETCH_TYPES()
    
#endif // _DSP_PREFETCH_
