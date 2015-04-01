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

/*-----------------------------------------------------------------------------
* C standard library
*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
* Platform and Chip Support
*----------------------------------------------------------------------------*/
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_bootcfgAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_chipAux.h>
#include <ti/drv/qmss/qmss_drv.h>

/*-----------------------------------------------------------------------------
* For Monitor
*----------------------------------------------------------------------------*/
#include "monitor.h"
#include "util.h"
#ifdef TI_66AK2H
#include "edma.h"
#endif
#include "trace.h"
#include "ocl_qmss.h"

/*-----------------------------------------------------------------------------
* SYS/BIOS Header Files
*----------------------------------------------------------------------------*/
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

/*-----------------------------------------------------------------------------
* XDC includes
*----------------------------------------------------------------------------*/
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Log.h>

/*-----------------------------------------------------------------------------
* Application headers
*----------------------------------------------------------------------------*/
#ifdef TI_66AK2H
    #include "mpm_mailbox.h"
#else
    #include "mailBox.h"
    #define MPM_MAILBOX_ERR_FAIL                MAILBOX_ERR_FAIL
    #define MPM_MAILBOX_ERR_MAIL_BOX_FULL       MAILBOX_ERR_MAIL_BOX_FULL
    #define MPM_MAILB0X_ERR_EMPTY               MAILB0X_ERR_EMPTY
    #define MPM_MAILBOX_READ_ERROR              MAILBOX_READ_ERROR
    #define MPM_MAILBOX_MEMORY_LOCATION_LOCAL   MAILBOX_MEMORY_LOCATION_LOCAL
    #define MPM_MAILBOX_MEMORY_LOCATION_REMOTE  MAILBOX_MEMORY_LOCATION_REMOTE
    #define MPM_MAILBOX_DIRECTION_RECEIVE       MAILBOX_DIRECTION_RECEIVE
    #define MPM_MAILBOX_DIRECTION_SEND          MAILBOX_DIRECTION_SEND
    #define mpm_mailbox_config_t                mailBox_config_t
    #define mpm_mailbox_create                  mailBox_create
    #define mpm_mailbox_open                    mailBox_open
    #define mpm_mailbox_write                   mailBox_write
    #define mpm_mailbox_read                    mailBox_read
    #define mpm_mailbox_query                   mailBox_query
    #define mpm_mailbox_get_alloc_size          mailBox_get_alloc_size
#endif

#if defined(ULM_ENABLED)
#include "tiulm.h"
#endif

#if defined(GDB_ENABLED)
#include "GDB_server.h"
DDR (EdmaMgr_Handle, gdb_channel) = NULL;
#endif

/*-----------------------------------------------------------------------------
* Mailbox structures
*----------------------------------------------------------------------------*/
FAST_SHARED(void*, rx_mbox) = NULL;
FAST_SHARED(void*, tx_mbox) = NULL;
FAST_SHARED_1D(char, rx_mbox_mem, 64);
FAST_SHARED_1D(char, tx_mbox_mem, 64);

PRIVATE(Msg_t*, printMsg)   = NULL;
PRIVATE_1D(char, print_msg_mem, sizeof(Msg_t));

/******************************************************************************
* Defines a fixed area of 64 bytes at the start of L2, where the kernels will
* resolve the get_global_id type calls
******************************************************************************/
#pragma DATA_SECTION(kernel_config_l2, ".workgroup_config");
EXPORT kernel_config_t kernel_config_l2;

/******************************************************************************
* Initialization Routines
******************************************************************************/
static int initialize_cores         (void);
static int initialize_qmss          (void);
static int initialize_bios_tasks    (void);
static int initialize_host_mailboxes(void);
static int initialize_memory        (void);
static int initialize_gdbserver     (void);
static void reset_memory            (void);

/******************************************************************************
* Bios Task Routines
******************************************************************************/
#define BIOS_TASK(name) void name(void)

BIOS_TASK(listen_host_task);
BIOS_TASK(service_wg_task);

/******************************************************************************
* External prototypes
******************************************************************************/
extern unsigned dsp_speed();
extern void*    dsp_rpc(void* p, void *more_args, uint32_t more_args_size);
extern int      tomp_initOpenMPforOpenCL(void);
extern void     tomp_exitOpenMPforOpenCL(void);

/******************************************************************************
* Workgroup dispatch routines
******************************************************************************/
static int  incVec(unsigned dims, unsigned *vec, unsigned *inc, unsigned *maxs);
void        mail_safely(uint8_t* msg, size_t msgSz, uint32_t msgId);

