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

#include <ti/csl/csl_xmc.h>
#include <ti/csl/csl_xmcAux.h>
#include <ti/csl/csl_msmc.h>
#include <ti/csl/csl_msmcAux.h>
#include <ti/csl/csl_semAux.h>
#include <ti/csl/cslr_tmr.h>
#include <ti/csl/csl_emif4fAux.h>
#include <ti/csl/csl_cacheAux.h>

#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/******************************************************************************
* initialize_memory
******************************************************************************/
void initialize_memory(void)
{
    int32_t mask = _disable_interrupts();

    enableCache (0x0c, 0x0c); // enable write through for msmc
    enableCache (0x90, 0xff); // enable write through for ddr
    disableCache(0x9f, 0x9f); // disable cache for DDR3_NC and vrings

    _restore_interrupts(mask);
}

/******************************************************************************
* dsp_speed
* See evmk2g.gel SetPll1() for magic numbers
******************************************************************************/
unsigned dsp_speed(void)
{
    const unsigned CLKIN_val  = 24;
    char *BOOTCFG_BASE_ADDR = (char*)0x02620000;
    char *CLOCK_BASE_ADDR   = (char*)0x02310000;
    int MAINPLLCTL0         = (*(int*)(BOOTCFG_BASE_ADDR + 0x350));
    int MULT                = (*(int*)(CLOCK_BASE_ADDR + 0x110));
    int OUTDIV              = (*(int*)(CLOCK_BASE_ADDR + 0x108));

    unsigned mult       = 1 + ((MULT & 0x3F) | ((MAINPLLCTL0 & 0x7F000) >> 6));
    unsigned prediv     = 1 + (MAINPLLCTL0 & 0x3F);
    unsigned output_div = 1 + ((OUTDIV >> 19) & 0xF);
    return CLKIN_val * mult / prediv / output_div;
}

/******************************************************************************
* Atomics
******************************************************************************/

EXPORT uint32_t __sem_lock(int idx)
{
    while (!CSL_semAcquireDirect(idx));
    return 0;
}

EXPORT void __sem_unlock(int idx, uint32_t lvInt)
{
    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
    CSL_semReleaseSemaphore(idx);
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
