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
#ifdef TI_66AK2X
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
#ifdef TI_66AK2X
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
* Monitor configuration
*----------------------------------------------------------------------------*/
FAST_SHARED(int, n_cores) = -1;

#ifdef TI_66AK2X
int OCL_QMSS_HW_QUEUE_BASE_IDX             = 7300;
int OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM = 8192;
int OCL_QMSS_FIRST_MEMORY_REGION_IDX       = 16;
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
* Defines an area of bytes in L2, where the kernels will
* resolve the get_global_id type calls
******************************************************************************/
#pragma DATA_SECTION(kernel_config_l2, ".workgroup_config");
EXPORT kernel_config_t kernel_config_l2;

/******************************************************************************
* Define a kernel msg area where kernel attributes may be written from core 0
* before workgroups begin.
******************************************************************************/
PRIVATE(kernel_msg_t, kernel_attributes);

/******************************************************************************
* This initialization is dependent on the order of fields in kernel_config_t
******************************************************************************/
FAST_SHARED(kernel_config_t, task_config) = 
     { 1, {1,1,1}, {1,1,1}, {0,0,0}, {0,0,0}, 0, 0, 0, 0, 0 };

/******************************************************************************
* Initialization Routines
******************************************************************************/
static int initialize_memory        (void);
static int initialize_host_mailboxes(void);
static int initialize_configuration (void);
static int initialize_cores         (void);
static int initialize_qmss          (void);
static int initialize_bios_tasks    (void);
static int initialize_gdbserver     (void);

static void reset_memory            (void);

/******************************************************************************
* Bios Task Routines
******************************************************************************/
#define BIOS_TASK(name) void name(void)

BIOS_TASK(listen_host_task);
BIOS_TASK(service_wg_task);

/******************************************************************************
* QMSS Packet overlays 
******************************************************************************/
typedef struct 
{ 
    uint32_t cmd; 
    uint32_t id; 
    flush_msg_t Fmsg; 
} broadcast_pkt_t; 

typedef struct 
{ 
    uint32_t cmd; 
    uint32_t WG_gid_start[3]; 
    uint32_t WG_id; 
} workgroup_pkt_t;

typedef struct { 
    uint32_t     cmd; 
    uint32_t     WG_alloca_start; 
    uint32_t     WG_alloca_size; 
    uint32_t     L2_scratch_start;
    uint32_t     L2_scratch_size;
    kernel_msg_t kmsg; 
    flush_msg_t  fmsg;
} task_pkt_t;

/******************************************************************************
* External prototypes
******************************************************************************/
extern unsigned dsp_speed();
extern void*    dsp_rpc(void* p, void *stk_args, uint32_t stk_args_size);
extern int      tomp_initOpenMPforOpenCL(void);
extern void     tomp_exitOpenMPforOpenCL(void);

/******************************************************************************
* Workgroup dispatch routines
******************************************************************************/
static int  incVec(unsigned dims, unsigned *vec, unsigned *inc, unsigned *maxs);
void        mail_safely(uint8_t* msg, size_t msgSz, uint32_t msgId);

static void  process_kernel_local      (Msg_t* msg);
static void  process_kernel_distributed(Msg_t* msg);
static void  process_task_local        (Msg_t* msg);
static void  process_task_distributed  (Msg_t* msg);

static int   process_exit_command  (void);
static void  broadcast             (uint32_t flush_msg_id, flush_msg_t *f_msg);

static void  service_broadcast     (void* ptr);
static void  service_task          (void* ptr);
static void  service_workgroup     (void* ptr);

static inline void set_mpax_for_extended_memory(flush_msg_t* flush_msg);
static inline void restore_mpax_for_extended_memory(flush_msg_t* flush_msg);
static        void all_core_copy (void *dest, void *src, size_t size);

/******************************************************************************
* MASTER_THREAD : One core acts as a master.  This macro identifies that thread
******************************************************************************/
#define MASTER_THREAD (DNUM == 0)

