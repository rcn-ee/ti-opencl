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

/* this define must precede inclusion of any xdc header file */
#define Registry_CURDESC OCL__Desc
#define MODULE_NAME "OCLMonitor"


/* xdctools header files */
#include <xdc/std.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Registry.h>
#include <xdc/runtime/System.h>
#if defined(_SYS_BIOS)
#include <xdc/runtime/IHeap.h>
#endif
 
/*-----------------------------------------------------------------------------
* IPC header files
*----------------------------------------------------------------------------*/
#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/MultiProc.h>
#if defined(_SYS_BIOS)
#include <ti/ipc/SharedRegion.h>
#include <ti/opencl/configuration_dsp.h>
#endif
#if defined(DEVICE_AM572x)
#include <ti/pm/IpcPower.h>
#endif

/*-----------------------------------------------------------------------------
* BIOS header files
*----------------------------------------------------------------------------*/
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>

/*-----------------------------------------------------------------------------
* C standard library
*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

/*-----------------------------------------------------------------------------
* Application headers
*----------------------------------------------------------------------------*/
#include "monitor.h"
#include "util.h"
#include "edma.h"
#include "trace.h"
#include "tal/mbox_msgq_shared.h"

#if defined(ULM_ENABLED)
#include "tiulm.h"
#endif

#if defined(GDB_ENABLED)
#include "GDB_server.h"
#endif


typedef struct 
{
    MessageQ_Handle   dspQue;  // DSP creates,  Host writes, DSP  reads
    MessageQ_QueueId  hostQue; // Host creates, DSP  writes, Host reads
} OCL_MessageQueues;


DDR (Registry_Desc, Registry_CURDESC);

PRIVATE (bool,              enable_printf)       = false;
PRIVATE (bool,              edmamgr_initialized) = false;
PRIVATE (OCL_MessageQueues,  ocl_queues);
PRIVATE (int,               n_cores);

extern Semaphore_Handle runOmpSem;

/******************************************************************************
* Defines a fixed area of 64 bytes at the start of L2, where the kernels will
* resolve the get_global_id type calls
******************************************************************************/
#pragma DATA_SECTION(kernel_config_l2, ".workgroup_config");
EXPORT kernel_config_t kernel_config_l2;

/******************************************************************************
* Initialization Routines
******************************************************************************/
static void initialize_gdbserver     ();
#if defined(DEVICE_AM572x) && !defined(_SYS_BIOS)
static void initialize_ipcpower_callbacks();
static void enable_ipcpower_suspend();
static void disable_ipcpower_suspend();
#endif
static void flush_buffers(flush_msg_t *Msg);

/******************************************************************************
* External prototypes
******************************************************************************/
extern void*    dsp_rpc(void* p, void *more_args, uint32_t more_args_size);

/******************************************************************************
* Workgroup dispatch routines
******************************************************************************/
static int  incVec(unsigned dims, unsigned *vec, unsigned *inc, unsigned *maxs);
static void respond_to_host(ocl_msgq_message_t *msgq_pkt, uint32_t msgId);

static void process_kernel_command(ocl_msgq_message_t* msgq_pkt);
static void process_task_command  (ocl_msgq_message_t* msgq_msg);
static void process_cache_command (int pkt_id, ocl_msgq_message_t *msgq_pkt);
static void process_exit_command  (ocl_msgq_message_t* msgq_msg);
static void process_setup_debug_command(ocl_msgq_message_t* msgq_pkt);
static void service_workgroup     (Msg_t* msg);
static bool setup_ndr_chunks      (int dims, uint32_t* limits, uint32_t* offsets,
                                   uint32_t *gsz, uint32_t* lsz);
static void process_configuration_message(ocl_msgq_message_t* msgq_pkt);


/* BIOS_TASKS */
/******************************************************************************
* Bios Task and helper routines
******************************************************************************/
static void ocl_main(UArg arg0, UArg arg1);
#ifndef _SYS_BIOS  // TODO: disable OpenMP for now
static void ocl_service_omp();
#endif
static bool create_mqueue(void);
static void ocl_monitor();

