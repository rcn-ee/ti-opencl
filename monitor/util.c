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
#include "ocl_device_defs.h"
#include <ti/csl/csl_xmc.h>
#include <ti/csl/csl_xmcAux.h>
#include <ti/csl/csl_msmc.h>
#include <ti/csl/csl_msmcAux.h>
#include <ti/csl/csl_semAux.h>
#include <ti/csl/cslr_tmr.h>
#include <ti/csl/csl_emif4fAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <stdio.h>

EXPORT void __mfence(void)
{
    ocl_mfence();
}

/******************************************************************************
* Clock Handling
******************************************************************************/
#define MY_HW_TIM_IDX 0
extern cregister volatile uint32_t TSCL;
extern cregister volatile uint32_t TSCH;

typedef struct 
{
    uint32_t globalStartTime;
    uint32_t localStartTime;
} ClockDsc_t;

FAST_SHARED_1D(ClockDsc_t, ClockDscTbl, MAX_NUM_CORES);

int initClockLocal(void)
{
    volatile CSL_TmrRegs* lvTimerRegPtr;

    lvTimerRegPtr = (CSL_TmrRegs *)(CSL_TIMER_0_REGS + 
                    (CSL_TIMER_1_REGS-CSL_TIMER_0_REGS)*MY_HW_TIM_IDX);
    TSCL = 0; // start local timer

    ClockDscTbl[DNUM].localStartTime = TSCL;
    ClockDscTbl[DNUM].globalStartTime = lvTimerRegPtr->CNTLO;

    return RETURN_OK;
}

int initClockGlobal(void)
{
    volatile CSL_TmrRegs* lvTimerRegPtr;

    lvTimerRegPtr = (CSL_TmrRegs *)(CSL_TIMER_0_REGS + 
                    (CSL_TIMER_1_REGS-CSL_TIMER_0_REGS)*MY_HW_TIM_IDX);

    lvTimerRegPtr->TGCR  = 0x00000000; // TIMMODE = 0; TIMHIRS = 0; TIMLORS = 0
    lvTimerRegPtr->PRDLO = (Uint32) 0xffffffff;
    lvTimerRegPtr->PRDHI = (Uint32) 0xffffffff;
    lvTimerRegPtr->CNTLO = 0;
    lvTimerRegPtr->CNTHI = 0;
    lvTimerRegPtr->TCR   = 0x00000080; // ENAMODE_LO=2; CLKSRC_LO=0; TI_EN_LO=0
    lvTimerRegPtr->TGCR  = 0x00000003; // TIMMODE = 0; TIMHIRS = 1; TIMLORS = 1

    return RETURN_OK;
}

uint32_t readClockGlobal(void)
{
    return (TSCL - ClockDscTbl[DNUM].localStartTime + 
            6 * ClockDscTbl[DNUM].globalStartTime);
}

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
* Barrier Implementation
******************************************************************************/
typedef struct 
{
    uint8_t tbl0[MAX_NUM_CORES];
    uint8_t tbl1[MAX_NUM_CORES];
} CoreBarrier_t;

FAST_SHARED(CoreBarrier_t, CoreBarrier);

//uint8_t CoreBarrierSense = 0;
PRIVATE(uint8_t, CoreBarrierSense) = 0;

