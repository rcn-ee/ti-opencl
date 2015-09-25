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
#include <stddef.h>
#include "monitor.h"
#ifdef DEVICE_K2H
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_xmc.h>
#include <ti/csl/csl_xmcAux.h>
#endif
#include <ti/csl/csl_cacheAux.h>
#include "util.h"

extern uint32_t ocl_l1d_mem_start;
extern uint32_t ocl_l1d_mem_size;

far void*     l1d_start = (void*)    &ocl_l1d_mem_start;
far uint32_t l1d_size   = (uint32_t) &ocl_l1d_mem_size;

extern cregister volatile unsigned int DNUM;

/******************************************************************************
* __core_num()
******************************************************************************/
EXPORT int __core_num() { return get_dsp_id(); }

/*-----------------------------------------------------------------------------
 * Variant across DSP cores. Place in nocache msmc to avoid false sharing.
 *----------------------------------------------------------------------------*/
static far PRIVATE(size_t, l1d_scratch_size) = 0;

EXPORT void*  __scratch_l1d_start() { return l1d_start; }
EXPORT size_t __scratch_l1d_size()  { return l1d_scratch_size; }

EXPORT uint32_t __cache_l1d_size()
{
    switch (CACHE_getL1DSize())
    {
        case CACHE_L1_0KCACHE:  return 0;
        case CACHE_L1_4KCACHE:  return (4  << 10);
        case CACHE_L1_8KCACHE:  return (8  << 10);
        case CACHE_L1_16KCACHE: return (16 << 10);
        case CACHE_L1_32KCACHE: return (32 << 10);
        default:                return (32 << 10);
    }
}

EXPORT uint32_t __cache_l2_size()
{
    switch (CACHE_getL2Size())
    {
        case CACHE_0KCACHE:    return 0;
        case CACHE_32KCACHE:   return (32   << 10);
        case CACHE_64KCACHE:   return (64   << 10);
        case CACHE_128KCACHE:  return (128  << 10);
        case CACHE_256KCACHE:  return (256  << 10);
        case CACHE_512KCACHE:  return (512  << 10);
        case CACHE_1024KCACHE: return (1024 << 10);
        default:               return (1024 << 10);
    }
}

PRIVATE(int32_t, _local_id_x)      EXPORT = 0;
PRIVATE(int32_t, _local_id_y)      EXPORT = 0;
PRIVATE(int32_t, _local_id_z)      EXPORT = 0;

EXPORT void __cache_l1d_none()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_0KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size;
}

EXPORT void __cache_l1d_all()
{
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = 0;
}

EXPORT void __cache_l1d_4k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_4KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size - (4 << 10);
}

EXPORT void __cache_l1d_8k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_8KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size - (8 << 10);
}

EXPORT void __cache_l1d_16k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_16KCACHE);
    CACHE_getL1DSize();
    l1d_scratch_size = l1d_size - (16 << 10);
}

EXPORT void __cache_l1d_flush()
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
}


EXPORT void __cache_l2_none()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_0KCACHE);
    CACHE_getL2Size ();
}

EXPORT void __cache_l2_128k()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_128KCACHE);
    CACHE_getL2Size ();
}

EXPORT void __cache_l2_256k()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_256KCACHE);
    CACHE_getL2Size ();
}

EXPORT void __cache_l2_512k()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_512KCACHE);
    CACHE_getL2Size ();
}

EXPORT void __cache_l2_flush()
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
}


#define OCL_SPINLOCK_IDX    1     // used in bulitins/lib/dsp/atomics.cl
#define ADDR_SPINLOCK(IDX)  ((volatile uint32_t *) (0x4A0F6800 + 4 * IDX))
uint32_t acquire_spinlock(int idx)
{
    uint32_t lvInt = _disable_interrupts();
    uint32_t acquired = 1;
    while (acquired != 0)  acquired = (* ADDR_SPINLOCK(idx)) & 0x1;
    return lvInt;
}

void release_spinlock(int idx, uint32_t lvInt)
{
    * ADDR_SPINLOCK(idx) = 0;
    _restore_interrupts(lvInt);
}

EXPORT uint32_t __sem_lock(int idx)
{
    return acquire_spinlock(idx);
}

EXPORT void __sem_unlock(int idx, uint32_t lvInt)
{
    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
    release_spinlock(idx, lvInt);
}

EXPORT void __inv(char*p, int sz)
{
    CACHE_invL2(p, sz, CACHE_NOWAIT);
    CSL_XMC_invalidatePrefetchBuffer();

    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
}