static void  process_kernel_command(Msg_t* msg);
static void  process_task_command  (Msg_t* msg);
static void  process_cache_command (flush_msg_t* Msg);
static int   process_exit_command  (void);
static void  core0_to_allcores     (uint32_t flush_msg_id, flush_msg_t *f_msg);


void _minit(void) { }

_CODE_ACCESS void __TI_writemsg(               unsigned char  command,
                                register const unsigned char *parm,
                                register const          char *data,
                                                unsigned int  length)
{
    if (tx_mbox == NULL || rx_mbox == NULL || printMsg == NULL) return;

    unsigned int msgLen = sizeof(kernel_msg_t) + sizeof(flush_msg_t);
    printMsg->u.message[0] = DNUM + '0';
    msgLen = (length <= msgLen-2) ? length : msgLen-2;
    memcpy(printMsg->u.message+1, data, msgLen);
    printMsg->u.message[msgLen+1] = '\0';
    if (tx_mbox != NULL && rx_mbox != NULL && printMsg != NULL)
        mail_safely((uint8_t*)printMsg, sizeof(Msg_t), 42);
    return;
}

_CODE_ACCESS void __TI_readmsg(register unsigned char *parm,
                               register char          *data)
{
    return;  // do nothing
}


/******************************************************************************
* main
******************************************************************************/
int main(void)
{
    if (!initialize_memory())           ERROR("Could not init Memory");
    if (!initialize_host_mailboxes())   ERROR("Could not init Mailboxes");
    if (!initialize_cores())            ERROR("Could not init Cores");
    if (!initialize_qmss())             ERROR("Could not init QMSS");
    if (!initialize_bios_tasks())       ERROR("Could not init BIOS tasks");
    if (tomp_initOpenMPforOpenCL() < 0) ERROR("Could not init OpenMP");
#ifdef TI_66AK2H
    if (!initialize_edmamgr())          ERROR("Could not init EdmaMgr"); 
#endif
    if (!initialize_gdbserver())        ERROR("Could not init GDB Server"); 

    waitAtCoreBarrier();

    //Semaphore_post(higherSem); // Enable the host listener
    BIOS_start();
}

void cycle_delay(uint32_t cycles)
{
    uint32_t start_val  = CSL_chipReadTSCL();
    while ((CSL_chipReadTSCL() - start_val) < cycles);
}

/******************************************************************************
* Setup the cores for execution
******************************************************************************/
static int initialize_cores()
{
#ifndef TI_66AK2H
    /*---------------------------------------------------------------------
    * should be abstracted to high level functions through platform library
    *--------------------------------------------------------------------*/
#define DEVICE_REG32_W(x,y)   *(volatile uint32_t *)(x)=(y)
#define IPCGR(x)            (0x02620240 + x*4)

    /*---------------------------------------------------------------------
    * ipc interrupt other cores
    *--------------------------------------------------------------------*/
    int core;
    if (DNUM == 0)
        for (core = 1; core < 8; core++)
        {
            DEVICE_REG32_W(IPCGR(core), 1);
            cycle_delay(1000000);
        }
#endif

    return RETURN_OK;
}

/******************************************************************************
* initialize_qmss
* Keystone 2 needs to get QMSS resources allocated by RM on ARM
******************************************************************************/
#ifdef TI_66AK2H
int OCL_QMSS_HW_QUEUE_BASE_IDX             = 7300;
int OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM = 8192;
int OCL_QMSS_FIRST_MEMORY_REGION_IDX       = 16;
#endif

static int initialize_qmss(void)
{
#ifdef TI_66AK2H
    if (DNUM == 0)
    {
        uint32_t size, trans_id;
        int      mail_available;
        do mail_available = mpm_mailbox_query(rx_mbox);
        while (!mail_available);
        Msg_t Msg;
        mpm_mailbox_read(rx_mbox, (uint8_t *)&Msg, &size, &trans_id);
        if (((int *)&Msg)[1] > 0)
        {
            OCL_QMSS_HW_QUEUE_BASE_IDX             = ((int *)&Msg)[1];
            OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM = ((int *)&Msg)[3];
            OCL_QMSS_FIRST_MEMORY_REGION_IDX       = ((int *)&Msg)[2];
        }
    }
#endif

    if (DNUM == 0 && !ocl_initGlobalQMSS())   return RETURN_FAIL; 
    waitAtCoreBarrier();
    if (!ocl_initLocalQMSS())                 return RETURN_FAIL; 
    waitAtCoreBarrier();

    if (DNUM == 0 && !initClockGlobal()) return RETURN_FAIL; 
    waitAtCoreBarrier();
    if (!initClockLocal())               return RETURN_FAIL; 
    waitAtCoreBarrier();

    return RETURN_OK;
}