/******************************************************************************
* main
******************************************************************************/
int main(void)
{
    if (!initialize_memory())           ERROR("Could not init Memory");
    if (!initialize_host_mailboxes())   ERROR("Could not init Mailboxes");
    if (!initialize_configuration())    ERROR("Could not init Configuration");
    if (!initialize_cores())            ERROR("Could not init Cores");
    if (!initialize_qmss())             ERROR("Could not init QMSS");
    if (!initialize_bios_tasks())       ERROR("Could not init BIOS tasks");
    if (tomp_initOpenMPforOpenCL() < 0) ERROR("Could not init OpenMP");
#ifdef TI_66AK2X
    if (!initialize_edmamgr())          ERROR("Could not init EdmaMgr"); 
#endif
    if (!initialize_gdbserver())        ERROR("Could not init GDB Server"); 

    waitAtCoreBarrier();

    BIOS_start();
}

/******************************************************************************
* listen_host_task - The primary task for the master DSP core.  It accepts 
*    message from the host and dispatches accordingly.
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
       /*----------------------------------------------------------------------
       * Busy loop waiting for messages from the host. Would prefer an 
       * interrupt based message and could sleep the dsps in the interim.
       *---------------------------------------------------------------------*/
       do mail_available = mpm_mailbox_query(rx_mbox); 
       while (!mail_available);

       Msg_t Msg;
       mpm_mailbox_read(rx_mbox, (uint8_t *)&Msg, &size, &trans_id);

       kernel_config_t* kcfg  = &Msg.u.k.config;

       /*----------------------------------------------------------------------
       * This two items of information from the message are overloading 
       * fields in the message type that are not populated coming from the 
       * host to the DSP.
       *---------------------------------------------------------------------*/
       int is_inorder_q = (kcfg->global_size[0]  == IN_ORDER_TASK_SIZE);
       int is_debugmode = (kcfg->WG_gid_start[0] == DEBUG_MODE_WG_GID_START);

       /*----------------------------------------------------------------------
       * Inspect the message type and dispatch wrok accordingly
       *---------------------------------------------------------------------*/
       switch (Msg.command)
       {
          case TASK: 
                      if (is_inorder_q || is_debugmode) process_task_local(&Msg);
                      else                              process_task_distributed(&Msg);
                      break;

          case NDRKERNEL: 
                      if (is_debugmode || n_cores == 1)
                          process_kernel_local(&Msg);
                      else
                          process_kernel_distributed(&Msg);
                      break;

          case CACHEINV: 
                      broadcast(FLUSH_MSG_CACHEINV, &Msg.u.k.flush); 
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
* process_task_local - Run local to the master core.  OpenMP constructs in the 
*    task may be used to control parallelism across cores.
******************************************************************************/
static void process_task_local(Msg_t* Msg) 
{
    kernel_config_t *kcfg       = &Msg->u.k.config;
    kernel_msg_t    *kmsg       = &Msg->u.k.kernel;
    flush_msg_t     *fmsg       = &Msg->u.k.flush;
    uint32_t         kernel_id  = kmsg->Kernel_id;

    void *           stk_args      = (void *) kmsg->args_on_stack_addr;
    uint32_t         stk_args_size = kmsg->args_on_stack_size;

    TRACE(ULM_OCL_IOT_OVERHEAD, kernel_id, 0);

    /*-----------------------------------------------------------------
    * Copy the static task config in L2, where the kernel wants it.
    * Also copy the variant portion from the host message.
    *----------------------------------------------------------------*/
    memcpy((void*)&kernel_config_l2, (void*)&task_config, sizeof(kernel_config_t));
    kernel_config_l2.WG_alloca_start  = kcfg->WG_alloca_start;
    kernel_config_l2.WG_alloca_size   = kcfg->WG_alloca_size;
    kernel_config_l2.L2_scratch_start = kcfg->L2_scratch_start;
    kernel_config_l2.L2_scratch_size  = kcfg->L2_scratch_size;

    /*---------------------------------------------------------------------
    * OpenCL will run local to this core, but for IOT case, OpenMP may
    * go wide across all cores, so we broadcast the mpax settings rather
    * than do a simple local mpax setup.
    *--------------------------------------------------------------------*/
    all_core_copy(&kernel_config_l2,  &kernel_config_l2, sizeof(kernel_config_t));
    broadcast(FLUSH_MSG_KERNEL_PROLOG, fmsg);

    /*--------------------------------------------------------------------
    * Run the task with potential OpenMP constructs 
    *--------------------------------------------------------------------*/
    clear_mpf();

    TRACE(ULM_OCL_IOT_KERNEL_START, kernel_id, 0);

    dsp_rpc(&kmsg->entry_point, stk_args, stk_args_size);

    TRACE(ULM_OCL_IOT_KERNEL_COMPLETE, kernel_id, 0);

    report_and_clear_mpf();

    mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), kernel_id);

    cacheInvAllL2(); 

    TRACE(ULM_OCL_IOT_CACHE_COHERENCE_COMPLETE, kernel_id, 0);

    broadcast(FLUSH_MSG_IOTASK_EPILOG, &Msg->u.k.flush);
    return;
}

