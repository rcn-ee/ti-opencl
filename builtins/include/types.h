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
#ifndef _TYPES_H_
#define _TYPES_H_

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

/*-----------------------------------------------------------------------------
* Standard types from Clang's stddef and stdint, Copyright (C) 2008 Eli Friedman
*----------------------------------------------------------------------------*/
typedef signed   __INT64_TYPE__ int64_t;
typedef unsigned __INT64_TYPE__ uint64_t;
typedef signed   __INT32_TYPE__ int32_t;
typedef unsigned __INT32_TYPE__ uint32_t;
typedef signed   __INT16_TYPE__ int16_t;
typedef unsigned __INT16_TYPE__ uint16_t;
typedef signed   __INT8_TYPE__  int8_t;
typedef unsigned __INT8_TYPE__  uint8_t;

#define __stdint_join3(a,b,c) a ## b ## c
#define  __intn_t(n) __stdint_join3( int, n, _t)
#define __uintn_t(n) __stdint_join3(uint, n, _t)

typedef __typeof__(((int*)0)-((int*)0)) ptrdiff_t;
typedef __typeof__(sizeof(int))         size_t;
typedef  __intn_t(__INTPTR_WIDTH__)     intptr_t;
typedef __uintn_t(__INTPTR_WIDTH__)     uintptr_t;

#undef __stdint_join3
#undef  __intn_t
#undef __uintn_t

/*-----------------------------------------------------------------------------
* OpenCL types 
*----------------------------------------------------------------------------*/
typedef uint8_t  uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

#if defined(CLANG_OLDER_THAN_3_3)
typedef unsigned int    sampler_t;
typedef struct image2d *image2d_t;
typedef struct image3d *image3d_t;
#endif

/*-----------------------------------------------------------------------------
* Vectors
*----------------------------------------------------------------------------*/
#define COAL_VECTOR(type, len)                                  \
   typedef type type##len __attribute__((ext_vector_type(len)))

#define COAL_VECTOR_SET(type) \
   COAL_VECTOR(type, 2);      \
   COAL_VECTOR(type, 3);      \
   COAL_VECTOR(type, 4);      \
   COAL_VECTOR(type, 8);      \
   COAL_VECTOR(type, 16);

COAL_VECTOR_SET(char)
COAL_VECTOR_SET(uchar)
COAL_VECTOR_SET(short)
COAL_VECTOR_SET(ushort)
COAL_VECTOR_SET(int)
COAL_VECTOR_SET(uint)
COAL_VECTOR_SET(long)
COAL_VECTOR_SET(ulong)
COAL_VECTOR_SET(float)
COAL_VECTOR_SET(double)

#undef COAL_VECTOR_SET
#undef COAL_VECTOR

#endif  //_TYPES_H_
