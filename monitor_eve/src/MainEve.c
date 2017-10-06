/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== MainEve1.c ========
 *
 */

/* xdctools header files */
#include <xdc/std.h>
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

#include <ti/sysbios/family/shared/vayu/IntXbar.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/csl/arch/arp32/arp32_wugen.h>
#include <ti/drv/pm/pmlib.h>

/* local header files */
#include "tal/mbox_msgq_shared.h"


//#include "alg_osal.h"
//#include "xdais_types.h"

#define EVEMSGQNAME2(core) ("OCL:" #core ":MsgQ")
#define EVEMSGQNAME(core)  EVEMSGQNAME2(core)

/* private functions */
static Void smain(UArg arg0, UArg arg1);

MessageQ_Handle     eveQ;
HeapBufMP_Handle    srHeapHandle;
ocl_msgq_message_t* ocl_msgq_pkt = NULL;


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
    Error_Block         eb;
    MessageQ_QueueId    queId;

    Error_init(&eb);

    /* attach to IPU */
    do {
        status = Ipc_attach(MultiProc_getId("IPU1"));
        Task_sleep(1);
    } while (status == Ipc_E_NOTREADY);

    /* the main loop */
    while(1) {
        status = MessageQ_get(eveQ, (MessageQ_Msg *)&ocl_msgq_pkt,
                              MessageQ_FOREVER);
#if 0
        if( ((App_Msg *)msg)->msg.command == BUILTIN_KERNEL )
            run_kernel(&((App_Msg *)msg)->msg);
        ((App_Msg *)msg)->msg.command = READY;
#endif
        ocl_msgq_pkt->message.trans_id = (unsigned int) ocl_msgq_pkt;
        ocl_msgq_pkt->message.u.command_retcode.retcode = (unsigned int) &smain;
        queId = MessageQ_getReplyQueue(ocl_msgq_pkt);
        MessageQ_put(queId, (MessageQ_Msg)ocl_msgq_pkt);
    }
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
