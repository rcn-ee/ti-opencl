/******************************************************************************
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
#include "util.h"

#include "message.h"

#include <ti/csl/csl_cacheAux.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/******************************************************************************
* initialize_memory
******************************************************************************/
void initialize_memory(void)
{
    extern uint32_t nocache_virt_start;
    extern uint32_t nocache_size;
    extern uint32_t nocache2_virt_start;
    extern uint32_t nocache2_size;

#ifdef _SYS_BIOS   // YUAN TODO: where is this place?
    uint32_t nc_virt = (uint32_t) 0x8E000000;
    uint32_t nc_size = (uint32_t) 0x01000000;
#else
    uint32_t nc_virt = (uint32_t) &nocache_virt_start;
    uint32_t nc_size = (uint32_t) &nocache_size;
    uint32_t nc2_virt = (uint32_t) &nocache2_virt_start;
    uint32_t nc2_size = (uint32_t) &nocache2_size;
#endif

    int32_t mask = _disable_interrupts();

    /***  BIOS is configuring the default cache sizes, see Platform.xdc ***/

    enableCache (0x40, 0x40); // enable write through for OCMC
    enableCache (0x80, 0xFF);
    disableCache(nc_virt >> 24, (nc_virt+nc_size-1) >> 24);
#ifndef _SYS_BIOS
    disableCache(nc2_virt >> 24, (nc2_virt+nc2_size-1) >> 24);
#endif

    _restore_interrupts(mask);

    return;
}

unsigned dsp_speed(void)
{
    /*-------------------------------------------------------------------------
     * F_dpll = F_ref * 2 * M / (N + 1)
     *-----------------------------------------------------------------------*/
    const unsigned AM57_DSP_PLL     = 10*1000*1000;  // 10 MHz
    unsigned CM_CLKSEL_DPLL_DSP_val = *((unsigned*) 0x4A005240);

    unsigned M   = ((CM_CLKSEL_DPLL_DSP_val & 0x7FF00) >> 8);
    unsigned N   =  (CM_CLKSEL_DPLL_DSP_val & 0x7F);
    float  speed = (float)AM57_DSP_PLL * 2 * M / (N + 1);
    return speed / 1e6;
}


/*
 * Sets the MultiProc core Id. Called via Startup from monitor.cfg  
 */
void ocl_set_multiproc_id()
{
    uint32_t dsp_id = DNUM;

    if      (dsp_id == 0) MultiProc_setLocalId(MultiProc_getId("DSP1"));
    else if (dsp_id == 1) MultiProc_setLocalId(MultiProc_getId("DSP2"));
    else    assert(0);
}

/******************************************************************************
* Atomics
******************************************************************************/

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