void waitAtCoreBarrier(void)
{
    volatile uint8_t* lvCoreBarrierPtr;
    volatile uint8_t lvGo = 0;
    int i;

    if (CoreBarrierSense == 0)
    {
        lvCoreBarrierPtr = CoreBarrier.tbl0;
        CoreBarrierSense = 1;
    }
    else
    {
        lvCoreBarrierPtr = CoreBarrier.tbl1;
        CoreBarrierSense = 0;
    }

    for (i = 0; i < n_cores; i++) *(lvCoreBarrierPtr + i) = 0;

    while (!lvGo)
    {
        lvGo = 1;
        *(lvCoreBarrierPtr + DNUM) = 1;
        for (i = 0; i < n_cores; i++) lvGo = lvGo & *(lvCoreBarrierPtr + i);
        *(lvCoreBarrierPtr + DNUM) = 1;
    }
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

/******************************************************************************
* set_MPAX
******************************************************************************/
void set_MPAX(int index, Uint32 bAddr, Uint8 segSize, Uint32 rAddr, Uint8 perm)
{
    CSL_XMC_XMPAXH mpaxh;
    mpaxh.bAddr = bAddr;
    mpaxh.segSize =  segSize;

    CSL_XMC_XMPAXL mpaxl;
    mpaxl.rAddr = rAddr;
    mpaxl.sr = (perm & 0x20) ? 1 : 0;
    mpaxl.sw = (perm & 0x10) ? 1 : 0;
    mpaxl.sx = (perm & 0x08) ? 1 : 0;
    mpaxl.ur = (perm & 0x04) ? 1 : 0;
    mpaxl.uw = (perm & 0x02) ? 1 : 0;
    mpaxl.ux = (perm & 0x01) ? 1 : 0;

    CSL_XMC_setXMPAXH(index, &mpaxh);
    CSL_XMC_setXMPAXL(index, &mpaxl);
}

/******************************************************************************
* set_MSMC_MPAX
******************************************************************************/
void set_MSMC_MPAX(int privId, int index, Uint32 bAddr, Uint8 segSize,
                                          Uint32 rAddr, Uint8 perm)
{
    CSL_MSMC_SESMPAXH lvSesMpaxh;
    lvSesMpaxh.baddr = bAddr; // 32-bit address >> 12
    lvSesMpaxh.segSz = segSize;

    CSL_MSMC_SESMPAXL lvSesMpaxl;
    lvSesMpaxl.raddr = rAddr; // 36-bit address >> 12
    lvSesMpaxl.sr = (perm & 0x20) ? 1 : 0;
    lvSesMpaxl.sw = (perm & 0x10) ? 1 : 0;
    lvSesMpaxl.sx = (perm & 0x08) ? 1 : 0;
    lvSesMpaxl.ur = (perm & 0x04) ? 1 : 0;
    lvSesMpaxl.uw = (perm & 0x02) ? 1 : 0;
    lvSesMpaxl.ux = (perm & 0x01) ? 1 : 0;

    CSL_MSMC_setSESMPAXH(privId, index, &lvSesMpaxh);
    CSL_MSMC_setSESMPAXL(privId, index, &lvSesMpaxl);
}

/******************************************************************************
* reset_MPAX
******************************************************************************/
void reset_MPAX(int index)
{
    CSL_XMC_XMPAXH mpaxh;
    CSL_XMC_XMPAXL mpaxl;

    memset(&mpaxh, 0, sizeof(CSL_XMC_XMPAXH));
    memset(&mpaxl, 0, sizeof(CSL_XMC_XMPAXL));

    CSL_XMC_setXMPAXH(index, &mpaxh);
    CSL_XMC_setXMPAXL(index, &mpaxl);
}

/******************************************************************************
* reset_MSMC_MPAX
******************************************************************************/
void reset_MSMC_MPAX(int privId, int index)
{
    CSL_MSMC_SESMPAXH lvSesMpaxh;
    CSL_MSMC_SESMPAXL lvSesMpaxl;

    memset(&lvSesMpaxh, 0, sizeof(CSL_MSMC_SESMPAXH));
    memset(&lvSesMpaxl, 0, sizeof(CSL_MSMC_SESMPAXL));

    CSL_MSMC_setSESMPAXH(privId, index, &lvSesMpaxh);
    CSL_MSMC_setSESMPAXL(privId, index, &lvSesMpaxl);
}

/******************************************************************************
* set_kernel_MPAX_settings
*   Set MPAX settings in the message for this kernel.
* MPAXH:  20-bit baddr, 7-bit padding, 5-bit segment size
* MPAXL:  24-bit raddr, 8-bit permission (Res, Res, SR, SW, SX, UR, UW, UX)
******************************************************************************/
void set_kernel_MPAXs(int num_mpaxs, uint32_t *settings)
{
    uint32_t lvInt = _disable_interrupts();
    int i;
    for (i = 0; i < num_mpaxs; i++)
    {
        uint32_t bAddr   = settings[2*i+1] >> 12;   // 20 bit
        uint8_t  segSize = settings[2*i+1] & 0x1F;  //  5 bit
        uint32_t rAddr   = settings[2*i]   >> 8;    // 24 bit
        uint8_t  perm    = settings[2*i]   & 0xFF;  //  8 bit
        set_MPAX     (      i+FIRST_FREE_XMC_MPAX, bAddr, segSize, rAddr, perm);
        set_MSMC_MPAX(DNUM, i+FIRST_FREE_SES_MPAX, bAddr, segSize, rAddr, perm);
    }
    _restore_interrupts(lvInt);
}

/******************************************************************************
* reset_kernel_MPAX_settings
*   Reset MPAX settings in the message for this kernel.
******************************************************************************/
void reset_kernel_MPAXs(int num_mpaxs)
{
    uint32_t lvInt = _disable_interrupts();
    int i;
    for (i = 0; i < num_mpaxs; i++)
    {
        reset_MPAX     (      i+FIRST_FREE_XMC_MPAX);
        reset_MSMC_MPAX(DNUM, i+FIRST_FREE_SES_MPAX);
    }
    _restore_interrupts(lvInt);
}


#define GPIO_IN_DATA 0x02320020     // for DSP number 

uint32_t get_dsp_id()
{
    uint32_t dsp_id = ((*(unsigned int*) GPIO_IN_DATA) & 0x6) >> 1; 
    return dsp_id;
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

    unsigned mult       = 1 + ((MULT & 0x3F) | ((MAINPLLCTL0 & 0x7F000) >> 6));
    unsigned prediv     = 1 + (MAINPLLCTL0 & 0x3F);
    unsigned output_div = 1 + ((OUTDIV >> 19) & 0xF);
    float speed = (float)DSP_PLL * mult / prediv / output_div;
    return speed / 1e6;
}

/******************************************************************************
* clear_mpf()
*   Clear Memory Protection Failure registers
******************************************************************************/
void clear_mpf()
{
    CSL_XMC_clearFault();
}


/******************************************************************************
* report_and_clear_mpf()
*   Report Memory Protection Failure: address, status.  Clear afterwards.
******************************************************************************/
void report_and_clear_mpf()
{
    Uint32 addr = CSL_XMC_getFaultAddress();
    if (addr == 0)  return;

    CSL_XMC_MPFSR mpfsr;
    CSL_XMC_getFaultStatus(&mpfsr);
    printf("Illegal MemAccess: 0x%x, local:%d, srswsxuruwux:%d%d%d%d%d%d\n",
           addr, mpfsr.local, mpfsr.sr, mpfsr.sw, mpfsr.sx,
                              mpfsr.ur, mpfsr.uw, mpfsr.ux);
    CSL_XMC_clearFault();
}

uint32_t makeAddressGlobal(uint32_t coreIdx, uint32_t address)
{
    if (address < 0x01000000)
    {
        address = (0x10 + coreIdx) << 24 | address;
    }

    return address;
}


