/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPL
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABL
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICE
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AN
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF T
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "string.h"
#include "util.h"
#include "monitor.h"

#ifdef DEVICE_K2H
#include <ti/csl/csl_msmcAux.h>
#endif

#include <ti/csl/csl_xmcAux.h>
#include <ti/csl/csl_cacheAux.h>

EXPORT void __mfence(void)
{
    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
}

/******************************************************************************
* Clock Handling
******************************************************************************/
extern cregister volatile uint32_t TSCL;
extern cregister volatile uint32_t TSCH;

EXPORT uint32_t __clock(void)
{
    return TSCL;
}

EXPORT uint64_t __clock64(void)
{
    uint32_t low  = TSCL;
    uint32_t high = TSCH;
    return _itoll(high,low);
}

EXPORT void __cycle_delay (uint64_t cyclesToDelay)
{
    uint64_t now = __clock64 ();
    while((__clock64() - now) < cyclesToDelay);
}

/******************************************************************************
* disableCache
******************************************************************************/
void disableCache(unsigned first, unsigned last)
{
    volatile unsigned int *MAR = (volatile unsigned int *)0x1848000;

    /*-------------------------------------------------------------------------
    * Need to WbInv before changing MAR bits
    *------------------------------------------------------------------------*/
    cacheWbInvAllL2();

    int i;
    for (i = first; i <= last; ++i) MAR[i] = 0x0;
}

/******************************************************************************
* enableCache (with prefetching and writethrough on)
******************************************************************************/
void enableCache(unsigned first, unsigned last)
{
    volatile unsigned int *MAR = (volatile unsigned int *)0x1848000;

    /*-------------------------------------------------------------------------
    * Need to WbInv before changing MAR bits
    *------------------------------------------------------------------------*/
    cacheWbInvAllL2();

    /*-------------------------------------------------------------------------
    * Cacheable, Prefetchable, Write through
    *------------------------------------------------------------------------*/
    int i;
    for (i = first; i <= last; ++i) MAR[i] = 0xB;
}

/******************************************************************************
* cacheWbInvAllL2 
*     Write back and invalidate all cache
******************************************************************************/
void cacheWbInvAllL2 ()
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvAllL2(CACHE_NOWAIT);
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
    return;
}

/******************************************************************************
* cacheInvAllL2 
*     Invalidate all cache
******************************************************************************/
void cacheInvAllL2 ()
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvAllL1d(CACHE_NOWAIT); 
    __mfence();
    // THIS NEED TO BE INLINED!!! USE -O* WHEN COMPILING!
    CACHE_invAllL2(CACHE_NOWAIT);
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
    return;
}
/******************************************************************************
* cacheWbInvL2 
*     Write back and invalidate all cache
******************************************************************************/
void cacheWbInvL2 (uint8_t* bufferPtr, uint32_t bufferSize)
{
    uint32_t lvInt = _disable_interrupts();
    CACHE_wbInvL2(bufferPtr, bufferSize, CACHE_NOWAIT);
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
    return;
}

/******************************************************************************
* cacheInvL2 
*     Write back and invalidate all cache
******************************************************************************/
void cacheInvL2 (uint8_t* bufferPtr, uint32_t bufferSize)
{
    uint32_t lvInt = _disable_interrupts();

    CACHE_wbInvL1d(bufferPtr, bufferSize, CACHE_NOWAIT); 
    __mfence();
    CACHE_invL2(bufferPtr, bufferSize, CACHE_NOWAIT);
    CSL_XMC_invalidatePrefetchBuffer();
    __mfence();
    _restore_interrupts(lvInt);
    return;
}


#define DSP_SYS_HWINFO  (0x01D00004)

uint32_t get_dsp_id()
{
    uint32_t id = (*((uint32_t *)DSP_SYS_HWINFO)) & 0xf;
    return id;
}

uint32_t count_trailing_zeros(uint32_t x)
{
    int cnt = 0;

    if (!x) return 32;

    while ((x & 1) == 0)
    {
        x >>= 1;
        cnt ++;
    }
    return cnt;
}

unsigned dsp_speed()
{
    const unsigned DSP_PLL  = 122880000;
    char *BOOTCFG_BASE_ADDR = (char*)0x02620000;
    char *CLOCK_BASE_ADDR   = (char*)0x02310000;
    int MAINPLLCTL0         = (*(int*)(BOOTCFG_BASE_ADDR + 0x350));
    int MULT                = (*(int*)(CLOCK_BASE_ADDR + 0x110));
    int OUTDIV              = (*(int*)(CLOCK_BASE_ADDR + 0x108));

    unsigned mult = 1 + ((MULT & 0x3F) | ((MAINPLLCTL0 & 0x7F000) >> 6));
    unsigned prediv = 1 + (MAINPLLCTL0 & 0x3F);
    unsigned output_div = 1 + ((OUTDIV >> 19) & 0xF);
    unsigned speed = DSP_PLL * mult / prediv / output_div;
    return speed / 1000000;
}
