/*
 *
 * Copyright (C) 2012-2014 Texas Instruments Incorporated - http://www.ti.com/
 *
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <platform.h>

#include <ti/csl/csl_xmcAux.h>
#include <ti/csl/csl_cpsgmiiAux.h>
#include <ti/csl/cslr_cpsgmii.h>
#include <ti/csl/csl_cpsw.h>
#include <ti/csl/csl_cacheAux.h>

#include "sdk/inc/initcfg.h"

#define BOOT_MAGIC_ADDR               0x87FFFC    // boot add - end of L2 
#define BOOT_MAGIC_CONTENTS           (*((volatile unsigned int *)BOOT_MAGIC_ADDR))
#define DEF_INIT_CONFIG_UART_BAUDRATE 115200
#define BOOTROM_RESET_VECTOR_LOCATION 0x20b0fc00
#define DEVICE_INTCTL_BASE            0x1800000

/* The mux registers are indexed from 1, not 0 */
#define INTCTL_REG_MUX(x)   (0x104 + (4*((x)-1)))
#define DEVICE_REG32_W(x,y)   *(volatile uint32_t *)(x)=(y)
#define DEVICE_REG32_R(x)    (*(volatile uint32_t *)(x))

/****************************************************************************
 * Generic bit extraction macros
 ****************************************************************************/
#define BOOTBITMASK(x,y)  (((((uint32_t)1 << \
    (((uint32_t)x)-((uint32_t)y)+(uint32_t)1)) - (uint32_t)1)) << ((uint32_t)y))

#define BOOT_READ_BITFIELD(z,x,y)   (((uint32_t)z) & BOOTBITMASK(x,y)) >> (y)
#define BOOT_SET_BITFIELD(z,f,x,y)  (((uint32_t)z) & ~BOOTBITMASK(x,y)) | \
                                    ((((uint32_t)f) << (y)) & BOOTBITMASK(x,y))

#define DEVICE_INT_IPC     91
#define DEVICE_CACHE_BASE  0x1840000
#define CACHE_REG_L2WBIV   0x5004
#define CACHE_REG_L1PINV   0x5028
#define CACHE_REG_L1DWBIV  0x5044

extern volatile unsigned int cregister ICR;
extern volatile unsigned int cregister IFR;
extern volatile unsigned int cregister ISTP;
extern volatile unsigned int cregister CSR;
extern volatile unsigned int cregister IER;
extern volatile unsigned int cregister DNUM;
extern volatile unsigned int cregister FADCR;
extern volatile unsigned int cregister FMCR;

#define TI667X_IRQ_EOI		0x21800050 // End of Interrupt Register
#define LEGACY_A_IRQ_STATUS_RAW	0x21800180 // Raw Interrupt Status Register
#define LEGACY_A_IRQ_STATUS	0x21800184 // Interrupt Enabled Status Register

#define IPCGR(x)            (0x02620240 + x*4)
#define IPCAR(x)            (0x02620280 + x*4)

/* Boot time init configuration */
#pragma DATA_SECTION(init_config,".init_config");
dsp_init_cfg_t init_config;

/** ============================================================================
 *   @n@b Init_Switch
 *
 *   @b Description
 *   @n This API sets up the ethernet switch subsystem and its Address Lookup
 *      Engine (ALE) in "Switch" mode.
 *
 *   @param[in]
 *   @n mtu             Maximum Frame length to configure on the switch.
 *
 *   @return
 *   @n None
 * =============================================================================
 */