/******************************************************************************
* flush_buffers
******************************************************************************/
void flush_buffers(flush_msg_t *Msg)
{
    /*-------------------------------------------------------------------------
    * For now we will flush all L2 due to an issue with the below.
    *
    * This issue involves prefetching of lines beyond our defined buffers.
    * Since the prefetch will result in lines being cached that we are not 
    * aware of, if the host subsequently writes that area and then this core 
    * reads the area, it will get stale data in the prefetched overflow area.
    * 
    * Possible solutions include:
    * 1) invalidate all L2 cache as a sledgehammer
    * 2) invalidate our buffers before the kernel runs, this ensures that the 
    *    exact area we care about is flushed. The cost will be running flush 
    *    before the kernel runs.
    * 3) Understand the prefetch boundaries and also add a pre and post 
    *    overflow to our defined buffers.
    *
    * The cost of out infalidates is mostly in the writes that need to occur.
    * We write nothing back from L2, but we do WBInv on L1, because it could 
    * be caching an area that is not write though (i.e. l2 sram).  One day we 
    * could perhaps write a routine that would invalidate lines in L1 associated
    * with ddr addresss, and would leave other lines alone.
    *------------------------------------------------------------------------*/
    cacheInvAllL2(); return; 

#if 0
    int i;
    for (i = 0; i < (Msg->numBuffers << 1); i += 2)
    {
       /*----------------------------------------------------------------------
       * The cache routines should be given a bufPtr aligned to a cache line
       *    and a size that is a multiple of the cache line size.  The bufPtr
       *    alignment should be handled, since the memory allocator will only 
       *    allocate blocks on cache line boundaries.  The size arg may not be 
       *    a multiple of cache line size, but it should be safe to round it 
       *    up to the next cache line size boundary, since again the allocator
       *    will allocate memmory only in cache line size chunks. Also, note
       *    that invL2 calls will also invalidate L1P and L1D.
       *---------------------------------------------------------------------*/
       cacheInvL2((void*)Msg->buffers[i], 
                  CACHE_ROUND_TO_LINESIZE(L2, Msg->buffers[i+1], 1));
    }
#endif
}


/******************************************************************************
* kernel_complete
*   This function is called when all workgroups for a kernel have completed.
******************************************************************************/
EVENT_HANDLER(service_kernel_complete)
{
    /*-------------------------------------------------------------------------
    * We only preschedule if the core is not 0, because core 0 does not 
    * currently service tasks and if it prescheduled a task it would not 
    * complete.  When core 0 does participate in task servicing, it can also
    * prefetch here.
    *------------------------------------------------------------------------*/
    // if (DNUM != 0) ti_em_preschedule();

    typedef struct { uint32_t cmd; uint32_t id; flush_msg_t Fmsg; } pkt_t; 
    pkt_t*             pkt = (pkt_t *) eventHdl;
    flush_msg_t* flush_msg = &pkt->Fmsg;
    uint32_t     num_mpaxs = flush_msg->num_mpaxs;

    /*-------------------------------------------------------------------------
    * The barrier ensures that each core picks up a single flush event from
    * the queue and waits for remaining cores. Without the barrier, a core
    * can incorreclty pick up and execute multiple flush events, resulting in
    * undefined behavior.
    *------------------------------------------------------------------------*/
    switch(pkt->id)
    {
        case FLUSH_MSG_KERNEL_PROLOG:
            if (num_mpaxs > 0)
                set_kernel_MPAXs(num_mpaxs, flush_msg->mpax_settings);
            printMsg = (Msg_t *) print_msg_mem;
            waitAtCoreBarrier(); 
            break;

        case FLUSH_MSG_IOTASK_EPILOG:
            printMsg = NULL;
            if (num_mpaxs > 0) reset_kernel_MPAXs(num_mpaxs);
            waitAtCoreBarrier(); 
            break;

        case FLUSH_MSG_CACHEINV:
        default:
 
            if (flush_msg->numBuffers > 0 || num_mpaxs > 0)
            {
                if (flush_msg->numBuffers > 0) flush_buffers(flush_msg);
                if (num_mpaxs > 0)             reset_kernel_MPAXs(num_mpaxs);
            }

            waitAtCoreBarrier(); 

            /*-----------------------------------------------------------------
            *  Send ready message after core barrier to ensure all cores
            *  complete their workgroup before the message is sent by core 0
            *----------------------------------------------------------------*/
            if (DNUM == 0)
                mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), pkt->id);
 
            TRACE(ULM_OCL_NDR_CACHE_COHERENCE_COMPLETE, pkt->id, 0);
            break;
    }

    ocl_descriptorFree(eventHdl);

    if (DNUM == 0) Semaphore_post(higherSem);
}

