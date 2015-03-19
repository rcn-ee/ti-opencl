#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ti/platform/platform.h>
#include <ti/csl/csl_bootcfgAux.h>
#include <ti/csl/csl_cacheAux.h>
#include "mailBox.h"
#include "message.h"

#pragma DATA_SECTION(L2_KCFG, ".workgroup_config");
struct 
{
    unsigned num_dims;
    unsigned global_sz_0;
    unsigned global_sz_1;
    unsigned global_sz_2;
    unsigned local_sz_0;
    unsigned local_sz_1;
    unsigned local_sz_2;
    unsigned global_off_0;
    unsigned global_off_1;
    unsigned global_off_2;
    unsigned WG_gid_start_0;
    unsigned WG_gid_start_1;
    unsigned WG_gid_start_2;
} L2_KCFG;

extern cregister volatile unsigned int DNUM;

/*This should be abstracted to high level functions through platform library */
#define DEVICE_REG32_W(x,y)   *(volatile uint32_t *)(x)=(y)
#define DEVICE_REG32_R(x)    (*(volatile uint32_t *)(x))
#define IPCGR(x)            (0x02620240 + x*4)

/******************************************************************************
* getNodeId
******************************************************************************/
int32_t getNodeId(void)
{
#define GPIO_IN_DATA 0x02320020	/* for DSP number */

    uint32_t dsp_id  = (((*(unsigned int*) GPIO_IN_DATA) & 0x6) >> 1);		/* GPIO 1~2 */
    uint32_t core_id = DNUM;
    return((int32_t)(dsp_id*8 + core_id));
}

/******************************************************************************
* getChipMasterNodeId : Return core 0 on the same chip as the current core
******************************************************************************/
int32_t getChipMasterNodeId(void)
{
#define GPIO_IN_DATA 0x02320020	/* for DSP number */

    uint32_t dsp_id  = (((*(unsigned int*) GPIO_IN_DATA) & 0x6) >> 1);		/* GPIO 1~2 */
    return((int32_t)(dsp_id*8));
}

/******************************************************************************
* disableCache
******************************************************************************/
void disableCache(unsigned start, unsigned next_start)
{
    volatile unsigned int *MAR = (volatile unsigned int *)0x1848000;

    start >>= 24;
    next_start--;
    next_start >>= 24;

    int i;
    for (i = start; i <= next_start; ++i) MAR[i] = 0x0;
}

/******************************************************************************
* enableCache
******************************************************************************/
void enableCache(unsigned start, unsigned next_start)
{
    volatile unsigned int *MAR = (volatile unsigned int *)0x1848000;

    start >>= 24;
    next_start--;
    next_start >>= 24;

    /*-------------------------------------------------------------------------
    * Cacheable, Prefetchable, Write through
    *------------------------------------------------------------------------*/
    int i;
    for (i = start; i <= next_start; ++i) MAR[i] = 0xB;
}

/******************************************************************************
* Setup the cores for execution
******************************************************************************/
void initializeCores()
{
    if (DNUM == 0)
    {
        platform_init_flags flags;
        platform_init_config config;

        /*---------------------------------------------------------------------
        * Platform initialization
        *--------------------------------------------------------------------*/
        flags.pll    = 0x0;
        flags.ddr    = 0x0;
        flags.tcsl   = 0x1;
        flags.phy    = 0x0;
        flags.ecc    = 0x0;
        config.pllm  = 0;

        platform_init(&flags, &config);
        CSL_BootCfgUnlockKicker();

        /*---------------------------------------------------------------------
        * wake up the other cores
        *--------------------------------------------------------------------*/
        int core;
        for (core = 1; core < 8; core++)
        {
            /*-----------------------------------------------------------------
            * IPC interrupt other cores
            *----------------------------------------------------------------*/
            DEVICE_REG32_W(IPCGR(core), 1);
            platform_delay(1000);
        }
    }
}

/******************************************************************************
* incVec
******************************************************************************/
int incVec(unsigned dims, unsigned *vec, unsigned *inc, unsigned *maxs)
{
    int overflow = 0;
    int i;

    for (i = 0; i < dims; ++i)
    {
        vec[i] += inc[i];

        if (vec[i] < maxs[i]) 
             { overflow = 0; break; }
        else { vec[i] = 0; overflow = 1; }
    }

    return overflow;
}


