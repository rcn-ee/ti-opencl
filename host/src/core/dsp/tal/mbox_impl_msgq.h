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
#pragma once

/* package header files */
#include <ti/ipc/Std.h>
/* Work around IPC usage of typedef void */
#define Void void
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/MessageQ.h>
#undef Void

#include "u_locks_pthread.h"
#include "u_lockable.h"
#include "device.h"

#include "mbox_interface.h"

#include "mbox_msgq_shared.h"

using namespace Coal;

class MBoxMsgQ : public MBox, public Lockable
{
    public:
        MBoxMsgQ(Coal::DSPDevice *device);
        ~MBoxMsgQ();
        void     to   (uint8_t *msg, uint32_t  size, uint8_t  id=0);
        int32_t  from (uint8_t *msg, uint32_t *size, uint8_t* id=0);
        bool     query(uint8_t id=0);

  private:
    void     write (uint8_t *buf, uint32_t size, uint32_t trans_id, uint8_t id=0);
    uint32_t read  (uint8_t *buf, uint32_t *size, uint8_t *id=0);

  private:
    MessageQ_Handle    hostQue;   // created by host
    MessageQ_QueueId   dspQue[Ocl_MaxNumDspMsgQueues]; // created by DSPs
    UInt16             heapId;    // heap for MessageQ_alloc, 0 on host
    Coal::DSPDevice   *p_device;
};

inline void MBoxMsgQ::to(uint8_t *msg, uint32_t  size, uint8_t id)
{
    static unsigned trans_id = TX_ID_START;

    Lock lock(this);
    write(msg, size, trans_id++, id);
}

inline int32_t MBoxMsgQ::from (uint8_t *msg, uint32_t *size, uint8_t *id)
{
    return read(msg, size, id);
}