/******************************************************************************
* service_workgroup
*   This function is called by the Queue manager system to service an 
*   individual work group for a kernel.
******************************************************************************/
EVENT_HANDLER(service_workgroup)
{
    // if (DNUM != 0) ti_em_preschedule(); // prefetch request

    /*-------------------------------------------------------------------------
    * access event buffer of received event
    *------------------------------------------------------------------------*/
    Msg_t *msg = (Msg_t*) eventHdl;
    uint32_t wg_type = msg->command;

    /*-------------------------------------------------------------------------
    * Flush descriptors in the workgroup queue are handled by
    * service_kernel_complete
    *------------------------------------------------------------------------*/
    if (wg_type == FLUSH)
    {
        service_kernel_complete(eventHdl);
        return;
    }

    kernel_msg_t* kernelMsgPtr = &msg->u.k.kernel;

    /*---------------------------------------------------------
    * Copy the configuration in L2, where the kernel wants it
    *--------------------------------------------------------*/
    memcpy((void*)&kernel_config_l2, (void*)&kernelMsgPtr->config, 
           sizeof(kernel_config_t));

    uint32_t num_mpaxs = msg->u.k.flush.num_mpaxs;
    if (wg_type == TASK && num_mpaxs > 0)  // OOTask self-set MPAX
        set_kernel_MPAXs(num_mpaxs, msg->u.k.flush.mpax_settings);
    printMsg = (Msg_t *) print_msg_mem;  // enable printf before ocl kernel

    /*-------------------------------------------------------------------------
    * Run the kernel workgroup
    *------------------------------------------------------------------------*/
    uint32_t kId  = kernel_config_l2.Kernel_id;
    uint32_t wgId = kernel_config_l2.WG_id;

    uint32_t more_args_size = msg->u.k.flush.args_on_stack_size;
    void *   more_args      = (void *) msg->u.k.flush.args_on_stack_addr;
    TRACE(wg_type == TASK ? ULM_OCL_OOT_KERNEL_START : ULM_OCL_NDR_KERNEL_START, kId, wgId);
    clear_mpf();
    dsp_rpc(&kernelMsgPtr->entry_point, more_args, more_args_size);
    TRACE(wg_type == TASK ? ULM_OCL_OOT_KERNEL_COMPLETE : ULM_OCL_NDR_KERNEL_COMPLETE, kId, wgId);
    report_and_clear_mpf();

    if (wg_type == TASK) 
    {
        mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), kId);

        flush_msg_t*  flushMsgPtr  = &msg->u.k.flush;
        flush_buffers(flushMsgPtr);
        TRACE(ULM_OCL_OOT_CACHE_COHERENCE_COMPLETE, kId, wgId);
    }

    printMsg = NULL;  // disable printf after ocl kernel
    if (wg_type == TASK && num_mpaxs > 0)  reset_kernel_MPAXs(num_mpaxs);

    ocl_descriptorFree(eventHdl);
}


/******************************************************************************
* service_exit
*   This function is called by the Queue manager system when it receives an
*   exit command from the host.
******************************************************************************/
EVENT_HANDLER(service_exit) 
{ 
    TRACE(ULM_OCL_EXIT, 0, 0);

#ifdef TI_66AK2H
    waitAtCoreBarrier();
    if (DNUM == 0)
    {
        tomp_exitOpenMPforOpenCL();
        ocl_exitGlobalQMSS();
    }
    free_edma_channel_pool();
#endif
#if defined(GDB_ENABLED)
    if (DNUM == 0 && gdb_channel != NULL) EdmaMgr_free(gdb_channel);
#endif
    cacheWbInvAllL2();
    reset_memory();
    exit(0); 
}

/******************************************************************************
* initialize_bios_tasks
* Core   0: service task, listen task
* Core 1-7: service task
******************************************************************************/
#define LISTEN_STACK_SIZE  0x2800
#define SERVICE_STACK_SIZE 0x2800
PRIVATE_1D(char, lstack, LISTEN_STACK_SIZE);
PRIVATE_1D(char, sstack, SERVICE_STACK_SIZE);

static int initialize_bios_tasks(void)
{
    Task_Handle taskHandle;
    Task_Params taskParams;

    Task_Params_init(&taskParams);
    taskParams.priority  = 1; // LOW_PRIORITY;
    taskParams.stackSize = SERVICE_STACK_SIZE;
    taskParams.stack = (xdc_Ptr)makeAddressGlobal(DNUM, (uint32_t)sstack);
    taskHandle = Task_create((Task_FuncPtr)service_wg_task, &taskParams, NULL);
    if (taskHandle == NULL) return RETURN_FAIL;

    if (DNUM != 0) return RETURN_OK;

    Task_Params_init(&taskParams);
    taskParams.priority  = 15; // HIGH_PRIORITY;
    taskParams.stackSize = LISTEN_STACK_SIZE;
    taskParams.stack = (xdc_Ptr)makeAddressGlobal(DNUM, (uint32_t)lstack);
    taskHandle = Task_create((Task_FuncPtr)listen_host_task, &taskParams, NULL);
    if (taskHandle == NULL) return RETURN_FAIL;

    return RETURN_OK;
}