/******************************************************************************
* main
******************************************************************************/
void* dsp_rcp(void* p);
void main(void)
{
    CACHE_setL1DSize(CACHE_L1_MAXIM1);  // L1D all cache
    CACHE_setL1PSize(CACHE_L1_MAXIM1);  // L1P all cache
    CACHE_setL2Size (CACHE_128KCACHE);  // L2  1/4 cache

    enableCache (0x0C000000, 0xD0000000); // Cache on for MSMC
    disableCache(0x80000000, 0x81000000); // Cache off for 1st 16M DDR
    enableCache (0x81000000, 0xC0000000); // Cache on for remainder DDR

    initializeCores();

    int32_t node_id   = getNodeId();
    int32_t master_id = getChipMasterNodeId();
    Msg_t   Msg;

    int32_t rx_mbox;
    int32_t tx_mbox;

    /*-------------------------------------------------------------------------
    * If the master core, open two way mailboxes to the host
    *------------------------------------------------------------------------*/
    if (DNUM == 0)
    {
        mailBox_init(MAILBOX_NODE_ID_HOST, node_id);
        mailBox_init(node_id, MAILBOX_NODE_ID_HOST);

        rx_mbox = mailBox_open(MAILBOX_NODE_ID_HOST, node_id);
        tx_mbox = mailBox_open(node_id, MAILBOX_NODE_ID_HOST);
    }
    else return;

    int     ready   = (int)READY;
    int     success = (int)SUCCESS;

    while (1)
    {
        uint32_t    size;
        uint32_t    trans_id;

        mailBox_read(rx_mbox, (uint8_t *)&Msg, &size, &trans_id);

        int i;
        for (i = 0; i < Msg.number_commands; ++i)
        {
            switch (Msg.commands[i].code)
            {
                default:   break;
                case EXIT: return;

                case CACHEINV: CACHE_invL2((void*)Msg.commands[i].addr, 
                                           Msg.commands[i].size, CACHE_WAIT);
                               break;

                case NDRKERNEL: 
                 {
                    unsigned* Kcfg_ptr   = (void*)Msg.commands[i].addr;
                    unsigned  Kcfg_size  = Msg.commands[i].size;
                    unsigned* args= &Kcfg_ptr[sizeof(L2_KCFG)/sizeof(unsigned)];
                    unsigned  WGid[3] = {0,0,0};
                    int       done;

                    /*---------------------------------------------------------
                    * Invalidate the packet, so we read fresh data
                    *--------------------------------------------------------*/
                    CACHE_invL2((void*)Kcfg_ptr, Kcfg_size, CACHE_WAIT);

                    /*---------------------------------------------------------
                    * Copy the configuration in L2, where the kernel wants it
                    *--------------------------------------------------------*/
                    memcpy((void*)&L2_KCFG, Kcfg_ptr, sizeof(L2_KCFG));
                    
                    /*---------------------------------------------------------
                    * Iterate over wach Work Group
                    *--------------------------------------------------------*/
                    do 
                    {
                        L2_KCFG.WG_gid_start_0 = L2_KCFG.global_off_0 + WGid[0];
                        L2_KCFG.WG_gid_start_1 = L2_KCFG.global_off_1 + WGid[1];
                        L2_KCFG.WG_gid_start_2 = L2_KCFG.global_off_2 + WGid[2];

                        dsp_rcp(args);

                        done = incVec(L2_KCFG.num_dims, WGid, 
                                     &L2_KCFG.local_sz_0, &L2_KCFG.global_sz_0);
                    } while (!done);

                    /*---------------------------------------------------------
                    * Go Ahead and indicate success, before the buffers are 
                    * invalidated, becuase the invalidation code will complete
                    * before any new tasks can start.
                    *--------------------------------------------------------*/
                    mailBox_write(tx_mbox, (uint8_t *)&success, sizeof(success),
                                  trans_id); 
                    break;
                 }
            }
        }

        mailBox_write(tx_mbox, (uint8_t *)&ready, sizeof(ready), trans_id); 
    } 
}