void Init_Switch (uint32_t mtu)
{
    CSL_CPSW_3GF_PORTSTAT               portStatCfg;
    uint32_t  rx_max_len = mtu + 14 + 4; /* 4 bytes of FCS */
    CSL_CPSW_3GF_ALE_PORTCONTROL        alePortControlCfg;

    /* Enable the CPPI port, i.e., port 0 that does all
     * the data streaming in/out of EMAC.
     */
    CSL_CPSW_3GF_enablePort0 ();
    CSL_CPSW_3GF_disableVlanAware ();
    CSL_CPSW_3GF_setPort0VlanReg (0, 0, 0);
    CSL_CPSW_3GF_setPort0RxMaxLen (rx_max_len);

    /* Enable statistics on both the port groups:
     *
     * MAC Sliver ports -   Port 1, Port 2
     * CPPI Port        -   Port 0
     */
    portStatCfg.p0AStatEnable   =   1;
    portStatCfg.p0BStatEnable   =   1;
    portStatCfg.p1StatEnable    =   1;
    portStatCfg.p2StatEnable    =   1;
    CSL_CPSW_3GF_setPortStatsEnableReg (&portStatCfg);

    /* Setup the Address Lookup Engine (ALE) Configuration:
     *      (1) Enable ALE.
     *      (2) Clear stale ALE entries.
     *      (3) Disable VLAN Aware lookups in ALE since
     *          we are not using VLANs by default.
     *      (4) No Flow control
     *      (5) Configure the Unknown VLAN processing
     *          properties for the switch, i.e., which
     *          ports to send the packets to.
     */
    CSL_CPSW_3GF_enableAle ();
    CSL_CPSW_3GF_clearAleTable ();
    CSL_CPSW_3GF_disableAleVlanAware ();
    CSL_CPSW_3GF_disableAleTxRateLimit ();

    /* Setting the Switch MTU Size to more than needed */
    CSL_CPGMAC_SL_setRxMaxLen(0, rx_max_len);
    CSL_CPGMAC_SL_setRxMaxLen(1, rx_max_len);


    /* Configure the address in "Learning"/"Forward" state */
    alePortControlCfg.portState             =   ALE_PORTSTATE_FORWARD;
    alePortControlCfg.dropUntaggedEnable    =   0;
    alePortControlCfg.vidIngressCheckEnable =   0;
    alePortControlCfg.noLearnModeEnable     =   0;
    alePortControlCfg.mcastLimit            =   0;
    alePortControlCfg.bcastLimit            =   0;

    CSL_CPSW_3GF_setAlePortControlReg (0, &alePortControlCfg);
    CSL_CPSW_3GF_setAlePortControlReg (1, &alePortControlCfg);
    CSL_CPSW_3GF_setAlePortControlReg (2, &alePortControlCfg);

#ifdef SIMULATOR_SUPPORT
    CSL_CPSW_3GF_enableAleBypass();
#endif
    /* Done with switch configuration */
    return;
}

/******************************************************************************
* Init_MAC
******************************************************************************/
void Init_MAC(uint32_t macPortNum)
{
    CSL_CPGMAC_SL_resetMac (macPortNum);
    while (CSL_CPGMAC_SL_isMACResetDone (macPortNum) != TRUE);

    /* Setup the MAC Control Register for this port:
     *      (1) Enable Full duplex
     *      (2) Enable GMII
     *      (3) Enable Gigabit
     *      (4) Enable External Configuration. This enables
     *          the "Full duplex" and "Gigabit" settings to be
     *          controlled externally from SGMII
     *      (5) Don't Enable any control/error frames
     *      (6) Enable short frames
     */
    CSL_CPGMAC_SL_enableFullDuplex (macPortNum);
    CSL_CPGMAC_SL_enableGMII (macPortNum);
    CSL_CPGMAC_SL_enableGigabit (macPortNum);
    CSL_CPGMAC_SL_enableExtControl (macPortNum);
}

/******************************************************************************
* flushCache 
******************************************************************************/
static void flushCache (void)
{
    uint32_t  key;

    /* Disable Interrupts */
    key = _disable_interrupts();
    CSL_XMC_invalidatePrefetchBuffer();

    /*-------------------------------------------------------------------------
    * Also flushes L1P and L1D. 
    *------------------------------------------------------------------------*/
    CACHE_wbInvAllL2(CACHE_NOWAIT);

    _mfence();
    asm(" NOP 9");
    asm(" NOP 7");

    /* Reenable Interrupts. */
    _restore_interrupts(key);
}

/******************************************************************************
* hwWbInvL1DInline
******************************************************************************/
void inline hwWbInvL1DInline(void)
{
    DEVICE_REG32_W (DEVICE_CACHE_BASE + CACHE_REG_L1DWBIV, 1);
}

/*******************************************************************************
 * FUNCTION PURPOSE: Setup a mux value
 *******************************************************************************
 * DESCRIPTION: Configures one of the 12 interrupt mux values. Original value
 *              is lost.
 ******************************************************************************/