/******************************************************************************
* listen_host_task
******************************************************************************/
BIOS_TASK(listen_host_task)
{
    uint32_t size;
    uint32_t trans_id;
    int      mail_available;

    /*-------------------------------------------------------------------------
    * Continue listening to the host until an exit command is sent
    *------------------------------------------------------------------------*/
    while (1)
    {
       do mail_available = mpm_mailbox_query(rx_mbox); 
       while (!mail_available);

       Msg_t Msg;
       mpm_mailbox_read(rx_mbox, (uint8_t *)&Msg, &size, &trans_id);

       switch (Msg.command)
       {
          case TASK: 
                      process_task_command(&Msg);
                      break;

          case NDRKERNEL: 
                      process_kernel_command(&Msg);
                      break;

          case CACHEINV: 
                      process_cache_command(&Msg.u.k.flush); 
                      break; 

          case EXIT:     
                      process_exit_command(); 
                      break;

          case FREQUENCY:    
                      mail_safely((uint8_t *)&readyMsg, 
                                    sizeof(readyMsg), dsp_speed());
                      break;
          default:    
                      mail_safely((uint8_t *)&readyMsg, 
                                    sizeof(readyMsg), trans_id);
                      break;
       }
    }
}

/******************************************************************************
* service_wg_task
******************************************************************************/
extern void tomp_dispatch_once(void);
BIOS_TASK(service_wg_task)
{
    while (1) 
    {
        ocl_dispatch_once();
        tomp_dispatch_once();
    }
}

/******************************************************************************
* Core 0 sends flush message to all cores with certain type of message id
*        pends until flush message is processed
* Note: serviced by each core with service_kernel_complete()
******************************************************************************/
static void core0_to_allcores(uint32_t flush_msg_id, flush_msg_t *f_msg)
{    
    int core;
    for (core = 0; core < NUM_CORES; core++)
    {
        uint32_t *ptr = NULL;
        do ptr = (uint32_t *)ocl_descriptorAlloc();
        while (ptr == NULL);

        *ptr++ = FLUSH;
        *ptr++ = flush_msg_id;
        memcpy(ptr, f_msg, sizeof(flush_msg_t));

        ocl_descriptorPush(ocl_queues.wgQ, ptr);
    }

    Semaphore_pend(higherSem, BIOS_WAIT_FOREVER);
}