/******************************************************************************
* process_task_distributed
******************************************************************************/
static void process_task_distributed(Msg_t* Msg) 
{
    uint32_t kernel_id = Msg->u.k.kernel.Kernel_id;

    TRACE(ULM_OCL_OOT_OVERHEAD, kernel_id, 0);

    task_pkt_t* pkt = (task_pkt_t*) ocl_descriptorAlloc();

    pkt->cmd              = Msg->command; 
    pkt->WG_alloca_start  = Msg->u.k.config.WG_alloca_start;
    pkt->WG_alloca_size   = Msg->u.k.config.WG_alloca_size ;
    pkt->L2_scratch_start = Msg->u.k.config.L2_scratch_start;
    pkt->L2_scratch_size  = Msg->u.k.config.L2_scratch_size;
    memcpy(&pkt->kmsg, &Msg->u.k.kernel, sizeof(kernel_msg_t));
    memcpy(&pkt->fmsg, &Msg->u.k.flush,  sizeof(flush_msg_t));

    ocl_descriptorPush(ocl_queues.wgQ, pkt);
}


/******************************************************************************
* process_kernel_local: Debug Mode: Run all NDRange workgroups on Core 0.
******************************************************************************/
static void process_kernel_local(Msg_t* msg)
{
    int               done;
    uint32_t          workgroup     = 0;
    uint32_t          WGid[3]       = {0,0,0};
    kernel_config_t * kcfg          = &msg->u.k.config;
    kernel_msg_t    * kmsg          = &msg->u.k.kernel;
    flush_msg_t     * fmsg          = &msg->u.k.flush;
    int               kernel_id     = kmsg->Kernel_id;
    void *            stk_args      = (void *) kmsg->args_on_stack_addr;
    uint32_t          stk_args_size = kmsg->args_on_stack_size;

    TRACE(ULM_OCL_NDR_OVERHEAD, kernel_id, 0);

    /*-------------------------------------------------------------------------
    * Set mpax only for core 0.
    *------------------------------------------------------------------------*/
    set_mpax_for_extended_memory(fmsg);

    memcpy((void*)&kernel_config_l2, kcfg, sizeof(kernel_config_t));

    /*-------------------------------------------------------------------------
    * Iteratively run all workgroups locally 
    *------------------------------------------------------------------------*/
    do
    {
        /*---------------------------------------------------------------------
        * update the 4 variant fields in kernel_config_l2
        *--------------------------------------------------------------------*/
        kernel_config_l2.WG_gid_start[0] = kcfg->global_offset[0] + WGid[0];
        kernel_config_l2.WG_gid_start[1] = kcfg->global_offset[1] + WGid[1];
        kernel_config_l2.WG_gid_start[2] = kcfg->global_offset[2] + WGid[2];
        kernel_config_l2.WG_id           = workgroup++;

        clear_mpf();

        TRACE(ULM_OCL_NDR_KERNEL_START, kernel_id, kcfg->WG_id);

        dsp_rpc(&kmsg->entry_point, stk_args, stk_args_size);

        TRACE(ULM_OCL_NDR_KERNEL_COMPLETE, kernel_id, kcfg->WG_id);

        report_and_clear_mpf();

        done = incVec(kcfg->num_dims, WGid, &kcfg->local_size[0], &kcfg->global_size[0]);
    } while (!done);

    mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), kernel_id);

    if (fmsg->need_cache_op) cacheInvAllL2(); 

    TRACE(ULM_OCL_NDR_CACHE_COHERENCE_COMPLETE, kernel_id, 0);

    restore_mpax_for_extended_memory(fmsg);
}