extern tomp_initOpenMPforOpenCLPerApp(int master_core, int num_cores);
extern tomp_exitOpenMPforOpenCL(void);
extern void tomp_dispatch_once(void);
extern void tomp_dispatch_finish(void);
extern bool tomp_dispatch_is_finished(void);

#ifdef _SYS_BIOS
// Prevent RTS versions of malloc etc. from getting pulled into link
void _minit(void) { }
#endif

/*******************************************************************************
* MASTER_CORE : One core acts as a master.  This macro identifies that thread
*******************************************************************************/
#define MASTER_CORE (DNUM == 0 || n_cores == 1)

#define LISTEN_STACK_SIZE 0x2800
#define SERVICE_STACK_SIZE 0x10000
PRIVATE_1D(char, lstack, LISTEN_STACK_SIZE);

/******************************************************************************
* main
******************************************************************************/
#if !defined(_SYS_BIOS)
int main(int argc, char* argv[])
#else
int rtos_init_ocl_dsp_monitor(int argc, char* argv[])
#endif
{
    edmamgr_initialized = false;

    /* register with xdc.runtime to get a diags mask */
    Registry_Result result = Registry_addModule(&Registry_CURDESC, MODULE_NAME);
    assert(result == Registry_SUCCESS);

    /* enable ENTRY/EXIT/INFO log events */
    Diags_setMask(MODULE_NAME"-EXF");

    /* Setup non-cacheable memory, etc... */
    initialize_memory();

#ifdef _SYS_BIOS
    /*------------------------------------------------------------------------
    * SYSBIOS mode: Ipc_start() needs to be called explicitly.
    * SYNC_PAIR protocol: need to attach peer core explicitly. Also gives
    *     the host freedom to choose involved DSPs, compared to SYNC_ALL.
    *------------------------------------------------------------------------*/
    int status = Ipc_start();
    UInt16 remoteProcId = MultiProc_getId("HOST");
    do {
        status = Ipc_attach(remoteProcId);
    } while ((status < 0) && (status == Ipc_E_NOTREADY));
#endif

#if !defined(DEVICE_AM572x)
    initialize_gdbserver();
#endif
#if defined(DEVICE_AM572x) && !defined(_SYS_BIOS)
    initialize_ipcpower_callbacks();
#endif

    Task_Params     taskParams;
    Error_Block     eb;
    Log_print0(Diags_ENTRY, "--> main:");

    /* Create main thread (interrupts not enabled in main on BIOS) */
    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.instance->name = "ocl_main";
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
#if !defined(_SYS_BIOS)
    taskParams.priority = 7; // LOWER_PRIORITY
#else
    taskParams.priority = ti_opencl_get_OCL_monitor_priority();
#endif
    taskParams.stackSize = LISTEN_STACK_SIZE;
    taskParams.stack = (xdc_Ptr)lstack; // L2 private
    Task_create(ocl_main, &taskParams, &eb);
    if (Error_check(&eb)) {
        System_abort("main: failed to create ocl_main thread");
    }

    #ifndef _SYS_BIOS
    /* Create a task to service OpenMP kernels */
    extern uint32_t service_stack_start;
    uint32_t stack_start = (uint32_t) &service_stack_start;

    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.instance->name = "ocl_service_omp";
    taskParams.priority = 8; // HIGHER_PRIORITY
    taskParams.stackSize = SERVICE_STACK_SIZE;
    taskParams.stack = (xdc_Ptr) (stack_start + DNUM * SERVICE_STACK_SIZE);
    Task_create(ocl_service_omp, &taskParams, &eb);
    if (Error_check(&eb))
    {
        System_abort("main: failed to create ocl_service_omp thread");
    }

    Log_print0(Diags_ENTRY | Diags_INFO, "main: created omp task");
    #endif

#if !defined(_SYS_BIOS)
    /* Start scheduler, this never returns */
    BIOS_start();

    /* Should never get here */
    Log_print0(Diags_EXIT, "<-- main:");
#endif
    return (0);
}