/******************************************************************************
* process_task_command
******************************************************************************/
static void process_task_command(Msg_t* Msg) 
{
    kernel_config_t * kcfg = &Msg->u.k.kernel.config;
    uint32_t     kernel_id = kcfg->Kernel_id;
    int         is_inorder = (kcfg->global_sz_0 == IN_ORDER_TASK_SIZE);
    int       is_debugmode = (kcfg->WG_gid_start_0 == DEBUG_MODE_WG_GID_START);

    /*---------------------------------------------------------
    * Iterate over each Work Group
    *--------------------------------------------------------*/

    /*---------------------------------------------------------
    * OpenMP C code Mode: In Order Queue + Task: flag in global_size_0
    * Let Core 0 handle it directly (OpenMP requires it)
    * Out-of-Order Task in Debug mode also only run on Core 0
    *--------------------------------------------------------*/
    if (is_inorder || is_debugmode)
    {
       TRACE(ULM_OCL_IOT_OVERHEAD, kernel_id, 0);
        /*---------------------------------------------------------
        * Copy the configuration in L2, where the kernel wants it
        *--------------------------------------------------------*/
        kernel_config_l2.num_dims         = 1;
        kernel_config_l2.global_sz_0      = 1;
        kernel_config_l2.global_sz_1      = 1;
        kernel_config_l2.global_sz_2      = 1;
        kernel_config_l2.local_sz_0       = 1;
        kernel_config_l2.local_sz_1       = 1;
        kernel_config_l2.local_sz_2       = 1;
        kernel_config_l2.global_off_0     = 0;
        kernel_config_l2.global_off_1     = 0;
        kernel_config_l2.global_off_2     = 0;
        kernel_config_l2.WG_gid_start_0   = 0;
        kernel_config_l2.WG_gid_start_1   = 0;
        kernel_config_l2.WG_gid_start_2   = 0;
        kernel_config_l2.Kernel_id        = kernel_id;
        kernel_config_l2.WG_id            = 0;
        kernel_config_l2.stats            = 0;
        kernel_config_l2.WG_alloca_start  = kcfg->WG_alloca_start;
        kernel_config_l2.WG_alloca_size   = kcfg->WG_alloca_size;

        core0_to_allcores(FLUSH_MSG_KERNEL_PROLOG, &Msg->u.k.flush);

        /*--------------------------------------------------------------------
        * Run the OpenMP program
        *--------------------------------------------------------------------*/
        uint32_t more_args_size = Msg->u.k.flush.args_on_stack_size;
        void *   more_args      = (void *) Msg->u.k.flush.args_on_stack_addr;
        clear_mpf();
        TRACE(is_inorder ? ULM_OCL_IOT_KERNEL_START
                         : ULM_OCL_OOT_KERNEL_START, kernel_id, 0);
        dsp_rpc(&((kernel_msg_t *)&Msg->u.k.kernel)->entry_point,
                more_args, more_args_size);
        TRACE(is_inorder ? ULM_OCL_IOT_KERNEL_COMPLETE
                         : ULM_OCL_OOT_KERNEL_COMPLETE, kernel_id, 0);
        report_and_clear_mpf();
        mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), kernel_id);
        flush_msg_t*  flushMsgPtr  = &Msg->u.k.flush;
        flush_buffers(flushMsgPtr);
        TRACE(is_inorder ? ULM_OCL_IOT_CACHE_COHERENCE_COMPLETE
                         : ULM_OCL_OOT_CACHE_COHERENCE_COMPLETE, kernel_id, 0);

        core0_to_allcores(FLUSH_MSG_IOTASK_EPILOG, &Msg->u.k.flush);
        return;
    }
    TRACE(ULM_OCL_OOT_OVERHEAD, kernel_id, 0);

    /*---------------------------------------------------------------------
    * If I generate more WGs than I have buffers reserved for, then this 
    * call will block until buffers become available.
    *--------------------------------------------------------------------*/
    void* eventBuf = NULL;
    do eventBuf = ocl_descriptorAlloc();
    while (eventBuf == NULL);

    Msg_t* emMsg = (Msg_t*) eventBuf;
    memcpy(eventBuf, Msg, sizeof(Msg_t));

    kernel_config_t *cfg = &emMsg->u.k.kernel.config;
    cfg->num_dims         = 1;
    cfg->global_sz_0      = 1;
    cfg->global_sz_1      = 1;
    cfg->global_sz_2      = 1;
    cfg->local_sz_0       = 1;
    cfg->local_sz_1       = 1;
    cfg->local_sz_2       = 1;
    cfg->global_off_0     = 0;
    cfg->global_off_1     = 0;
    cfg->global_off_2     = 0;
    cfg->WG_gid_start_0   = 0;
    cfg->WG_gid_start_1   = 0;
    cfg->WG_gid_start_2   = 0;
    cfg->Kernel_id        = kernel_id;
    cfg->WG_id            = 0;
    cfg->stats            = 0;

    ocl_descriptorPush(ocl_queues.wgQ, eventBuf);
}


