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
#include "custom_rsc_table_vayu_ipu1.h"
#include "tal/mbox_msgq_shared.h"
#include "src/utils_eveloader.h"

#define MAX_NUM_EVES               (4)
#define MAX_NUM_COMPLETION_PENDING (16)

/* private data */
int              num_eve_devices;
int              eve_work_count = 0;
MessageQ_Handle  eveProxyQueue = NULL;
MessageQ_QueueId eveQueues[MAX_NUM_EVES];

ocl_msgq_message_t* ocl_msgq_pkt = NULL;
ocl_msgq_message_t* eve_ocl_msgq_pkt[MAX_NUM_EVES][MAX_NUM_COMPLETION_PENDING];
int                 eve_ocl_msgq_avail_slot[MAX_NUM_EVES];

/* private functions */
static Void smain(UArg arg0, UArg arg1);
static bool create_mqueue();
static int  GetNumEVEDevices();


/*
 *  ======== main ========
 */
Int main(Int argc, Char* argv[])
{
    Error_Block     eb;
    Task_Params     taskParams;
    Int             status;

    Log_print0(Diags_ENTRY, "--> main:");

    /* Check available EVE devices to see if OpenCL firmware applies */
    num_eve_devices = GetNumEVEDevices();
    if (num_eve_devices <= 0)
    {
        Log_print0(Diags_INFO | Diags_USER6,
                   "OpenCL runtime firmware does not apply. Exit.");
        return (-1);
    }

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

    Log_print0(Diags_INFO | Diags_USER6, "Creating msg queue...");
    /* Create the M4 proxy queue for EVEs */
    if (!create_mqueue())
    {
        Log_print0(Diags_INFO | Diags_USER6, "failed to create msg queue");
        System_abort("main: create_mqueue() failed");
    }

    Log_print0(Diags_INFO | Diags_USER6, "Booting EVEs...");
    /* Boot the EVEs */
    status = Utils_eveBoot(num_eve_devices);
    if (status)
    {
        Log_print0(Diags_INFO | Diags_USER6, "failed to boot EVEs");
        System_abort("main: Utils_eveBoot() failed");
    }

    Log_print0(Diags_INFO | Diags_USER6, "Starting BIOS...");
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
    int                 i, j;

    Log_print0(Diags_ENTRY | Diags_INFO, "--> smain:");

    /* Attaching to EVEs */
    Log_print0(Diags_INFO | Diags_USER6, "Attaching to EVEs...");
    for (i = 0; i < num_eve_devices; i++)
    {
        const char *eve_name = (i == 0) ? "EVE1" :
                               (i == 1) ? "EVE2" :
                               (i == 2) ? "EVE3" :
                                          "EVE4";
        do {
            status = Ipc_attach(MultiProc_getId((String) eve_name));
            Task_sleep(1);
        } while (status == Ipc_E_NOTREADY);

        if (status < 0) {
            Log_print1(Diags_INFO | Diags_USER6,
                       "Attaching %s failed", (xdc_IArg) eve_name);
            return;
        }
        Log_print1(Diags_INFO | Diags_USER6,
                   "%s attached", (xdc_IArg) eve_name);
    }

    /* Opening MsgQs on EVEs */
    Log_print0(Diags_INFO | Diags_USER6, "Opening MsgQ on EVEs...");
    for (i = 0; i < num_eve_devices; i++)
    {
        const char *queue_name = (i == 0) ? "OCL:EVE1:MsgQ" :
                                 (i == 1) ? "OCL:EVE2:MsgQ" :
                                 (i == 2) ? "OCL:EVE3:MsgQ" :
                                            "OCL:EVE4:MsgQ";
        status = MessageQ_open((String) queue_name, &eveQueues[i]);
        if (status < 0) {
            Log_print1(Diags_INFO | Diags_USER6,
                       "Opening %s failed", (xdc_IArg) queue_name);
            return;
        }
        Log_print1(Diags_INFO | Diags_USER6,
                   "%s opened", (xdc_IArg) queue_name);
    }

    /* Pre-allocating msgs to EVEs */
    Log_print0(Diags_INFO | Diags_USER6, "Pre-allocating msgs to EVEs...");
    for (i = 0; i < num_eve_devices; i++)
    {
        for (j = 0; j < MAX_NUM_COMPLETION_PENDING; j++)
        {
            eve_ocl_msgq_pkt[i][j] = (ocl_msgq_message_t *)
                MessageQ_alloc(XDC_CFG_HeapID_Eve, sizeof(ocl_msgq_message_t));
            if (eve_ocl_msgq_pkt[i][j] == NULL)
            {
                 Log_print2(Diags_INFO | Diags_USER6,
                            "Failed to pre-allocate msgs for EVE: %d, %d",
                            i, j);
                 return;
            }
        }
        eve_ocl_msgq_avail_slot[i] = 0;
    }
    Log_print0(Diags_INFO | Diags_USER6,
               "Done OpenCL runtime initialization. Waiting for messages...");

    /* Loop forever, proxying between host and EVE */
    while (TRUE) {
        status = MessageQ_get(eveProxyQueue, (MessageQ_Msg *)&ocl_msgq_pkt,
                              MessageQ_FOREVER);
        if (status < 0)  break;

        if ((ocl_msgq_pkt->message.command & EVE_MSG_COMMAND_MASK) == 0)
        {
            /* From host, command is not masked */
            /* Allocate msg to EVE */
            int eve_id = ocl_msgq_pkt->message.core_id;
            int slot   = eve_ocl_msgq_avail_slot[eve_id];
            ocl_msgq_message_t* eve_pkt = eve_ocl_msgq_pkt[eve_id][slot];
            slot += 1;
            eve_ocl_msgq_avail_slot[eve_id] =
                               (slot == MAX_NUM_COMPLETION_PENDING) ? 0 : slot;

            /* Copy host_msg into EVE msg, mask command, remember host_msg */
            memcpy(&eve_pkt->message, &ocl_msgq_pkt->message, sizeof(Msg_t));
            eve_pkt->message.command |= EVE_MSG_COMMAND_MASK;
            eve_pkt->message.pid = (uint32_t)ocl_msgq_pkt;
            MessageQ_setReplyQueue(eveProxyQueue, (MessageQ_Msg) eve_pkt);
            MessageQ_put(eveQueues[eve_id], (MessageQ_Msg) eve_pkt);
            eve_work_count += 1;
        }
        else
        {
            /* From EVE, command is masked */
            /* Retrieve original host_msg, unmask command, restore pid */
            ocl_msgq_message_t* host_ocl_msgq_pkt = (ocl_msgq_message_t*)
                                                    ocl_msgq_pkt->message.pid;
            MessageQ_QueueId hostReplyQueue = MessageQ_getReplyQueue(
                                                            host_ocl_msgq_pkt);
            ocl_msgq_pkt->message.command &= (~EVE_MSG_COMMAND_MASK);
            ocl_msgq_pkt->message.pid = host_ocl_msgq_pkt->message.pid;

            if (ocl_msgq_pkt->message.command != PRINT)
            {
                /* Use original host_msg to send back to host */
                memcpy(&host_ocl_msgq_pkt->message,
                       &ocl_msgq_pkt->message, sizeof(Msg_t));
                MessageQ_put(hostReplyQueue, (MessageQ_Msg) host_ocl_msgq_pkt);
                eve_work_count -= 1;
            }
            else
            {
                /* Allocate a new message for printf to send back to host */
                ocl_msgq_message_t * host_print_pkt = (ocl_msgq_message_t *)
                MessageQ_alloc(XDC_CFG_HeapID_Host, sizeof(ocl_msgq_message_t));
                if (host_print_pkt != NULL)
                {
                    memcpy(&host_print_pkt->message,
                           &ocl_msgq_pkt->message, sizeof(Msg_t));
                    MessageQ_put(hostReplyQueue, (MessageQ_Msg) host_print_pkt);
                }
                else
                    Log_print0(Diags_INFO | Diags_USER6,
                               "msgq alloc failed for print");
                MessageQ_free((MessageQ_Msg) ocl_msgq_pkt);
            }
        }
    }  /* while (TRUE) */

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
        Log_print0(Diags_INFO | Diags_USER6,
                   "create_mqueue: EVE Proxy MessageQ creation failed");
        return false;
    }
    Log_print1(Diags_INFO | Diags_USER6,
               "%s ready", (xdc_IArg) "OCL:EVEProxy:MsgQ");

    /* get the SR_0 heap handle */
    IHeap_Handle heap = (IHeap_Handle) SharedRegion_getHeap(0);
    if (heap == NULL) {
        Log_print0(Diags_INFO | Diags_USER6, "SharedRegion getHeap failed\n" );
        return false;
    }
    /* Register this heap with MessageQ for communicating with EVE */
    Int status = MessageQ_registerHeap(heap, XDC_CFG_HeapID_Eve);
    Log_print0(Diags_INFO | Diags_USER6, "Heap for EVE ready");

    return true;
}