/******************************************************************************
* ocl_main
******************************************************************************/
void ocl_main(UArg arg0, UArg arg1)
{
    Log_print0(Diags_ENTRY | Diags_INFO, "--> ocl_main:");

    /* create the dsp queue */
    if (!create_mqueue())
        System_abort("main: create_mqueue() failed");

#if !defined(DEVICE_AM572x)
    initialize_gdbserver();
#else
    #if !defined(_SYS_BIOS)
    // On AM57x, indicate that heaps must be initialized. It's ok to set this on both
    // DSP cores because there is a barrier hit before the heap is initialized.
    extern int need_mem_init;
    need_mem_init = 1;
    #endif
#endif

    /* printfs are enabled ony for the duration of an OpenCL kernel */
    enable_printf = false;

    /* OpenCL monitor loop - does not return */
    ocl_monitor();

    /* delete the message queue */
    int status = MessageQ_delete(&ocl_queues.dspQue);

    if (status < 0)
    {
        Log_error1("Server_finish: error=0x%x", (IArg)status);
    }

    return;
}


/******************************************************************************
* ocl_monitor
******************************************************************************/
void ocl_monitor()
{
    Log_print0(Diags_ENTRY | Diags_INFO, "--> ocl_monitor:");

    while (true)
    {
                
        Log_print0(Diags_INFO, "ocl_monitor: Waiting for message");

        ocl_msgq_message_t *msgq_pkt;

        MessageQ_get(ocl_queues.dspQue, (MessageQ_Msg *)&msgq_pkt, 
                     MessageQ_FOREVER);

        /* Get the host queue id from the message & save it */
        MessageQ_QueueId  hostQueId = MessageQ_getReplyQueue(msgq_pkt);
        if (ocl_queues.hostQue == MessageQ_INVALIDMESSAGEQ)
            ocl_queues.hostQue = hostQueId;

        enable_printf = true;

        /* Get a pointer to the OpenCL payload in the message */
        Msg_t *ocl_msg =  &(msgq_pkt->message);

        switch (ocl_msg->command)
        {
            case TASK: 
                Log_print0(Diags_INFO, "TASK\n");
                process_task_command(msgq_pkt);
                break;

            case NDRKERNEL: 
                Log_print0(Diags_INFO, "NDRKERNEL\n");
                process_kernel_command(msgq_pkt);
                process_cache_command(ocl_msg->u.k.kernel.Kernel_id, msgq_pkt);
                TRACE(ULM_OCL_NDR_CACHE_COHERENCE_COMPLETE,
                      ocl_msg->u.k.kernel.Kernel_id, 0);
                break;

            case CACHEINV: 
                Log_print0(Diags_INFO, "CACHEINV\n");
                process_cache_command(-1, msgq_pkt); 
                break; 

            case EXIT:     
                Log_print0(Diags_INFO, "EXIT\n");
                TRACE(ULM_OCL_EXIT, 0, 0);
                process_exit_command(msgq_pkt);
                break;

            case SETUP_DEBUG:
                Log_print0(Diags_INFO, "SETUP_DEBUG\n");
                process_setup_debug_command(msgq_pkt);
                break;

            case FREQUENCY:
                Log_print0(Diags_INFO, "FREQUENCY\n");
                respond_to_host(msgq_pkt, dsp_speed());
                break;

            case CONFIGURE_MONITOR:
                Log_print0(Diags_INFO, "CONFIGURE_MONITOR\n");
                process_configuration_message(msgq_pkt);
                break;

            default:
                Log_print1(Diags_INFO, "OTHER (%d)\n", ocl_msg->command);
                break;
        }

        enable_printf = false;

    } /* while (true) */
}


PRIVATE(ocl_msgq_message_t*, omp_msgq_pkt) = NULL;

