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
#include "monitor.h"
#ifdef DEVICE_K2H
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_xmc.h>
#include <ti/csl/csl_xmcAux.h>
#endif
#include <ti/csl/csl_cacheAux.h>
#include "util.h"

extern cregister volatile unsigned int DNUM;

/******************************************************************************
* __core_num()
******************************************************************************/
EXPORT int __core_num() { return DNUM; }

PRIVATE(int32_t, _local_id_x)      EXPORT = 0;
PRIVATE(int32_t, _local_id_y)      EXPORT = 0;
PRIVATE(int32_t, _local_id_z)      EXPORT = 0;

EXPORT void __cache_l1d_none()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_0KCACHE);
}

EXPORT void __cache_l1d_all()
{
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
}

EXPORT void __cache_l1d_4k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_4KCACHE);
}

EXPORT void __cache_l1d_8k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_8KCACHE);
}

EXPORT void __cache_l1d_16k()
{
    CACHE_wbInvAllL1d(CACHE_NOWAIT);
    __mfence();
    CACHE_setL1DSize(CACHE_L1_16KCACHE);
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
}

EXPORT void __cache_l2_128k()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_128KCACHE);
}

EXPORT void __cache_l2_256k()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_256KCACHE);
}

EXPORT void __cache_l2_512k()
{
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    __mfence();
    CACHE_setL2Size (CACHE_512KCACHE);
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
