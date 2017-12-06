/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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

/*
 *  ======== MainEve.c ========
 *
 */

/* xdctools header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/IHeap.h>

/* package header files */
#include <ti/ipc/Ipc.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/HeapBufMP.h>
#include <ti/ipc/SharedRegion.h>

#include <ti/sysbios/family/shared/vayu/IntXbar.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/csl/arch/arp32/arp32_wugen.h>
#include <ti/drv/pm/pmlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* local header files */
#include "tal/mbox_msgq_shared.h"
#include "eve_builtins.h"


//#include "alg_osal.h"
//#include "xdais_types.h"

#define EVEMSGQNAME2(core) ("OCL:" #core ":MsgQ")
#define EVEMSGQNAME(core)  EVEMSGQNAME2(core)

/* external functions */
extern void* eve_rpc(void *p, int args_on_stack_size, void *args_in_reg);

/* private functions */
static Void smain(UArg arg0, UArg arg1);
static void process_task_command(Msg_t* ocl_msg);

MessageQ_Handle     eveQ;
HeapBufMP_Handle    srHeapHandle;
ocl_msgq_message_t* ocl_msgq_pkt  = NULL;
IHeap_Handle        msg_heap      = NULL;
MessageQ_QueueId    enable_printf = MessageQ_INVALIDMESSAGEQ;


/******************************************************************************
* main
******************************************************************************/
Int main(Int argc, Char* argv[])
{
    Error_Block     eb;
    Task_Params     taskParams;
    MessageQ_Params msgqParams;
    Int             status;

    /* Doing a Timer Xbar configuration update as BIOS is hardcoding this
     * value
     * 6th Irq Cross bar instance is tied to 339th instance for Timer 14.
     * 7th Irq Cross bar instance is tied to 340th instance for Timer 15.
     */
    //IntXbar_connect(6U, 340U);
    //IntXbar_connect(7U, 341U);

    do
    {
        status = Ipc_start();
    } while (status != Ipc_S_SUCCESS);

    /* create EVE's message queue */
    MessageQ_Params_init(&msgqParams);
    eveQ = MessageQ_create(EVEMSGQNAME(EVECORE), &msgqParams);
    if(eveQ == NULL)  return (-1);

    /* must initialize the error block before using it */
    Error_init(&eb);

    /* create main thread (interrupts not enabled in main on BIOS) */
    Task_Params_init(&taskParams);
    taskParams.instance->name = "smain";
    taskParams.arg0 = (UArg)argc;
    taskParams.arg1 = (UArg)argv;
    taskParams.stackSize = 0x1000;
    Task_create(smain, &taskParams, &eb);

    if (Error_check(&eb)) {
        System_abort("main: failed to create application startup thread");
    }

    /* start scheduler, this never returns */
    BIOS_start();

    return (0);
}


/******************************************************************************
* smain
******************************************************************************/
Void smain(UArg arg0, UArg arg1)
{
    Int                 status = 0;
    MessageQ_QueueId    replyQ;

    /* attach to IPU */
    do {
        status = Ipc_attach(MultiProc_getId("IPU1"));
        Task_sleep(1);
    } while (status == Ipc_E_NOTREADY);

    /* Setup heap for message queue: used for printf */
    msg_heap = (IHeap_Handle) SharedRegion_getHeap(0);
    if (msg_heap != NULL)  MessageQ_registerHeap(msg_heap, XDC_CFG_HeapID_Eve);

    /* the main loop */
    while (true) {
        status = MessageQ_get(eveQ, (MessageQ_Msg *)&ocl_msgq_pkt,
                              MessageQ_FOREVER);
        replyQ = MessageQ_getReplyQueue(ocl_msgq_pkt);
        enable_printf = replyQ;

        Msg_t *ocl_msg = &(ocl_msgq_pkt->message);
        int retcode    =  CL_SUCCESS;

        switch (ocl_msg->command & (~EVE_MSG_COMMAND_MASK))
        {
            case TASK:
                process_task_command(ocl_msg);
                break;
            case EXIT:
                break;
            default:
                break;
        }

        enable_printf = MessageQ_INVALIDMESSAGEQ;

        ocl_msg->trans_id = ocl_msg->u.k_eve.Kernel_id;
        ocl_msg->u.command_retcode.retcode = retcode;
        MessageQ_put(replyQ, (MessageQ_Msg)ocl_msgq_pkt);
    }
}

/******************************************************************************
* process_task_command
******************************************************************************/
static void process_task_command(Msg_t* ocl_msg)
{
    uint32_t bik_index = ocl_msg->u.k_eve.builtin_kernel_index;
    eve_rpc(tiocl_eve_builtin_kernel_table[bik_index],
            ocl_msg->u.k_eve.args_on_stack_size,
            ocl_msg->u.k_eve.args_in_reg);
}


/******************************************************************************
* CIO support from dispatched kernels.
*    these low level routines are called from stdio and we use them to
*    redirect the io to the host for display.
******************************************************************************/
_CODE_ACCESS void writemsg(               unsigned char  command,
                           register const unsigned char *parm,
                           register const          char *data,
                                           unsigned int  length)
{
    if (msg_heap == NULL || enable_printf == MessageQ_INVALIDMESSAGEQ)
        return;

    ocl_msgq_message_t *msg = (ocl_msgq_message_t *)
                MessageQ_alloc(XDC_CFG_HeapID_Eve, sizeof(ocl_msgq_message_t));
    if (!msg) return;

    Msg_t *printMsg   = &(msg->message);
    printMsg->command = (PRINT | EVE_MSG_COMMAND_MASK);
    printMsg->pid     = ocl_msgq_pkt->message.pid;

    unsigned int msgLen = sizeof(printMsg->u.message);
    printMsg->u.message[0] = __eve_num() + '0';
    msgLen = (length <= msgLen-2) ? length : msgLen-2;
    memcpy(printMsg->u.message+1, data, msgLen);
    printMsg->u.message[msgLen+1] = '\0';

    MessageQ_put(enable_printf,  (MessageQ_Msg)msg);
    return;
}

_CODE_ACCESS void readmsg(register unsigned char *parm,
                          register char          *data)
{
    return;  // do nothing
}


/******************************************************************************
* mainARP32_0_TimerTick
******************************************************************************/
void mainARP32_0_TimerTick(UArg arg)
{
    Clock_tick();
}

/******************************************************************************
* eve_idleFxn
******************************************************************************/
void eve_idleFxn()
{
#if 0
    UInt32 key = Hwi_disable();
    ARP32_WUGEN_IRQ_Interrupt_Lookup();
    Hwi_restore(key);

    /* Enter low power */
    PMLIBCpuIdle(PMHAL_PRCM_PD_STATE_RETENTION);
#endif
}