/******************************************************************************
* process_task_command
******************************************************************************/
static void process_task_command(ocl_msgq_message_t* msgq_pkt) 
{
    Msg_t* Msg = &(msgq_pkt->message);

    kernel_config_t * kcfg  = &Msg->u.k.config;
    uint32_t  kernel_id = Msg->u.k.kernel.Kernel_id;
    int      is_inorder = (kcfg->global_size[0] == IN_ORDER_TASK_SIZE);

    /*---------------------------------------------------------
    * Copy the configuration in L2, where the kernel wants it
    *--------------------------------------------------------*/
    kernel_config_l2.num_dims          = 1;
    kernel_config_l2.global_size[0]    = 1;
    kernel_config_l2.global_size[1]    = 1;
    kernel_config_l2.global_size[2]    = 1;
    kernel_config_l2.local_size[0]     = 1;
    kernel_config_l2.local_size[1]     = 1;
    kernel_config_l2.local_size[2]     = 1;
    kernel_config_l2.global_offset[0]  = 0;
    kernel_config_l2.global_offset[1]  = 0;
    kernel_config_l2.global_offset[2]  = 0;
    kernel_config_l2.WG_gid_start[0]   = 0;
    kernel_config_l2.WG_gid_start[1]   = 0;
    kernel_config_l2.WG_gid_start[2]   = 0;
    kernel_config_l2.WG_id            = 0;
    kernel_config_l2.WG_alloca_start  = kcfg->WG_alloca_start;
    kernel_config_l2.WG_alloca_size   = kcfg->WG_alloca_size;
    kernel_config_l2.L2_scratch_start = kcfg->L2_scratch_start;
    kernel_config_l2.L2_scratch_size  = kcfg->L2_scratch_size;

    /*--------------------------------------------------------------------
    * Run the Task 
    *--------------------------------------------------------------------*/
    uint32_t more_args_size = Msg->u.k.kernel.args_on_stack_size;
    void *   more_args      = (void *) Msg->u.k.kernel.args_on_stack_addr;

#ifndef _SYS_BIOS
    if (is_inorder)
    {
       omp_msgq_pkt = msgq_pkt;
       Semaphore_post(runOmpSem);
       /* in order task was completed by ocl_service_omp task*/
       if (omp_msgq_pkt != NULL)
          /* Error */; 
    }
    else
#endif
    {
       TRACE(ULM_OCL_OOT_KERNEL_START, kernel_id, 0);
       dsp_rpc(&((kernel_msg_t *)&Msg->u.k.kernel)->entry_point,
               more_args, more_args_size);
       TRACE(ULM_OCL_OOT_KERNEL_COMPLETE, kernel_id, 0);

       respond_to_host(msgq_pkt, kernel_id);

       flush_msg_t*  flushMsgPtr  = &Msg->u.k.flush;
       flush_buffers(flushMsgPtr);
       TRACE(ULM_OCL_OOT_CACHE_COHERENCE_COMPLETE, kernel_id, 0);
    }

    return;
}

#ifndef _SYS_BIOS
/******************************************************************************
* ocl_service_omp - This is it's own task to switch the stack to DDR.
******************************************************************************/
void ocl_service_omp()
{
    Log_print0(Diags_ENTRY | Diags_INFO, "--> ocl_service_omp:");

    while (true)
    {
       Semaphore_pend(runOmpSem, BIOS_WAIT_FOREVER);

       if (omp_msgq_pkt != NULL)
       {
          /*-------------------------------------------------------------------
          * Run the in order Task.  OpenMP kernels run here. 
          *-------------------------------------------------------------------*/
          Msg_t* Msg = &(omp_msgq_pkt->message);
          uint32_t kernel_id = Msg->u.k.kernel.Kernel_id;
          uint32_t more_args_size = Msg->u.k.kernel.args_on_stack_size;
          void* more_args = (void *) Msg->u.k.kernel.args_on_stack_addr;

          if (MASTER_CORE)
          {
             TRACE(ULM_OCL_IOT_KERNEL_START, kernel_id, 0);
             dsp_rpc(&((kernel_msg_t *)&Msg->u.k.kernel)->entry_point,
                     more_args, more_args_size);
             TRACE(ULM_OCL_IOT_KERNEL_COMPLETE, kernel_id, 0);
             tomp_dispatch_finish();

             respond_to_host(omp_msgq_pkt, kernel_id);
          }
          else
          {
             do
             {
                 tomp_dispatch_once();
             } 
             while (!tomp_dispatch_is_finished());

             /* Not sending a response to host, delete the msg */
             MessageQ_free((MessageQ_Msg)omp_msgq_pkt);
          }

          flush_msg_t*  flushMsgPtr = &Msg->u.k.flush;
          flush_buffers(flushMsgPtr);
          TRACE(ULM_OCL_IOT_CACHE_COHERENCE_COMPLETE, kernel_id, 0);

          omp_msgq_pkt = NULL;
       }
       else
       {
          /* Error */;
       }
    } /* while (true) */
}
#endif


