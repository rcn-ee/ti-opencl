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
#include <stdlib.h>
#include <time.h>

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

static void lost_dsp();

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

    const tiocl::DeviceInfo& device_info = tiocl::DeviceInfo::Instance();
    const std::set<uint8_t>& compute_units = device_info.GetComputeUnits();

#ifdef _SYS_BIOS
    /* Ipc_attach must be called in ProcSync_PAIR protocol */
    for (int i : compute_units)
    {
        char dspName[8];
        snprintf(dspName, 8, "DSP%d", i+1);
        UInt16 remoteProcId = MultiProc_getId(String(dspName));
        assert(remoteProcId != MultiProc_INVALIDID);
        do {
            status = Ipc_attach(remoteProcId);
        } while ((status < 0) && (status == Ipc_E_NOTREADY));
    }

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

    int j = 0;
    for(int i : compute_units)
    {
        assert(i < Ocl_MaxNumDspMsgQueues);

        MessageQ_QueueId queue;
#if !defined(_SYS_BIOS)
        status = MessageQ_open(const_cast<char *>(Ocl_DspMsgQueueName[i]), &queue);
#else
        do {
            status = MessageQ_open(const_cast<char *>(Ocl_DspMsgQueueName[i]), &queue);
            Task_sleep(1);
        } while (status == MessageQ_E_NOTFOUND);
#endif

        if(status == MessageQ_S_SUCCESS)
            dspQue[j++] = queue;
    }

    if (j != p_device->dspCores())
        ReportError(ErrorType::Fatal, ErrorKind::MessageQueueCountMismatch,
                    j, p_device->dspCores());

// YUAN DEBUG Begin
// Test the EVE Proxy queue on M4
    MessageQ_QueueId eveProxyQueue;
    status = MessageQ_open(const_cast<char *>("OCL:EVEProxy:MsgQ"), &eveProxyQueue);
    if (status == MessageQ_S_SUCCESS)
    {
        printf("Opening EVE proxy queue: success\n");
      for (j = 0; j < 4; j++)
      {
        ocl_msgq_message_t *msg = (ocl_msgq_message_t *)
                            MessageQ_alloc(heapId, sizeof(ocl_msgq_message_t));
        assert (msg != NULL);
        MessageQ_setReplyQueue(hostQue, (MessageQ_Msg)msg);
        msg->message.trans_id = 54321;
        msg->message.u.k_eve.host_msg = 0;  // from host
        msg->message.u.k_eve.eve_id   = j;  // to which eve

    cl_ulong rs;
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
        clock_gettime(CLOCK_REALTIME, &tp);
    rs = tp.tv_nsec / 1e3;  // convert to microseconds
    rs += tp.tv_sec * 1e6;  // convert to microseconds

        status = MessageQ_put(eveProxyQueue, (MessageQ_Msg)msg);
        if (status < 0)  printf("Sending msg to EVE proxy queue: failed\n");
        else
        {
            status = MessageQ_get(hostQue, (MessageQ_Msg *)&msg,
                                  MessageQ_FOREVER);
            if (status < 0)  printf("Receiving from EVE proxy queue: failed\n");
            else if (status != MessageQ_E_UNBLOCKED)
            {
    cl_ulong rs2;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
        clock_gettime(CLOCK_REALTIME, &tp);
    rs2 = tp.tv_nsec / 1e3;  // convert to microseconds
    rs2 += tp.tv_sec * 1e6;  // convert to microseconds

                assert(msg != NULL);
                printf("Receive from EVE transid: 0x%x, retcode: 0x%x\n", msg->message.trans_id, msg->message.u.command_retcode.retcode);
                printf("Delay: %lld us\n", rs2 - rs);
                MessageQ_free((MessageQ_Msg)msg);
            }
        }
      }
    }
    else
    {
        printf("Failed to open EVE proxy queue\n");
    }
// YUAN DEBUG End
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
    if (status < 0)
        lost_dsp();
    assert (status == MessageQ_S_SUCCESS);

    return;
}

uint32_t MBoxMsgQ::read (uint8_t *buf, uint32_t *size, uint8_t* id)
{ 
    ocl_msgq_message_t *msg = NULL;

    int status = MessageQ_get(hostQue, (MessageQ_Msg *)&msg, MessageQ_FOREVER);
    if (status < 0)
        lost_dsp();

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

/* Attempt to cleanly terminate when a DSP is lost on Linux. For now, implement
 * the unsynchronized workaround described in MCT-518/MCT-540 which may produce
 * race conditions and work improperly depending on timing, but typically
 * enables resetting all DSPs (to handle OMP barrier reset) with clean
 * termination of IPC and the application allowing another OCL application to
 * run without rebooting. */
static void lost_dsp()
{
#if defined(_SYS_BIOS)
    /* Recovery not supported on SYS_BIOS host */
    ReportError(
        ErrorType::Fatal, ErrorKind::LostDSP,
        " The runtime is unable to recover.");

#elif defined(DEVICE_AM57)
    ReportError(
        ErrorType::FatalNoExit, ErrorKind::LostDSP,
        " Please wait while the DSPs are reset and the runtime attempts to "
        "terminate. A reboot may be required before running another OpenCL "
        "application if this fails. See the kernel log for fault information.");

    /* Let the faulted DSP settle from iommu-fault-handler induced reset before
     * cleaning up IPC */
    sleep(1);
    Ipc_stop();

    /* Reset all DSPs to avoid hang on OMP barrier synchronization and clean up
     * any other corrupted DSP state */
    char const *dspnames[] = {"40800000.dsp", "41000000.dsp"};

    const tiocl::DeviceInfo& device_info = tiocl::DeviceInfo::Instance();
    const std::set<uint8_t>& compute_units = device_info.GetComputeUnits();

    for (int i : compute_units)
    {
        if(!(i >= 0 && i < (sizeof dspnames / sizeof *dspnames)))
            continue;

        std::string dspname = dspnames[i];
        std::string command =
            "echo " + dspname + " >/sys/bus/platform/drivers/omap-rproc/unbind; "
            "echo " + dspname + " >/sys/bus/platform/drivers/omap-rproc/bind";

        system(command.c_str());
    }

    /* Sleep again before exiting to make sure all DSPs are finished resetting
     * to be ready for the next application */
    sleep(2);

    _exit(EXIT_FAILURE);

#else
    ReportError(
        ErrorType::FatalNoExit, ErrorKind::LostDSP,
        " Please wait while the runtime attempts to terminate. A reboot "
        " may be required before running another OpenCL application if "
        " this fails. See the kernel log for fault information.");

    /* Let the faulted DSP settle from iommu-fault-handler induced reset before
     * cleaning up IPC and exiting to be ready for the next application */
    sleep(1);
    Ipc_stop();

    _exit(EXIT_FAILURE);

#endif
}
