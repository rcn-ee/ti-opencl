/*
 * Copyright (c) 2013-2014, Texas Instruments Incorporated
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
 *  ======== main_Ipu1.c ========
 *
 */

/* xdctools header files */
#include <xdc/std.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/IHeap.h>

/* package header files */
#include <ti/ipc/Ipc.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/MultiProc.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/ipc/SharedRegion.h>

/*-----------------------------------------------------------------------------
* C standard library
*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* local header files */
#include "rsc_table.h"
#include "tal/mbox_msgq_shared.h"
#include "src/rtos/utils_common/include/utils_eveloader.h"

/* private data */
// YUAN TODO: two queues: eve0fromhost, eve0fromeve
// Or should it be one, how can we tell who sends it (eve or host?)
MessageQ_Handle  eveProxyQueue = NULL;
MessageQ_QueueId hostReplyQueue = NULL;
MessageQ_QueueId eve1Queue = NULL;
MessageQ_QueueId eve2Queue = NULL;
MessageQ_QueueId eve3Queue = NULL;
MessageQ_QueueId eve4Queue = NULL;
ocl_msgq_message_t* ocl_msgq_pkt = NULL;
ocl_msgq_message_t* eve_ocl_msgq_pkt = NULL;

/* private functions */
static Void smain(UArg arg0, UArg arg1);
static bool create_mqueue();
static void respond_to_host(ocl_msgq_message_t *msgq_pkt, uint32_t msgId);


/*
 *  ======== main ========
 */
Int main(Int argc, Char* argv[])
{
    Error_Block     eb;
    Task_Params     taskParams;
    Int             status;

    Log_print0(Diags_ENTRY, "--> main:");

    /* SR0 requires Ipc_start() */
    do
    {
        status = Ipc_start();
    } while (status != Ipc_S_SUCCESS);

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

    Log_print0(Diags_INFO, "Creating msg queue...");
    /* Create the M4 proxy queue for EVEs */
    if (!create_mqueue())
    {
        Log_print0(Diags_INFO, "failed to create message queues");
        System_abort("main: create_mqueue() failed");
    }

    Log_print0(Diags_INFO, "Booting EVEs...");
    /* Boot the EVEs */
#if 1
    status = Utils_eveBoot();
    if (status)
    {
        Log_print0(Diags_INFO, "failed to boot EVEs");
        System_abort("main: Utils_eveBoot() failed");
    }
#endif

    Log_print0(Diags_INFO, "Starting BIOS...");
    /* start scheduler, this never returns */
    BIOS_start();

    /* should never get here */
    Log_print0(Diags_EXIT, "<-- main:");
    return (0);
}


/*
 *  ======== smain ========
 */