/******************************************************************************
* process_kernel_command
******************************************************************************/
static void process_kernel_command(ocl_msgq_message_t *msgq_pkt)
{
    Msg_t           *msg = &(msgq_pkt->message);
    kernel_config_t *cfg = &msg->u.k.config;

    int              done;
    uint32_t         workgroup = 0;
    uint32_t         WGid[3]   = {0,0,0};
    uint32_t         limits[3];
    uint32_t         offsets[3];

    memcpy(limits,  cfg->global_size,   sizeof(limits));
    memcpy(offsets, cfg->global_offset, sizeof(offsets));

    bool is_debug_mode = (cfg->WG_gid_start[0] == DEBUG_MODE_WG_GID_START);
    if (is_debug_mode)
    {
        if (!(MASTER_CORE)) return;
    }
    else
    {
        bool any_work = setup_ndr_chunks(cfg->num_dims, limits, offsets,
                                        cfg->global_size, cfg->local_size);
        if (!any_work) return;
    }

    int factor = (n_cores > 1) ? DNUM : 0;

    workgroup = factor * (limits[0] * limits[1] * limits[2]) /
                (cfg->local_size[0] * cfg->local_size[1] * cfg->local_size[2]);
    /*---------------------------------------------------------
    * Iterate over each Work Group
    *--------------------------------------------------------*/
    do 
    {
        cfg->WG_gid_start[0] = offsets[0] + WGid[0];
        cfg->WG_gid_start[1] = offsets[1] + WGid[1];
        cfg->WG_gid_start[2] = offsets[2] + WGid[2];
        cfg->WG_id          = workgroup++;

        TRACE(ULM_OCL_NDR_KERNEL_START, msg->u.k.kernel.Kernel_id, cfg->WG_id);
        service_workgroup(msg);
        TRACE(ULM_OCL_NDR_KERNEL_COMPLETE, msg->u.k.kernel.Kernel_id,
                                                                   cfg->WG_id);

        done = incVec(cfg->num_dims, WGid, &cfg->local_size[0], limits);

    } while (!done);
}

/******************************************************************************
* setup_ndr_chunks
******************************************************************************/
#define IS_MULTIPLE(x, y)       ((y) % (x) == 0)
static bool setup_ndr_chunks(int dims, uint32_t* limits, uint32_t* offsets,
                                      uint32_t *gsz, uint32_t* lsz)
{
    // Degenrate case - only one core available
    if (n_cores == 1)
        return true;

    // Try to split across cores along the first dimension where global and local sz is
    // a multiple of chunk size
    int num_chunks = n_cores;
    while (--dims >= 0)
        if (IS_MULTIPLE(num_chunks, gsz[dims]) && IS_MULTIPLE(lsz[dims], gsz[dims] / num_chunks))
        {
            limits[dims] /= num_chunks;
            offsets[dims] += (DNUM * limits[dims]);
            return true;
        }

    // If we failed to split, execute on one of the cores
    return (MASTER_CORE);
}



