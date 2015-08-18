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
#ifndef _DSP_C_H_
#define _DSP_C_H_

#ifndef __OPENCL_VERSION__
#define __global
#define __local

#include <stdint.h>
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*-----------------------------------------------------------------------------
* The extended TI built-in function set
*----------------------------------------------------------------------------*/
void     __touch           (const __global char *p, uint32_t size);
uint32_t __core_num        (void);
uint32_t __clock           (void);
uint64_t __clock64         (void);
void     __cycle_delay     (uint64_t cyclesToDelay);
void     __mfence          (void);

void*    __scratch_l1d_start(void);
uint32_t __scratch_l1d_size (void);

void     __cache_l1d_none  (void);
void     __cache_l1d_all   (void);
void     __cache_l1d_4k    (void);
void     __cache_l1d_8k    (void);
void     __cache_l1d_16k   (void);
void     __cache_l1d_flush (void);

void     __cache_l2_none   (void);
void     __cache_l2_128k   (void);
void     __cache_l2_256k   (void);
void     __cache_l2_512k   (void);
void     __cache_l2_flush  (void);

void     __heap_init_ddr  (__global void *ptr, size_t size);
void*    __malloc_ddr     (size_t size);
void*    __calloc_ddr     (size_t num, size_t size);
void*    __realloc_ddr    (void *ptr,  size_t size);
void     __free_ddr       (void *ptr);
void*    __memalign_ddr   (size_t alignment, size_t size);

void     __heap_init_msmc (__global void *ptr, size_t size);
void*    __malloc_msmc    (size_t size);
void*    __calloc_msmc    (size_t num, size_t size);
void*    __realloc_msmc   (void *ptr, size_t size);
void     __free_msmc      (void *ptr);
void*    __memalign_msmc  (size_t alignment, size_t size);

void     __heap_init_l2   (__local void *ptr, size_t size);
void*    __malloc_l2      (size_t size);

#ifndef __OPENCL_VERSION__
#undef __global
#undef __local
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif
