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
 
/*-----------------------------------------------------------------------------
* IPC header files
*----------------------------------------------------------------------------*/
#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>

/*-----------------------------------------------------------------------------
* BIOS header files
*----------------------------------------------------------------------------*/
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

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
#include "mbox_msgq_shared.h"

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

PRIVATE (bool,              enable_printf) = false;
PRIVATE (OCL_MessageQueues, ocl_queues);

extern Semaphore_Handle higherSem;

/******************************************************************************
* Defines a fixed area of 64 bytes at the start of L2, where the kernels will
* resolve the get_global_id type calls
******************************************************************************/
#pragma DATA_SECTION(kernel_config_l2, ".workgroup_config");
EXPORT kernel_config_t kernel_config_l2;

/******************************************************************************
* Initialization Routines
******************************************************************************/
static void initialize_memory        (void);
static void initialize_gdbserver     ();
static void flush_buffers(flush_msg_t *Msg);

/******************************************************************************
* External prototypes
******************************************************************************/
extern unsigned dsp_speed();
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
static void service_workgroup     (Msg_t* msg);
static int  setup_ndr_chunks      (int dims, uint32_t* limits, uint32_t* offsets, 
                                   uint32_t *gsz, uint32_t* lsz);



/* BIOS_TASKS */
/******************************************************************************
* Bios Task and helper routines
******************************************************************************/
static void ocl_main(UArg arg0, UArg arg1);
static void ocl_service_omp(UArg arg0, UArg arg1);
static bool create_mqueue(void);
static void ocl_monitor();

extern tomp_initOpenMPforOpenCL(void);
extern tomp_exitOpenMPforOpenCL(void);
extern void tomp_dispatch_once(void);
extern void tomp_dispatch_finish(void);
extern void tomp_dispatch_finish(void);
extern bool tomp_dispatch_is_finished(void);

// Prevent RTS versions of malloc etc. from getting pulled into link
//void _minit(void) { }

/*******************************************************************************
* MASTER_THREAD : One core acts as a master.  This macro identifies that thread
*******************************************************************************/
#define MASTER_THREAD (DNUM == 0)

#define LISTEN_STACK_SIZE 0x2800
#define SERVICE_STACK_SIZE 0x10000
PRIVATE_1D(char, lstack, LISTEN_STACK_SIZE);

/******************************************************************************
* main
******************************************************************************/
int main(int argc, char* argv[])
{    
    /* register with xdc.runtime to get a diags mask */
    Registry_Result result = Registry_addModule(&Registry_CURDESC, MODULE_NAME);
    assert(result == Registry_SUCCESS);

    /* enable ENTRY/EXIT/INFO log events */
    Diags_setMask(MODULE_NAME"-EXF");

    initialize_memory();
    initialize_edmamgr();
    initialize_gdbserver();

    Task_Params     taskParams;
    Error_Block     eb;
    Log_print0(Diags_ENTRY, "--> main:");

    /* Create main thread (interrupts not enabled in main on BIOS) */
    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.instance->name = "ocl_main";
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
    taskParams.priority = 3; // LOWER_PRIORITY
    taskParams.stackSize = LISTEN_STACK_SIZE;
    taskParams.stack = (xdc_Ptr)lstack; // L2 private
    Task_create(ocl_main, &taskParams, &eb);
    if (Error_check(&eb)) {
        System_abort("main: failed to create ocl_main thread");
    }

    /* Create a task to service OpenMP kernels */
    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.instance->name = "ocl_service_omp";
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
    taskParams.priority = 5; // HIGHER_PRIORITY
    taskParams.stackSize = SERVICE_STACK_SIZE;
    taskParams.stack = (xdc_Ptr)malloc(SERVICE_STACK_SIZE); // ddr shared
    Task_create(ocl_service_omp, &taskParams, &eb);
    if (Error_check(&eb)) {
        System_abort("main: failed to create ocl_service_omp thread");
    }

    if (tomp_initOpenMPforOpenCL() < 0)
       /* Error */ ;

    /* Start scheduler, this never returns */
    BIOS_start();

    /* Should never get here */
    Log_print0(Diags_EXIT, "<-- main:");
    return (0);
}

/******************************************************************************
* ocl_main
******************************************************************************/
void ocl_main(UArg arg0, UArg arg1)
{
    Log_print0(Diags_ENTRY | Diags_INFO, "--> ocl_main:");

    /* printfs are enabled ony for the duration of an OpenCL kernel */
    enable_printf = false;

    /* create the dsp queue */
    if (create_mqueue())
    {
        /* OpenCL monitor loop - does not return */
        ocl_monitor();

        /* delete the message queue */
        int status = MessageQ_delete(&ocl_queues.dspQue);

        if (status < 0) 
        {
            Log_error1("Server_finish: error=0x%x", (IArg)status);
        }
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
                Log_print0(Diags_INFO, "NDKERNEL\n");
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

            case FREQUENCY:
                Log_print0(Diags_INFO, "FREQUENCY\n");
                respond_to_host(msgq_pkt, dsp_speed());
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

    if (is_inorder)
    {
       omp_msgq_pkt = msgq_pkt;
       Semaphore_post(higherSem);
       /* in order task was completed by ocl_service_omp task*/
       if (omp_msgq_pkt != NULL)
          /* Error */; 
    }
    else
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

/******************************************************************************
* ocl_service_omp - This is it's own task to switch the stack to DDR.
******************************************************************************/
void ocl_service_omp(UArg arg0, UArg arg1)
{
    Log_print0(Diags_ENTRY | Diags_INFO, "--> ocl_service_omp:");

    while (true)
    {
       Semaphore_pend(higherSem, BIOS_WAIT_FOREVER);

       if (omp_msgq_pkt != NULL)
       {
          /*-------------------------------------------------------------------
          * Run the in order Task.  OpenMP kernels run here. 
          *-------------------------------------------------------------------*/
          Msg_t* Msg = &(omp_msgq_pkt->message);
          uint32_t kernel_id = Msg->u.k.kernel.Kernel_id;
          uint32_t more_args_size = Msg->u.k.kernel.args_on_stack_size;
          void* more_args = (void *) Msg->u.k.kernel.args_on_stack_addr;

          if (MASTER_THREAD)
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
        if (get_dsp_id() != 0) return;
    }
    else
    {
        int any_work = setup_ndr_chunks(cfg->num_dims, limits, offsets,
                                        cfg->global_size, cfg->local_size);
        if (!any_work) return;
    }

    workgroup = get_dsp_id() * (limits[0] * limits[1] * limits[2]) /
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
static int setup_ndr_chunks(int dims, uint32_t* limits, uint32_t* offsets, 
                                      uint32_t *gsz, uint32_t* lsz)
{
    int num_chunks = NUM_CORES;

    while (--dims >= 0)
        if (IS_MULTIPLE(num_chunks, gsz[dims]) && IS_MULTIPLE(lsz[dims], gsz[dims] / num_chunks))
        {
            limits[dims] /= num_chunks;
            offsets[dims] += (get_dsp_id() * limits[dims]);
            return true;
        }

    return (get_dsp_id() == 0);
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
    /* Not sending a response to host, delete the msg */
    MessageQ_free((MessageQ_Msg)msg_pkt);

    Log_print0(Diags_INFO, "ocl_monitor: EXIT, no response");

    cacheWbInvAllL2();
}


/******************************************************************************
* initialize_memory
******************************************************************************/
void initialize_memory(void)
{
    extern uint32_t nocache_virt_start;
    extern uint32_t nocache_size;
    extern uint32_t nocache2_virt_start;
    extern uint32_t nocache2_size;

    uint32_t nc_virt = (uint32_t) &nocache_virt_start;
    uint32_t nc_size = (uint32_t) &nocache_size;
    uint32_t nc2_virt = (uint32_t) &nocache2_virt_start;
    uint32_t nc2_size = (uint32_t) &nocache2_size;

    int32_t mask = _disable_interrupts();

    /***  BIOS is configuring the default cache sizes, see Platform.xdc ***/

    enableCache (0x40, 0x40); // enable write through for OCMC
    enableCache (0x80, 0xFF);
    disableCache(nc_virt >> 24, (nc_virt+nc_size-1) >> 24);
    disableCache(nc2_virt >> 24, (nc2_virt+nc2_size-1) >> 24);

    _restore_interrupts(mask);

    return;
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


#if 0
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

    _restore_interrupts(mask);
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
    MessageQ_put(ocl_queues.hostQue, (MessageQ_Msg)msgq_pkt);
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
    printMsg->u.message[0] = get_dsp_id() + '0';
    msgLen = (length <= msgLen-2) ? length : msgLen-2;
    memcpy(printMsg->u.message+1, data, msgLen);
    printMsg->u.message[msgLen+1] = '\0';

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
    MessageQ_Params     msgqParams;
    char                msgqName[MSGQ_NAME_LENGTH];

    /* Create DSP message queue (inbound messages from ARM) */
    MessageQ_Params_init(&msgqParams);
    snprintf(msgqName, MSGQ_NAME_LENGTH, Ocl_DspMsgQueName,
             MultiProc_getName(MultiProc_self()));
    ocl_queues.dspQue = MessageQ_create(msgqName, &msgqParams);

    if (ocl_queues.dspQue == NULL) 
    {
        Log_print1(Diags_INFO,"create_mqueue: DSP %d MessageQ creation failed",
                   get_dsp_id());
        return false;
    }

    ocl_queues.hostQue = MessageQ_INVALIDMESSAGEQ;

    Log_print1(Diags_INFO,"create_mqueue: DSP %d queue ready", get_dsp_id());

    return true;
}


/*
 * Sets the MultiProc core Id. Called via Startup from monitor.cfg  
 */
void ocl_set_multiproc_id()
{
    uint32_t dsp_id = get_dsp_id();

    if      (dsp_id == 0) MultiProc_setLocalId(MultiProc_getId("DSP1"));
    else if (dsp_id == 1) MultiProc_setLocalId(MultiProc_getId("DSP2"));
    else    assert(0);
}