/******************************************************************************
* process_kernel_distributed
******************************************************************************/
static void process_kernel_distributed(Msg_t* msg)
{
    int               done;
    uint32_t          workgroup = 0;
    uint32_t          WGid[3]   = {0,0,0};
    kernel_config_t * kcfg      = &msg->u.k.config;
    kernel_msg_t    * kmsg      = &msg->u.k.kernel;
    flush_msg_t     * fmsg      = &msg->u.k.flush;
    int               kernel_id = kmsg->Kernel_id;

    TRACE(ULM_OCL_NDR_OVERHEAD, kernel_id, 0);

    /*-------------------------------------------------------------------------
    * Copy over the kernel attributes that a fixed across workgroups.  
    * Only the workgroup variant information is passed in the queues.
    *------------------------------------------------------------------------*/
    all_core_copy(&kernel_attributes, kmsg, sizeof(kernel_msg_t));
    all_core_copy(&kernel_config_l2,  kcfg, sizeof(kernel_config_t));

    /*-------------------------------------------------------------------------
    * set MPAX settings for all cores
    *------------------------------------------------------------------------*/
    broadcast(FLUSH_MSG_KERNEL_PROLOG, fmsg);

    /*---------------------------------------------------------
    * Iterate over each Work Group
    *--------------------------------------------------------*/
    do 
    {
        workgroup_pkt_t* pkt = ocl_descriptorAlloc();

        pkt->cmd             = msg->command;
        pkt->WG_gid_start[0] = kcfg->global_offset[0] + WGid[0];
        pkt->WG_gid_start[1] = kcfg->global_offset[1] + WGid[1];
        pkt->WG_gid_start[2] = kcfg->global_offset[2] + WGid[2];
        pkt->WG_id           = workgroup++;

        ocl_descriptorPush(ocl_queues.wgQ, pkt);

        done = incVec(kcfg->num_dims, WGid, &kcfg->local_size[0], &kcfg->global_size[0]);

    } while (!done);

    /*-------------------------------------------------------------------------
    * reset MPAX settings for all cores and handle cache coherency operations
    *------------------------------------------------------------------------*/
    broadcast(kernel_id, fmsg);
}

/******************************************************************************
* process_exit_command
******************************************************************************/
static int process_exit_command(void)
{
    int i;
    rx_mbox = NULL;  // One extra guard to indicate: do NOT print anymore

    for (i = 0; i < n_cores - 1; i++)
    {
        void* ptr = ocl_descriptorAlloc();
        ocl_descriptorPush(ocl_queues.exitQ, ptr);
    }

    service_exit(NULL);
    return 0;
}
    