/******************************************************************************
* process_kernel_command
******************************************************************************/
static void process_kernel_command(Msg_t* msg)
{
    int               done;
    uint32_t          workgroup = 0;
    uint32_t          WGid[3]   = {0,0,0};
    kernel_config_t * cfg       = &msg->u.k.kernel.config;
    int  is_debugmode = (cfg->WG_gid_start_0 == DEBUG_MODE_WG_GID_START);

    TRACE(ULM_OCL_NDR_OVERHEAD, cfg->Kernel_id, 0);

    /*-------------------------------------------------------------------------
    * Kernel Prolog: set MPAX settings for all cores
    *------------------------------------------------------------------------*/
    uint32_t num_mpaxs = msg->u.k.flush.num_mpaxs;
    if (num_mpaxs > 0)
        core0_to_allcores(FLUSH_MSG_KERNEL_PROLOG, &msg->u.k.flush);

    /*-------------------------------------------------------------------------
    * Debug Mode: Run all workgroups on Core 0, return
    *------------------------------------------------------------------------*/
    if (is_debugmode)
    {
        printMsg = (Msg_t *) print_msg_mem;

        do
        {
            cfg->WG_gid_start_0 = cfg->global_off_0 + WGid[0];
            cfg->WG_gid_start_1 = cfg->global_off_1 + WGid[1];
            cfg->WG_gid_start_2 = cfg->global_off_2 + WGid[2];
            cfg->WG_id          = workgroup++;
            memcpy((void*)&kernel_config_l2, cfg, sizeof(kernel_config_t));

            /*-----------------------------------------------------------------
            * Run the kernel workgroup
            *----------------------------------------------------------------*/
            clear_mpf();
            TRACE(ULM_OCL_NDR_KERNEL_START,    cfg->Kernel_id, cfg->WG_id);
            dsp_rpc(&msg->u.k.kernel.entry_point,
                    (void *) msg->u.k.flush.args_on_stack_addr,
                    msg->u.k.flush.args_on_stack_size);
            TRACE(ULM_OCL_NDR_KERNEL_COMPLETE, cfg->Kernel_id, cfg->WG_id);
            report_and_clear_mpf();

            done = incVec(cfg->num_dims, WGid, &cfg->local_sz_0, &cfg->global_sz_0);
        } while (!done);

        printMsg = NULL;
        core0_to_allcores(cfg->Kernel_id, &msg->u.k.flush);
        return;
    }

    /*---------------------------------------------------------
    * Iterate over each Work Group
    *--------------------------------------------------------*/
    do 
    {
        void* eventHdl;

        /*---------------------------------------------------------------------
        * If I generate more WGs than I have buffers reserved for, then this 
        * call will block until buffers become available.
        *--------------------------------------------------------------------*/
        do eventHdl = ocl_descriptorAlloc();
        while (eventHdl == NULL);

        uint8_t* eventBuf    = (uint8_t*)(eventHdl);
        Msg_t* emMsg = (Msg_t*) eventBuf;
        memcpy(emMsg, msg, sizeof(Msg_t));
        emMsg->command = NDRKERNEL;
        kernel_config_t *cfg = &emMsg->u.k.kernel.config;

        cfg->WG_gid_start_0 = cfg->global_off_0 + WGid[0];
        cfg->WG_gid_start_1 = cfg->global_off_1 + WGid[1];
        cfg->WG_gid_start_2 = cfg->global_off_2 + WGid[2];
        cfg->WG_id          = workgroup++;

        ocl_descriptorPush(ocl_queues.wgQ, eventHdl);

        done = incVec(cfg->num_dims, WGid, &cfg->local_sz_0, &cfg->global_sz_0);

    } while (!done);

    core0_to_allcores(cfg->Kernel_id, &msg->u.k.flush);
}

/******************************************************************************
* process_cache_command 
******************************************************************************/
static void process_cache_command (flush_msg_t* Fmsg)
{
    core0_to_allcores(FLUSH_MSG_CACHEINV, Fmsg);
}

/******************************************************************************
* process_exit_command
******************************************************************************/
static int process_exit_command(void)
{
    int i;
    rx_mbox = NULL;  // One extra guard to indicate: do NOT print anymore

    for (i = 0; i < NUM_CORES - 1; i++)
    {
        void* eventHdl;
        do eventHdl = ocl_descriptorAlloc();
        while (eventHdl == NULL);

        ocl_descriptorPush(ocl_queues.exitQ, eventHdl);
    }

    service_exit(NULL);

    return 0;
}

char mbox_h2d[MBOX_SIZE] __attribute__((aligned(4096))) \
                         __attribute((section(".mbox_h2d")));

char mbox_d2h[MBOX_SIZE] __attribute__((aligned(4096))) \
                         __attribute((section(".mbox_d2h")));

/******************************************************************************
* initialize_host_mailboxes
******************************************************************************/
static int initialize_host_mailboxes(void)
{
    ((Msg_t *) print_msg_mem)->command = PRINT;
    if (DNUM != 0) return RETURN_OK;

    uint32_t mailboxallocsize = mpm_mailbox_get_alloc_size();
    if (!mailboxallocsize) return RETURN_FAIL;

    rx_mbox = rx_mbox_mem;
    if (!rx_mbox) return RETURN_FAIL;

    tx_mbox = tx_mbox_mem;
    if (!tx_mbox) return RETURN_FAIL;

    mpm_mailbox_config_t mbConfig;
    mbConfig.mem_start_addr   = (uint32_t)&mbox_h2d;
    mbConfig.mem_size         = MBOX_SIZE;
    mbConfig.max_payload_size = mbox_payload;

    int status;

    status = mpm_mailbox_create(rx_mbox, NULL,
                                MPM_MAILBOX_MEMORY_LOCATION_LOCAL, 
                                MPM_MAILBOX_DIRECTION_RECEIVE, 
                                &mbConfig);
    if (status == -1) return RETURN_FAIL;

    mbConfig.mem_start_addr = (uint32_t)&mbox_d2h;
    status = mpm_mailbox_create(tx_mbox, NULL,
                                MPM_MAILBOX_MEMORY_LOCATION_LOCAL, 
                                MPM_MAILBOX_DIRECTION_SEND, 
                                &mbConfig);
    if (status == -1) return RETURN_FAIL;

    status = mpm_mailbox_open(rx_mbox);
    if (status == -1) return RETURN_FAIL;

    status = mpm_mailbox_open(tx_mbox);
    if (status == -1) return RETURN_FAIL;

    return RETURN_OK;
}

