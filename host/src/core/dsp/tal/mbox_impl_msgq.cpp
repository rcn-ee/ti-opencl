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

#include "device_info.h"
#include "mbox_impl_msgq.h"
#include "core/error_report.h"

#ifndef _SYS_BIOS
/* Work around IPC usage of typedef void */
#define Void void
#include <ti/ipc/Ipc.h>
#include <ti/ipc/transports/TransportRpmsg.h>
#undef Void
#else
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
#define snprintf(a,b,c, ...)  sprintf(a,c,__VA_ARGS__)
#define getpid() 2016
#endif

#define HostMsgQueString    "OCL:MsgQ:%d"

using namespace Coal;
using namespace tiocl;

MBoxMsgQ::MBoxMsgQ(Coal::DSPDevice *device)
    : heapId(0), p_device(device)
{
#ifndef _SYS_BIOS
    /* Configure the transport factory */
    Ipc_transportConfig(&TransportRpmsg_Factory);
#endif

    /* Ipc_start must be called before any Message queue operations */
    int status = Ipc_start();
    assert (status == Ipc_S_SUCCESS || status == Ipc_S_ALREADYSETUP);

#ifdef _SYS_BIOS
    /* Ipc_attach must be called in ProcSync_PAIR protocol */
    UInt16 remoteProcId = MultiProc_getId(String("DSP1"));
    assert(remoteProcId != MultiProc_INVALIDID);
    do {
        status = Ipc_attach(remoteProcId);
    } while ((status < 0) && (status == Ipc_E_NOTREADY));

    remoteProcId = MultiProc_getId(String("DSP2"));
    assert(remoteProcId != MultiProc_INVALIDID);
    do {
        status = Ipc_attach(remoteProcId);
    } while ((status < 0) && (status == Ipc_E_NOTREADY));

    /* get the SR_0 heap handle */
    IHeap_Handle heap = (IHeap_Handle) SharedRegion_getHeap(0);

    /* Register this heap with MessageQ */
    status = MessageQ_registerHeap(heap, heapId);
#endif

    /* Create the host message queue (inbound messages from DSPs) */
    MessageQ_Params     msgqParams;
    MessageQ_Params_init(&msgqParams);

    char hostQueueName[32];
    snprintf(hostQueueName, sizeof hostQueueName, HostMsgQueString, getpid());
    hostQueueName[sizeof hostQueueName - 1] = '\0';
    hostQue = MessageQ_create(hostQueueName, &msgqParams);
    assert (hostQue != NULL);

    /* Open the DSP message queues (outbound messages to DSPs) */
    assert(p_device->dspCores() <= Ocl_MaxNumDspMsgQueues);

    const tiocl::DeviceInfo& device_info = tiocl::DeviceInfo::Instance();
    const std::set<uint8_t>& compute_units = device_info.GetComputeUnits();
    int j = 0;
    for(int i : compute_units)
    {
        assert(i < Ocl_MaxNumDspMsgQueues);

        MessageQ_QueueId queue;
        status = MessageQ_open(const_cast<char *>(Ocl_DspMsgQueueName[i]), &queue);

        if(status == MessageQ_S_SUCCESS)
            dspQue[j++] = queue;
    }

    if (j != p_device->dspCores())
        ReportError(ErrorType::Fatal, ErrorKind::MessageQueueCountMismatch,
                    j, p_device->dspCores());
}

void MBoxMsgQ::write (uint8_t *buf, uint32_t size, uint32_t trans_id, 
                      uint8_t id)
{ 
    assert(id < p_device->dspCores());

    ocl_msgq_message_t *msg = 
       (ocl_msgq_message_t *)MessageQ_alloc(heapId, sizeof(ocl_msgq_message_t));
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

uint32_t MBoxMsgQ::read (uint8_t *buf, uint32_t *size, uint8_t* id)
{ 
    ocl_msgq_message_t *msg = NULL;

    int status = MessageQ_get(hostQue, (MessageQ_Msg *)&msg, MessageQ_FOREVER);

    /*-------------------------------------------------------------------------
    * if a ptr to an id is passed in, return the core of the sender in it
    *------------------------------------------------------------------------*/
    if (id != 0)
    {
        MessageQ_QueueId dspQueId = MessageQ_getReplyQueue(msg);
        auto     it   = std::find(dspQue, dspQue + p_device->dspCores(), dspQueId);
        uint32_t core = std::distance(dspQue, it);
        *id = core;
    }

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
    /* Close the DSP message queues */
    for(int i = 0; i < p_device->dspCores(); ++i)
    {
        int status = MessageQ_close(&dspQue[i]);
        assert(status == MessageQ_S_SUCCESS);
    }

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
    int status = MessageQ_delete(&hostQue);
    assert (status == MessageQ_S_SUCCESS);

    Ipc_stop();
}