/******************************************************************************
* send a command to all cores through a qmss Q and wait for said command to 
* complete on at least core 0. 
* Note: serviced by each core with broad_handler()
******************************************************************************/
static void broadcast(uint32_t flush_msg_id, flush_msg_t *f_msg)
{    
    int      core;
    uint32_t cache_op  = f_msg->need_cache_op;
    uint32_t num_mpaxs = f_msg->num_mpaxs;

    if (flush_msg_id == FLUSH_MSG_CACHEINV)
        { cache_op  = 1; num_mpaxs = 0; }

    for (core = 0; core < n_cores; core++)
    {
        broadcast_pkt_t* pkt = (broadcast_pkt_t*) ocl_descriptorAlloc();

        pkt->cmd                = BROADCAST;
        pkt->id                 = flush_msg_id;
        pkt->Fmsg.need_cache_op = cache_op;
        pkt->Fmsg.num_mpaxs     = num_mpaxs;

        if (num_mpaxs) 
            memcpy(&pkt->Fmsg.mpax_settings, 
                   (void*)&f_msg->mpax_settings, 
                   sizeof(f_msg->mpax_settings));

        ocl_descriptorPush(ocl_queues.wgQ, pkt);
    }

    /*-------------------------------------------------------------------------
    * Wait for broadcast to complete before master thread continues
    *------------------------------------------------------------------------*/
    Semaphore_pend(higherSem, BIOS_WAIT_FOREVER);
}


/******************************************************************************
* service_wg_task - The task that runs on all DSP cores.  It is listening for 
*   QMSS queue entries to service, and until exit condition will indirectly 
*   call queue_handler.
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
* queue_handler
*   This function is called by the Queue manager system to service an 
*   individual work group for a kernel.
******************************************************************************/
EVENT_HANDLER(queue_handler)
{
    uint32_t *cmd = (uint32_t*) eventHdl;

    switch (*cmd)
    {
        case TASK:
            service_task(eventHdl);
            break;

        case NDRKERNEL:
            service_workgroup(eventHdl);
            break;

        case BROADCAST: 
            service_broadcast (eventHdl);
            break;
    }
    ocl_descriptorFree(eventHdl);
}

/******************************************************************************
* service_task
******************************************************************************/
void service_task(void *ptr)
{
    task_pkt_t*      pkt           = (task_pkt_t*) ptr;
    kernel_msg_t*    kmsg          = &pkt->kmsg;
    flush_msg_t*     fmsg          = &pkt->fmsg;
    uint32_t         kId           = kmsg->Kernel_id;
    void *           stk_args      = (void *) kmsg->args_on_stack_addr;
    uint32_t         stk_args_size = kmsg->args_on_stack_size;

    /*-------------------------------------------------------------------------
    * The task invariant get_xxx portions of config have not been written in
    * the descriptor, so we copy a fixed version of config and then update the
    * two fields that are variant for a task.
    *------------------------------------------------------------------------*/
    memcpy((void*)&kernel_config_l2, (void*)&task_config, sizeof(kernel_config_t));
    kernel_config_l2.WG_alloca_start  = pkt->WG_alloca_start;
    kernel_config_l2.WG_alloca_size   = pkt->WG_alloca_size;
    kernel_config_l2.L2_scratch_start = pkt->L2_scratch_start;
    kernel_config_l2.L2_scratch_size  = pkt->L2_scratch_size;

    set_mpax_for_extended_memory(fmsg);

    TRACE(ULM_OCL_OOT_KERNEL_START, kId, 0);

    clear_mpf();

    dsp_rpc(&kmsg->entry_point, stk_args, stk_args_size);

    TRACE(ULM_OCL_OOT_KERNEL_COMPLETE, kId, 0);

    report_and_clear_mpf();

    mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), kId);

    cacheInvAllL2(); 

    TRACE(ULM_OCL_OOT_CACHE_COHERENCE_COMPLETE, kId, 0);

    restore_mpax_for_extended_memory(fmsg);
}