/******************************************************************************
* service_workgroup - service an individual work group for a kernel.
******************************************************************************/
static void service_workgroup(Msg_t* msg)
{
    kernel_msg_t* kernelMsgPtr = &msg->u.k.kernel;

    /*---------------------------------------------------------
    * Copy the configuration in L2, where the kernel wants it
    *--------------------------------------------------------*/
    memcpy((void*)&kernel_config_l2, (void*)&msg->u.k.config, 
           sizeof(kernel_config_t));

    uint32_t more_args_size = msg->u.k.kernel.args_on_stack_size;
    void *   more_args      = (void *) msg->u.k.kernel.args_on_stack_addr;
    dsp_rpc(&kernelMsgPtr->entry_point, more_args, more_args_size);
}

/******************************************************************************
* process_cache_command 
******************************************************************************/
static void process_cache_command (int pkt_id, ocl_msgq_message_t *msgq_pkt)
{
    flush_msg_t* flush_msg = &msgq_pkt->message.u.k.flush;
    respond_to_host(msgq_pkt, pkt_id);
    flush_buffers(flush_msg);
}

/******************************************************************************
* process_exit_command
******************************************************************************/
static void process_exit_command(ocl_msgq_message_t *msg_pkt)
{
#if !defined( _SYS_BIOS)
    tomp_exitOpenMPforOpenCL();
#endif

#ifdef DEVICE_K2G
    respond_to_host(msg_pkt, -1);
    Log_print0(Diags_INFO, "ocl_monitor: EXIT");
    cacheWbInvAllL2();
    exit(0);
#else
    /* Not sending a response to host, delete the msg */
    MessageQ_free((MessageQ_Msg)msg_pkt);
    Log_print0(Diags_INFO, "ocl_monitor: EXIT, no response");

    #if !defined(_SYS_BIOS)
    enable_ipcpower_suspend();
    #endif

    cacheWbInvAllL2();

    // SYS_BIOS mode, exit
    #if defined(_SYS_BIOS)
    Ipc_stop();
    exit(0);
    #endif

#endif
}

/******************************************************************************
* process_setup_debug_command
******************************************************************************/
static void process_setup_debug_command(ocl_msgq_message_t* msg_pkt)
{
    MessageQ_free((MessageQ_Msg)msg_pkt);
#if defined(DEVICE_AM572x) && !defined(_SYS_BIOS)
    disable_ipcpower_suspend();
    initialize_gdbserver();
#else
#endif
}

static void initialize_gdbserver()
{
#if defined(GDB_ENABLED)
    
    // Dedicated DSP interrupt vector used by GDB monitor for DSP-ARM IPC
    int error = GDB_server_init(4); 

    if(error != 0)
        Log_error1("GDB monitor init failed, error code:%d\n",error);
    else
        Log_print0(Diags_INFO, "C66x GDB monitor init success...\n");

#endif

   return;
}

#if defined(DEVICE_AM572x) && !defined(_SYS_BIOS)
void ocl_suspend_call(Int event, Ptr data)
{
    free_edma_hw_channels();
}

void ocl_resume_call(Int event, Ptr data)
{
    restore_edma_hw_channels();
}

static void initialize_ipcpower_callbacks()
{
    IpcPower_registerCallback(IpcPower_Event_SUSPEND, ocl_suspend_call, NULL);
    IpcPower_registerCallback(IpcPower_Event_RESUME,  ocl_resume_call,  NULL);
}

extern int AET_C66x_GDB_Release(void);
static void enable_ipcpower_suspend()
{
#if defined(GDB_ENABLED)
    AET_C66x_GDB_Release();
#endif
    while (! IpcPower_canHibernate())  IpcPower_hibernateUnlock();
}

static void disable_ipcpower_suspend()
{
    if (IpcPower_canHibernate())  IpcPower_hibernateLock();
}
#endif

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
* respond_to_host
******************************************************************************/
static void respond_to_host(ocl_msgq_message_t *msgq_pkt, uint32_t msgId)
{
    msgq_pkt->message.trans_id = msgId;
    MessageQ_setReplyQueue(ocl_queues.dspQue,  (MessageQ_Msg)msgq_pkt);
    MessageQ_put          (ocl_queues.hostQue, (MessageQ_Msg)msgq_pkt);
}