/******************************************************************************
* initialize_memory
******************************************************************************/
static int initialize_memory(void)
{
    extern uint32_t nocache_phys_start;
    extern uint32_t nocache_virt_start;
    extern uint32_t nocache_size;

    uint32_t nc_phys = (uint32_t) &nocache_phys_start;
    uint32_t nc_virt = (uint32_t) &nocache_virt_start;
    uint32_t nc_size = (uint32_t) &nocache_size;

    int32_t mask = _disable_interrupts();

    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_setL2Size (CACHE_128KCACHE);

    enableCache (0x0C, 0x0C); // enable write through for msmc
    enableCache (0x80, 0xFF);
    disableCache(nc_virt >> 24, (nc_virt+nc_size-1) >> 24);

    nc_virt >>= 12;
    nc_phys >>= 12;
    nc_size = count_trailing_zeros(nc_size) - 1;

    set_MPAX(2, nc_virt, nc_size, nc_phys, DEFAULT_PERMISSION);

    if (DNUM == 0) 
    {
        /*---------------------------------------------------------------------
        * Use segment 2 for PRIVID 8, 9 and 10 (QMSS)
        *--------------------------------------------------------------------*/
        set_MSMC_MPAX(8,  2, nc_virt, nc_size, nc_phys, DEFAULT_PERMISSION);
#ifdef TI_66AK2H
        set_MSMC_MPAX(9,  2, nc_virt, nc_size, nc_phys, DEFAULT_PERMISSION);
#endif
        set_MSMC_MPAX(10, 2, nc_virt, nc_size, nc_phys, DEFAULT_PERMISSION);
    }

    _restore_interrupts(mask);

    return RETURN_OK;
}

/******************************************************************************
* reset_memory
******************************************************************************/
static void reset_memory(void)
{
    int32_t mask = _disable_interrupts();

    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_setL2Size (CACHE_0KCACHE);

    disableCache (0x80, 0xFF);
    reset_MPAX(2);
    if (DNUM == 0) 
    {
        reset_MSMC_MPAX(8,  2);
#ifdef TI_66AK2H
        reset_MSMC_MPAX(9,  2);
#endif
        reset_MSMC_MPAX(10, 2);
    }

    _restore_interrupts(mask);
}

/******************************************************************************
* incVec
******************************************************************************/
static int incVec(unsigned dims, unsigned *vec, unsigned *inc, unsigned *maxs)
{
    int overflow = 0;
    int i;

    for (i = 0; i < dims; ++i)
    {
        vec[i] += inc[i];

        if (vec[i] < maxs[i]) { overflow = 0; break;        }
        else                  { vec[i]   = 0; overflow = 1; }
    }

    return overflow;
}

/******************************************************************************
* mail_safely
******************************************************************************/
void mail_safely(uint8_t* msg, size_t msgSz, uint32_t msgId)
{
   int status;
   do 
   {
       sem_lock();
       status = mpm_mailbox_write(tx_mbox, msg, msgSz, msgId);
       sem_unlock();
   }
   while (status == MPM_MAILBOX_ERR_MAIL_BOX_FULL
          && tx_mbox != NULL && rx_mbox != NULL);
}


// Channel controller associated with specified dsp core
extern int32_t  *ti_sdo_fc_edmamgr_region2Instance;
#define DSP_CC(dspnum) ti_sdo_fc_edmamgr_region2Instance[dspnum] 

/******************************************************************************
* INITIALIZE_GDBSERVER() - The GDB server requires an edma channel. Core 0
* will provide the channel for GDB initialization. Once the GDB server is
* initialized start it running on all the DSPs.
******************************************************************************/
static int initialize_gdbserver()
{
#if defined(GDB_ENABLED)
   if (DNUM == 0)
   {
      gdb_channel = EdmaMgr_alloc(1);

      if (!gdb_channel || GDB_server_initGlob(4,DSP_CC(DNUM), gdb_channel) != 0)
	 return RETURN_FAIL;  
   }
    
   waitAtCoreBarrier();

   // Start up the gdb server on all the cores
   if (GDB_server_initLocal() != 0) return RETURN_FAIL;   
#endif

   return RETURN_OK;
}