#undef  CTRL_WKUP_STD_FUSE_DIE_ID_2
#define CTRL_WKUP_STD_FUSE_DIE_ID_2  0x4AE0C20C
static int  GetNumEVEDevices()
{
    uint32_t board_type = (  *((uint32_t *) CTRL_WKUP_STD_FUSE_DIE_ID_2)
                           & 0xFF000000) >> 24;
    int      num_eves = 0;
    if (     board_type == 0x3E ||  // AM5729-E (EtherCat)
             board_type == 0x4E)    // AM5729
        num_eves = 4;
    else if (board_type == 0x5F ||  // AM5749-E (EtherCat)
             board_type == 0xA6 ||  // AM5749
             board_type == 0x69)    // AM5749IDK (shown on package sticker)
                                    // (data sheet: 0x69 is Jacinto 6 Plus)
        num_eves = 2;

    Log_print1(Diags_INFO | Diags_USER6, "%d EVEs Available", num_eves);
    if (num_eves > MAX_NUM_EVES)
    {
        num_eves = MAX_NUM_EVES;
        Log_print1(Diags_INFO | Diags_USER6, "Capped to %d (max) EVEs",
                   num_eves);
    }
    return num_eves;
}


/*
 *  Only suspend IPU when there are no work on EVEs, otherwise, messages
 *      from EVE won't be able to wake up IPU
 */
extern void IpcPower_idle();
void OclPower_idle()
{
    if (eve_work_count <= 0)
        IpcPower_idle();
}