/******************************************************************************
* flush_buffers
******************************************************************************/
static void flush_buffers(flush_msg_t *Msg)
{
    cacheWbInvAllL2();
    return; 
}


/******************************************************************************
* CIO support from dispatched kernels.
*    these low level routines are called from stdio and we use them to 
*    redirect the io to the host for display.
******************************************************************************/
_CODE_ACCESS void __TI_writemsg(               unsigned char  command,
                                register const unsigned char *parm,
                                register const          char *data,
                                                unsigned int  length)
{
    if (!enable_printf)
        return;

    ocl_msgq_message_t *msg = 
        (ocl_msgq_message_t *)MessageQ_alloc(0, sizeof(ocl_msgq_message_t));
    if (!msg) return;

    Msg_t *printMsg = &(msg->message);
    printMsg->command = PRINT;

    unsigned int msgLen = sizeof(kernel_msg_t) + sizeof(flush_msg_t);
    printMsg->u.message[0] = DNUM + '0';
    msgLen = (length <= msgLen-2) ? length : msgLen-2;
    memcpy(printMsg->u.message+1, data, msgLen);
    printMsg->u.message[msgLen+1] = '\0';

    MessageQ_setReplyQueue(ocl_queues.dspQue,     (MessageQ_Msg)msg);
    int status = MessageQ_put(ocl_queues.hostQue, (MessageQ_Msg)msg);
    if (status != MessageQ_S_SUCCESS)
        Log_print0(Diags_INFO, "__TI_writemsg: Message put failed");

    return;
}

_CODE_ACCESS void __TI_readmsg(register unsigned char *parm,
                               register char          *data)
{
    return;  // do nothing
}



/*
 *  Create a DSP message queue. Returns false if queue creation fails
 */
static bool create_mqueue()
{
    MessageQ_Params msgqParams;

    /* Create DSP message queue (inbound messages from ARM) */
    MessageQ_Params_init(&msgqParams);
    ocl_queues.dspQue = MessageQ_create(Ocl_DspMsgQueueName[DNUM], &msgqParams);

    if (ocl_queues.dspQue == NULL) 
    {
        Log_print1(Diags_INFO,"create_mqueue: DSP %d MessageQ creation failed",
                   DNUM);
        return false;
    }

    ocl_queues.hostQue = MessageQ_INVALIDMESSAGEQ;

    Log_print1(Diags_INFO,"create_mqueue: %s ready", Ocl_DspMsgQueueName[DNUM]);

#if defined(_SYS_BIOS)
    /* get the SR_0 heap handle */
    IHeap_Handle heap = (IHeap_Handle) SharedRegion_getHeap(0);
    /* Register this heap with MessageQ */
    int status = MessageQ_registerHeap(heap, 0);
#endif

    return true;
}

#if defined(_SYS_BIOS)
void mainDsp1TimerTick(UArg arg)
{
    Clock_tick();
}
#endif


static void process_configuration_message(ocl_msgq_message_t* msgq_pkt)
{
    /* Get a pointer to the OpenCL payload in the message */
    Msg_t *ocl_msg = &(msgq_pkt->message);

    n_cores = ocl_msg->u.configure_monitor.n_cores;

    MessageQ_free((MessageQ_Msg)msgq_pkt);

    /* Do this early since the heap is initialized by OpenMP */
    int master_core = (n_cores > 1) ? 0 : DNUM;

    Log_print2(Diags_INFO,"Configuring OpenMP (%d, %d)\n", master_core, n_cores);

#if !defined(_SYS_BIOS)
    if (tomp_initOpenMPforOpenCLPerApp(master_core, n_cores) < 0)
        System_abort("main: tomp_initOpenMPforOpenCL() failed");
#endif

    if (!edmamgr_initialized)
    {
        initialize_edmamgr();
        edmamgr_initialized = true;
    }

    Log_print1(Diags_INFO, "\t (%d cores)\n", n_cores);
}