/******************************************************************************
* service_workgroup
******************************************************************************/
void service_workgroup(void *ptr)
{
    workgroup_pkt_t* pkt = (workgroup_pkt_t*) ptr;

    kernel_config_l2.WG_gid_start[0] = pkt->WG_gid_start[0];
    kernel_config_l2.WG_gid_start[1] = pkt->WG_gid_start[1];
    kernel_config_l2.WG_gid_start[2] = pkt->WG_gid_start[2];
    kernel_config_l2.WG_id           = pkt->WG_id;

    uint32_t      kId  = kernel_attributes.Kernel_id;
    uint32_t     wgId  = kernel_config_l2.WG_id;

    printMsg = (Msg_t *) print_msg_mem;  // enable printf before ocl kernel

    TRACE(ULM_OCL_NDR_KERNEL_START, kId, wgId);

    clear_mpf();

    dsp_rpc(&kernel_attributes.entry_point, 
            (void*)kernel_attributes.args_on_stack_addr, 
            kernel_attributes.args_on_stack_size);

    TRACE(ULM_OCL_NDR_KERNEL_COMPLETE, kId, wgId);

    report_and_clear_mpf();

    printMsg = NULL;  // disable printf after ocl kernel
}


/******************************************************************************
* service_broadcast
*   This function is called for a broadcast command that is to run once 
*   for every core.
******************************************************************************/
static void service_broadcast(void* ptr)
{
    broadcast_pkt_t*   pkt = (broadcast_pkt_t *) ptr;
    flush_msg_t* flush_msg = &pkt->Fmsg;
    uint32_t     num_mpaxs = flush_msg->num_mpaxs;
    uint32_t     cache_op  = flush_msg->need_cache_op;

    /*-------------------------------------------------------------------------
    * The barrier ensures that each core picks up a single flush event from
    * the queue and waits for remaining cores. Without the barrier, a core
    * can incorreclty pick up and execute multiple flush events, resulting in
    * undefined behavior.
    *------------------------------------------------------------------------*/
    switch(pkt->id)
    {
        case FLUSH_MSG_KERNEL_PROLOG:
            set_mpax_for_extended_memory(flush_msg);
            CACHE_invL1d(&kernel_config_l2,  sizeof(kernel_config_l2),  CACHE_WAIT); 
            CACHE_invL1d(&kernel_attributes, sizeof(kernel_attributes), CACHE_WAIT); 
            waitAtCoreBarrier(); 
            break;

        case FLUSH_MSG_IOTASK_EPILOG:
            restore_mpax_for_extended_memory(flush_msg);
            waitAtCoreBarrier(); 
            break;

        case FLUSH_MSG_CACHEINV:
        default:
 
            if (cache_op)      cacheInvAllL2(); 
            if (num_mpaxs > 0) reset_kernel_MPAXs(num_mpaxs);
            waitAtCoreBarrier(); 

            /*-----------------------------------------------------------------
            *  Send ready message after core barrier to ensure all cores
            *  complete their workgroup before the message is sent by core 0
            *----------------------------------------------------------------*/
            if (MASTER_THREAD)
                mail_safely((uint8_t*)&readyMsg, sizeof(readyMsg), pkt->id);
 
            TRACE(ULM_OCL_NDR_CACHE_COHERENCE_COMPLETE, pkt->id, 0);
            break;
    }

    if (MASTER_THREAD) Semaphore_post(higherSem);
}



/******************************************************************************
* service_exit
*   This function is called by the Queue manager system when it receives an
*   exit command from the host.
******************************************************************************/
EVENT_HANDLER(service_exit) 
{ 
    TRACE(ULM_OCL_EXIT, 0, 0);

#ifdef TI_66AK2X
    free_edma_channel_pool();
    waitAtCoreBarrier();
    if (MASTER_THREAD)
    {
        tomp_exitOpenMPforOpenCL();
        ocl_exitGlobalQMSS();
    }
#endif
#if defined(GDB_ENABLED)
    if (MASTER_THREAD && gdb_channel != NULL) EdmaMgr_free(gdb_channel);
#endif
    /* Send final exiting acknowledgement back to host */
    if (MASTER_THREAD)
    {
        rx_mbox = rx_mbox_mem;
        mail_safely((uint8_t*)&exitMsg, sizeof(Msg_t), 42);
        rx_mbox = NULL;
    }
    cacheWbInvAllL2();
    reset_memory();
    exit(0); 
}