void hwIntctlRoute (uint32_t vector, uint32_t eventNum)
{
    uint32_t muxp;
    uint32_t muxv;
    uint32_t base;

    if (vector > 3 && vector < 8)
        muxp = 1;
    else if (vector < 12)
        muxp = 2;
    else if (vector < 16)
        muxp = 3;
    else
        return;  /* Invalid vector */

    /* Which of the four events in each register (0-3) is determined by the the
     * two lsbs of the vector number. The least significant bit of the mux
     * valud (0, 8, 16, or 24) is found. */
    base = (vector & 0x3) * 8;

    /* Read the active mux, overwrite the event num with the desired value */
    muxv = DEVICE_REG32_R (DEVICE_INTCTL_BASE + INTCTL_REG_MUX(muxp));
    muxv = BOOT_SET_BITFIELD (muxv, eventNum, base+7, base);
    DEVICE_REG32_W (DEVICE_INTCTL_BASE + INTCTL_REG_MUX(muxp), muxv);

} /* hwIntctlRoute */




/******************************************************************************
 * FUNCTION PURPOSE: Idle
 ******************************************************************************
 * DESCRIPTION: The IPC interrupt is routed to the core, and idle is executed
 ******************************************************************************/
void idle_till_wakeup (void)
{
    uint32_t      csrReg;

    /* Clear any pending interrupts and route the IPC interrupt to vector 4 */
    ICR = 0x0013;
    hwIntctlRoute (4, DEVICE_INT_IPC);

    IER = (1 << 4) | 3;

    /* Point the ISTP to the ROM vector table */
    ISTP = (uint32_t)BOOTROM_RESET_VECTOR_LOCATION;

    /* Write back/invalidate L1D. Use inline since the magic address
     * may live on the same cache line as the current stack */
    hwWbInvL1DInline(); 

    /* Globally enable interrupts */
    csrReg = CSR | 1;
    CSR    = csrReg;

    asm(" NOP 4 ");
    asm(" IDLE ");
    asm(" NOP 4 ");

    /* On wakeup disable interrupts and restore the system state */
    csrReg = CSR & ~1;
    CSR    = csrReg;
    IER &= (~(1 << 4));
    ICR = 0x0013;
} 

/******************************************************************************
* wait_for_interrupt
******************************************************************************/
void wait_for_interrupt()
{
    if (DNUM == 0)
    {
        /* clear PCIe interrupt A */
        *((volatile unsigned int *)LEGACY_A_IRQ_STATUS) = 0x1;
        *((volatile unsigned int *)TI667X_IRQ_EOI)      = 0x0;

        while (*((volatile unsigned int *)LEGACY_A_IRQ_STATUS_RAW) != 0x1);

        /* clear PCIe interrupt A */
        *((volatile unsigned int *)LEGACY_A_IRQ_STATUS) = 0x1;
        *((volatile unsigned int *)TI667X_IRQ_EOI)      = 0x0;
    }
    else idle_till_wakeup();
}



/*-----------------------------------------------------------------------------
* Allocate memory for the system stack. The section will be sized by the linker.
*----------------------------------------------------------------------------*/
__asm("\t.global __TI_STACK_END");
__asm("\t.global __TI_STATIC_BASE");

#pragma DATA_ALIGN   (_stack, 8);
#pragma DATA_SECTION (_stack, ".stack");
char _stack[8];

/*-----------------------------------------------------------------------------
* We will place the c_int00 entry point into its own section so that a linker 
* command file can guarantee its placement.
*----------------------------------------------------------------------------*/
#pragma CODE_SECTION (_c_int00, ".c_int00");