Void smain(UArg arg0, UArg arg1)
{
    Int                 status = 0;
    Error_Block         eb;
    Bool                running = TRUE;

    Log_print0(Diags_ENTRY | Diags_INFO, "--> smain:");

#if 1
    Log_print0(Diags_INFO, "Attaching to EVEs...");
    /* Attaching to EVEs */
    do {
        status = Ipc_attach(MultiProc_getId("EVE1"));
        Task_sleep(1);
    } while (status == Ipc_E_NOTREADY);

    if (status < 0) {
        Log_print0(Diags_INFO,"Attach EVE1 failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE1 attached");

    do {
        status = Ipc_attach(MultiProc_getId("EVE2"));
        Task_sleep(1);
    } while (status == Ipc_E_NOTREADY);

    if (status < 0) {
        Log_print0(Diags_INFO,"Attach EVE2 failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE2 attached");

    do {
        status = Ipc_attach(MultiProc_getId("EVE3"));
        Task_sleep(1);
    } while (status == Ipc_E_NOTREADY);

    if (status < 0) {
        Log_print0(Diags_INFO,"Attach EVE3 failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE3 attached");

    do {
        status = Ipc_attach(MultiProc_getId("EVE4"));
        Task_sleep(1);
    } while (status == Ipc_E_NOTREADY);

    if (status < 0) {
        Log_print0(Diags_INFO,"Attach EVE4 failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE4 attached");
#endif

#if 1
    Log_print0(Diags_INFO, "Opening MsgQ on EVEs...");
    /* Opening MsgQs on EVEs */
    status = MessageQ_open("OCL:EVE1:MsgQ", &eve1Queue);
    if (status < 0) {
        Log_print0(Diags_INFO,"Opening EVE1 msgQ failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE1 msgQ opened");

    status = MessageQ_open("OCL:EVE2:MsgQ", &eve2Queue);
    if (status < 0) {
        Log_print0(Diags_INFO,"Opening EVE2 msgQ failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE2 msgQ opened");

    status = MessageQ_open("OCL:EVE3:MsgQ", &eve3Queue);
    if (status < 0) {
        Log_print0(Diags_INFO,"Opening EVE3 msgQ failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE3 msgQ opened");

    status = MessageQ_open("OCL:EVE4:MsgQ", &eve4Queue);
    if (status < 0) {
        Log_print0(Diags_INFO,"Opening EVE4 msgQ failed");
        return;
    }
    Log_print0(Diags_INFO, "EVE4 msgQ opened");
#endif

    Log_print0(Diags_INFO, "Pre-allocating msg to EVEs...");
    eve_ocl_msgq_pkt = (ocl_msgq_message_t *) MessageQ_alloc(XDC_CFG_HeapID_Eve,
                                                   sizeof(ocl_msgq_message_t));

    /* loop forever */
    while (running) {
        status = MessageQ_get(eveProxyQueue, (MessageQ_Msg *)&ocl_msgq_pkt,
                              MessageQ_FOREVER);
        if (status < 0)  goto leave;
        if (ocl_msgq_pkt->message.u.k_eve.host_msg == 0)  /* from Host */
        {
            hostReplyQueue = MessageQ_getReplyQueue(ocl_msgq_pkt);
            /* unsigned int trans_id = ocl_msgq_pkt->message.trans_id;
            Log_print2(Diags_INFO,
                       "forwarding to eve, trans_id: %d, host msg: 0x%p",
                       trans_id, ocl_msgq_pkt); */

            /* forward the message to EVE */
            memcpy(&eve_ocl_msgq_pkt->message, &ocl_msgq_pkt->message,
                   sizeof(Msg_t));
            eve_ocl_msgq_pkt->message.u.k_eve.host_msg = (uint32_t)ocl_msgq_pkt;
            MessageQ_setReplyQueue(eveProxyQueue,
                                   (MessageQ_Msg) eve_ocl_msgq_pkt);
            int eve_id = ocl_msgq_pkt->message.u.k_eve.eve_id;
            MessageQ_QueueId eveQueue = eve_id == 0 ? eve1Queue :
                                       (eve_id == 1 ? eve2Queue :
                                       (eve_id == 2 ? eve3Queue :
                                                      eve4Queue));
            MessageQ_put(eveQueue, (MessageQ_Msg) eve_ocl_msgq_pkt);
        }
        else  /* from EVE */
        {
            unsigned int trans_id = ocl_msgq_pkt->message.trans_id;
            /***
            Log_print1(Diags_INFO, "forwarding to host, trans_id: %d",
                       trans_id);
            ***/
            ocl_msgq_message_t* host_ocl_msgq_pkt = (ocl_msgq_message_t*)
                                        ocl_msgq_pkt->message.u.k_eve.host_msg;
#if 0
            memcpy(&host_ocl_msgq_pkt->message.u.command_retcode,
                   &ocl_msgq_pkt->message.u.command_retcode,
                   sizeof(command_retcode_t));
#else
            memcpy(&host_ocl_msgq_pkt->message,
                   &ocl_msgq_pkt->message,
                   sizeof(Msg_t));
#endif
            respond_to_host(host_ocl_msgq_pkt, trans_id);
        }
    } /* while (running) */

leave:
    Log_print1(Diags_EXIT, "<-- smain: %d", (IArg)status);
    return;
}

/*
 *  Create M4 proxy message queues for EVEs. Returns false if fails.
 */
static bool create_mqueue()
{
    MessageQ_Params msgqParams;

    /* Create DSP message queue (inbound messages from ARM) */
    MessageQ_Params_init(&msgqParams);
    eveProxyQueue = MessageQ_create((String)"OCL:EVEProxy:MsgQ", &msgqParams);

    if (eveProxyQueue == NULL)
    {
        Log_print0(Diags_INFO,
                   "create_mqueue: EVE Proxy MessageQ creation failed");
        return false;
    }

    Log_print1(Diags_INFO, "create_mqueue: %s ready",
               (xdc_IArg) "OCL:EVEProxy:MsgQ");

    /* get the SR_0 heap handle */
    IHeap_Handle heap = (IHeap_Handle) SharedRegion_getHeap(0);
    if (heap == NULL) {
        Log_print0(Diags_INFO,"SharedRegion getHeap failed\n" );
        return false;
    }
    /* Register this heap with MessageQ for communicating with EVE */
    Int status = MessageQ_registerHeap(heap, XDC_CFG_HeapID_Eve);
    Log_print0(Diags_INFO, "Heap for EVE ready");

    return true;
}

/******************************************************************************
* respond_to_host
******************************************************************************/
static void respond_to_host(ocl_msgq_message_t *msgq_pkt, uint32_t msgId)
{
    msgq_pkt->message.trans_id = msgId;
    // msgq_pkt->message.u.command_retcode.retcode = 0;

    MessageQ_setReplyQueue(eveProxyQueue,  (MessageQ_Msg)msgq_pkt);
    MessageQ_put          (hostReplyQueue, (MessageQ_Msg)msgq_pkt);
}