/******************************************************************************
* Configure the mpax registers for any extended memory arguments to the kernel
******************************************************************************/
static inline void set_mpax_for_extended_memory(flush_msg_t* flush_msg)
{
    uint32_t num_mpaxs = flush_msg->num_mpaxs;
    if (num_mpaxs > 0) set_kernel_MPAXs(num_mpaxs, flush_msg->mpax_settings);
    printMsg = (Msg_t *) print_msg_mem;
}

/******************************************************************************
* Reset the mpax registers after a kernel.  
******************************************************************************/
static inline void restore_mpax_for_extended_memory(flush_msg_t* flush_msg)
{
    uint32_t num_mpaxs = flush_msg->num_mpaxs;
    printMsg = NULL;
    if (num_mpaxs > 0) reset_kernel_MPAXs(num_mpaxs);
}

/******************************************************************************
* all_core_copy: Copy a memory block to a location in each cores's L2 
******************************************************************************/
static void all_core_copy(void *dest, void *src, size_t size)
{
    int i;

    // assert(dest) is an L2 location
    // assert region dest to dest+size is not valid in core i's L1D cache
    //   or will not be by the time core i access the region
   
    for (i = 0; i < n_cores; i++)
    {
        void *gdest = (void*) ((0x10 + i) << 24 | (uint32_t)dest);
        memcpy(gdest, src, size);
        CACHE_wbL1d(gdest, size, CACHE_NOWAIT);
    }
    CACHE_wbL1dWait();

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

    /***  BIOS is configuring the default cache sizes, see Platform.xdc ***/

    enableCache (0x0C, 0x0C); // enable write through for msmc
    enableCache (0x80, 0xFF);
    disableCache(nc_virt >> 24, (nc_virt+nc_size-1) >> 24);

    nc_virt >>= 12;
    nc_phys >>= 12;
    nc_size = count_trailing_zeros(nc_size) - 1;

    set_MPAX(2, nc_virt, nc_size, nc_phys, DEFAULT_PERMISSION);

    if (MASTER_THREAD) 
    {
        /*---------------------------------------------------------------------
        * Use segment 2 for PRIVID 8, 9 and 10 (QMSS)
        *--------------------------------------------------------------------*/
        set_MSMC_MPAX(8,  2, nc_virt, nc_size, nc_phys, DEFAULT_PERMISSION);
#ifdef TI_66AK2X
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
    CACHE_getL1DSize();
    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_getL1PSize();
    CACHE_setL2Size (CACHE_0KCACHE);
    CACHE_getL2Size ();

    disableCache (0x80, 0xFF);
    reset_MPAX(2);
    if (MASTER_THREAD) 
    {
        reset_MSMC_MPAX(8,  2);
#ifdef TI_66AK2X
        reset_MSMC_MPAX(9,  2);
#endif
        reset_MSMC_MPAX(10, 2);
    }

    _restore_interrupts(mask);
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
    if (!MASTER_THREAD) return RETURN_OK;

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

static int initialize_configuration(void)
{
#ifdef TI_66AK2X
    if (MASTER_THREAD)
    {
        while(!mpm_mailbox_query(rx_mbox))
            continue;

        Msg_t Msg;
        uint32_t size, trans_id;
        mpm_mailbox_read(rx_mbox, (uint8_t *)&Msg, &size, &trans_id);

        if(Msg.command != CONFIGURE_MONITOR)
            return RETURN_FAIL;

        if(Msg.u.configure_monitor.ocl_qmss_hw_queue_base_idx > 0)
        {
            OCL_QMSS_HW_QUEUE_BASE_IDX =
                Msg.u.configure_monitor.ocl_qmss_hw_queue_base_idx;

            OCL_QMSS_FIRST_DESC_IDX_IN_LINKING_RAM =
                Msg.u.configure_monitor.ocl_qmss_first_desc_idx_in_linking_ram;

            OCL_QMSS_FIRST_MEMORY_REGION_IDX =
                Msg.u.configure_monitor.ocl_qmss_first_memory_region_idx;
        }

        /* update n_cores last to trigger other cores' monitors to continue */
        *(int volatile *)&n_cores = Msg.u.configure_monitor.n_cores;
    }
    else
    {
        /* other cores wait until configuration is complete */
        while(*(int volatile *)&n_cores == -1)
            continue;
    }

#else
    n_cores = 8;

#endif

    return RETURN_OK;
}

#ifdef TI_66AK2X
static int initialize_cores() { return RETURN_OK; }
#else

/******************************************************************************
* cycle_delay - busy wait the dsp for a number of clock cycles
******************************************************************************/
static void cycle_delay(uint32_t cycles)
{
    uint32_t start_val  = CSL_chipReadTSCL();
    while ((CSL_chipReadTSCL() - start_val) < cycles);
}

/******************************************************************************
* Setup the cores for execution
******************************************************************************/
static int initialize_cores()
{
    /*---------------------------------------------------------------------
    * should be abstracted to high level functions through platform library
    *--------------------------------------------------------------------*/
#define DEVICE_REG32_W(x,y)   *(volatile uint32_t *)(x)=(y)
#define IPCGR(x)            (0x02620240 + x*4)

    /*---------------------------------------------------------------------
    * ipc interrupt other cores
    *--------------------------------------------------------------------*/
    int core;
    if (MASTER_THREAD)
        for (core = 1; core < n_cores; core++)
        {
            DEVICE_REG32_W(IPCGR(core), 1);
            cycle_delay(1000000);
        }

    return RETURN_OK;
}
#endif

/******************************************************************************
* initialize_qmss
* Keystone 2 needs to get QMSS resources allocated by RM on ARM
******************************************************************************/
static int initialize_qmss(void)
{
    if (MASTER_THREAD && !ocl_initGlobalQMSS())   return RETURN_FAIL; 
    waitAtCoreBarrier();
    if (!ocl_initLocalQMSS())                 return RETURN_FAIL; 
    waitAtCoreBarrier();

    if (MASTER_THREAD && !initClockGlobal()) return RETURN_FAIL; 
    waitAtCoreBarrier();
    if (!initClockLocal())               return RETURN_FAIL; 
    waitAtCoreBarrier();

    return RETURN_OK;
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

    if (!MASTER_THREAD) return RETURN_OK;

    Task_Params_init(&taskParams);
    taskParams.priority  = 15; // HIGH_PRIORITY;
    taskParams.stackSize = LISTEN_STACK_SIZE;
    taskParams.stack = (xdc_Ptr)makeAddressGlobal(DNUM, (uint32_t)lstack);
    taskHandle = Task_create((Task_FuncPtr)listen_host_task, &taskParams, NULL);
    if (taskHandle == NULL) return RETURN_FAIL;

    return RETURN_OK;
}

/******************************************************************************
* INITIALIZE_GDBSERVER() - The GDB server requires an edma channel. Core 0
* will provide the channel for GDB initialization. Once the GDB server is
* initialized start it running on all the DSPs.
******************************************************************************/
// Channel controller associated with specified dsp core
extern int32_t  *ti_sdo_fc_edmamgr_region2Instance;
#define DSP_CC(dspnum) ti_sdo_fc_edmamgr_region2Instance[dspnum] 

static int initialize_gdbserver()
{
#if defined(GDB_ENABLED)
   if (MASTER_THREAD)
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


/******************************************************************************
* Low level routines within the printf stack
******************************************************************************/
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