/*****************************************************************************/
/* C_INT00() - C ENVIRONMENT ENTRY POINT                                     */
/*****************************************************************************/
extern void __interrupt _c_int00()
{
    /*-------------------------------------------------------------------------
    * After a reset, the device should have invalidated caches.  The caches will
    * still be configured as they were prior to the reset.  Since this code was 
    * loaded into L2, we will ensure that L2 is configured as all sram.  
    *------------------------------------------------------------------------*/
    CACHE_setL2Size (CACHE_0KCACHE);
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL1PSize(CACHE_L1_32KCACHE);

    /*-------------------------------------------------------------------------
    * Set up the stack pointer in b15.
    * The stack pointer points 1 word past the top of the stack, so subtract
    * 1 word from the size. also the sp must be aligned on an 8-byte boundary
    *------------------------------------------------------------------------*/
    __asm("\t   MVKL\t\t   __TI_STACK_END - 4, SP");
    __asm("\t   MVKH\t\t   __TI_STACK_END - 4, SP");
    __asm("\t   AND\t\t   ~7,SP,SP");               

    /*-------------------------------------------------------------------------
    * Set up the global data page pointer in b14.
    *------------------------------------------------------------------------*/
    __asm("\t   MVKL\t\t   __TI_STATIC_BASE,DP");
    __asm("\t   MVKH\t\t   __TI_STATIC_BASE,DP");

    /*-------------------------------------------------------------------------
    * disable cache for all addrs over 0x1000:0000
    *------------------------------------------------------------------------*/
    memset((void*)0x01848040, 0, 960);

    /*-------------------------------------------------------------------------
    * disable mpax registers 3 and above
    *------------------------------------------------------------------------*/
    memset((void*)0x08000018, 0, 104);

    /*-------------------------------------------------------------------------
    * disable msmc ses mpax registers  except for the first one at each pri lev
    *------------------------------------------------------------------------*/
    if (DNUM == 0) 
    {
        int i;
        for (i=0; i < 16; i++)
            memset((void*)(0x0bc00600 + (i * 0x40) + 8), 0, 0x38);
    }

    /*-------------------------------------------------------------------------
    * Set up floating point registers
    *------------------------------------------------------------------------*/
    FADCR = 0; FMCR  = 0;

    /*-------------------------------------------------------------------------
    * Setup platform specifics, i.e. uarts, ethernet, etc.
    *------------------------------------------------------------------------*/
    if (DNUM == 0)
    {
        /*---------------------------------------------------------------------
        * Check if Boot time init configuration is loaded
        *
        * This code is reading l2 memory written by the host.  The values were
        * written before this code began running, so the cache invalidate at 
        * reset should ensure that when we read these values we will miss l1 and
        * read directly from l2.
        *--------------------------------------------------------------------*/
        platform_init_config config;
        config.pllm  = 0; // Original configuraion : default 0 -> 1 GHz 
        if (init_config.magic_number == 0xBABEFACE)
            config.pllm = init_config.dsp_pll_multiplier;

        /* Platform initialization */
        platform_init_flags flags;
        flags.pll  = 0x1;
        flags.ddr  = 0x1;
        flags.tcsl = 0x1;
        flags.phy  = 0x0;
        flags.ecc  = 0x1;

        platform_init(&flags, &config);
        platform_uart_init();
        platform_uart_set_baudrate(DEF_INIT_CONFIG_UART_BAUDRATE);

        memset((void*)&flags, 0 , sizeof(platform_init_flags));
        memset((void*)&config, 0, sizeof(platform_init_config));

        flags.pll = 0;
        flags.ddr = 0;
        flags.tcsl = 1;
        flags.phy = 1;
        flags.ecc = 0;
        platform_init(&flags, &config);

        Init_MAC(0);
        Init_MAC(1);

        Init_Switch(1506);
    }

    /*-------------------------------------------------------------------------
    * Once we write 0 to the boot magic addr, the host can proceed with the 
    * loading of another program that will subsequently run. We should ensure
    * that the caches are written back and clean at this point so that a 
    * subsequent writeback opertation will not clobber the program loaded 
    * from the host.  It is also clearly important that the loaded program 
    * did not write over the L2 area containing the remainder of this code.
    *------------------------------------------------------------------------*/
    BOOT_MAGIC_CONTENTS = 0;
    CACHE_wbInvL1d((void*)(BOOT_MAGIC_ADDR & ~0x3f), 64, CACHE_WAIT);

    wait_for_interrupt();

    /*-------------------------------------------------------------------------
    * We will now wait until an external entity writes the address to which we 
    * should jump 
    *------------------------------------------------------------------------*/
    while (1)
    {
        /*---------------------------------------------------------------------
        * invalidate the address so that we pick up the actual memory written by
        * the external entity.
        *--------------------------------------------------------------------*/
        CACHE_invL1d((void*)(BOOT_MAGIC_ADDR & ~0x3f), 64, CACHE_WAIT);
        void (*entry)() = (void (*)())(BOOT_MAGIC_CONTENTS);

        /*---------------------------------------------------------------------
        * If we have a non null pointer then we will branch to it. This 
        * essentially marks the end of this routine and the start of another,
        * so we should ensure that the caches are written back and clean at 
        * this point so that the following program starts with a clean cache 
        * system.
        *
        * It is also clearly important that the loaded program did not write 
        * over the L2 area containing this reset code. Since this reset code 
        * is entirely resident in the last 1/4 of L2, this area should not 
        * contain initialized data or code in the following program.
        *--------------------------------------------------------------------*/
        if (entry) 
        {
            flushCache();
            (*entry)();
        }
    }
}
