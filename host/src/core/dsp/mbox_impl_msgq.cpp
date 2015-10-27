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

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "mbox_impl_msgq.h"
#include "mbox_msgq_shared.h"

/* Work around IPC usage of typedef void */
#ifndef _SYS_BIOS
#define Void void
#include <ti/ipc/Ipc.h>
#include <ti/ipc/transports/TransportRpmsg.h>
#undef Void
#endif
#define HostMsgQueString    "OCL:MsgQ:%d"

#ifdef _SYS_BIOS
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/Registry.h>
#include <xdc/runtime/System.h>
#include <ti/ipc/Ipc.h>
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/SharedRegion.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#define snprintf  sprintf
#endif
using namespace Coal;

MBoxMsgQ::MBoxMsgQ(Coal::DSPDevice *device)
    : heapId(0), p_device(device)
{
#ifndef _SYS_BIOS
    /* Configure the transport factory */
    Ipc_transportConfig(&TransportRpmsg_Factory);
#endif
#ifdef _SYS_BIOS
    int status;
    IHeap_Handle  heap;

#endif

    /* Ipc_start must be called before any Message queue operations */
    status = Ipc_start();
    assert (status == Ipc_S_SUCCESS || status == Ipc_S_ALREADYSETUP);
#ifdef _SYS_BIOS
    /* get the SR_0 heap handle */
    heap = (IHeap_Handle)SharedRegion_getHeap(0);

    /* Register this heap with MessageQ */
    status = MessageQ_registerHeap(heap, heapId);

#endif

    /* Create the host message queue (inbound messages from DSPs) */
    MessageQ_Params     msgqParams;
    MessageQ_Params_init(&msgqParams);

    char hostQueueName[MSGQ_NAME_LENGTH];
    snprintf(hostQueueName, MSGQ_NAME_LENGTH, HostMsgQueString,"Host" );//getpid()
    hostQue = MessageQ_create("Host" , &msgqParams);
    assert (hostQue != NULL);

    /* Open DSP message queues for DSP1 and DSP2 */
    char dspQueueName[MSGQ_NAME_LENGTH];
    snprintf(dspQueueName, MSGQ_NAME_LENGTH, Ocl_DspMsgQueName, "DSP1");

    do {
        status = MessageQ_open("OCL:DSP1:MsgQ", &dspQue[0]);
#ifdef _SYS_BIOS		
        if (status == MessageQ_E_NOTFOUND) {
                       Task_sleep(1);
         }
#endif		 
    } while (status == MessageQ_E_NOTFOUND);

    snprintf(dspQueueName, MSGQ_NAME_LENGTH, Ocl_DspMsgQueName, "DSP2");
#if 0
    do {
        status = MessageQ_open(dspQueueName, &dspQue[1]);
    } while (status == MessageQ_E_NOTFOUND);
#endif

}

void MBoxMsgQ::write (uint8_t *buf, uint32_t size, uint32_t trans_id, 
                      uint8_t id)
{ 
    assert (id == 0 || id == 1);

#ifndef _SYS_BIOS
    IHeap_Handle  heap;
    /* get the SR_0 heap handle */
    heap = (IHeap_Handle)SharedRegion_getHeap(0);
    ocl_msgq_message_t *msg =
           (ocl_msgq_message_t *)Memory_alloc(heap, sizeof(ocl_msgq_message_t),0, NULL);
#else

    ocl_msgq_message_t *msg = 
       (ocl_msgq_message_t *)MessageQ_alloc(heapId, sizeof(ocl_msgq_message_t));
#endif
    assert (msg != NULL);

 
    /* set the return address in the message header */
    MessageQ_setReplyQueue(hostQue, (MessageQ_Msg)msg);

    assert (size <= sizeof(Msg_t));

    /* initialize payload */
    memcpy(&(msg->message), buf, size);
    msg->message.trans_id = trans_id;

    /* send message */
    int status = MessageQ_put(dspQue[id], (MessageQ_Msg)msg);
    assert (status == MessageQ_S_SUCCESS);

    return;
}

uint32_t MBoxMsgQ::read (uint8_t *buf, uint32_t *size, uint8_t id)
{ 
    ocl_msgq_message_t *msg = NULL;

    int status = MessageQ_get(hostQue, (MessageQ_Msg *)&msg, MessageQ_FOREVER);

    if (status == MessageQ_E_UNBLOCKED)
        return 0;

    assert (msg != NULL);

    *size    = sizeof(Msg_t);
    memcpy(buf, &(msg->message), *size);

    uint32_t trans_id = msg->message.trans_id;

    MessageQ_free((MessageQ_Msg)msg);

    return trans_id;
}

inline bool MBoxMsgQ::query(uint8_t id)
{
    return true;
    //return (MessageQ_count(hostQue) > 0);
}


MBoxMsgQ::~MBoxMsgQ(void)
{    
    /* Close the DSP message queue */
    int status = MessageQ_close(&dspQue[0]);
    assert (status == MessageQ_S_SUCCESS);

    status = MessageQ_close(&dspQue[1]);
    assert (status == MessageQ_S_SUCCESS);

    /* Unblocks reader thread that is blocked on a MessageQ_get(). The 
     * MessageQ_get() call will return with status MessageQ_E_UNBLOCKED 
     * indicating that it returned due to a MessageQ_unblock() rather than a 
     * timeout or a received message. This call should only be used during a 
     * shutdown sequence in order to ensure that there is no blocked reader 
     * on a queue before deleting the queue. 
     * A queue may not be used after it has been unblocked.
     */
    MessageQ_unblock(hostQue);

    /* Delete the host message queue */
    status = MessageQ_delete(&hostQue);
    assert (status == MessageQ_S_SUCCESS);

    Ipc_stop();
}


