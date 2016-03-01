/******************************************************************************
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Texas Instruments Incorporated nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef _mbox_msgq_shared_h_
#define _mbox_msgq_shared_h_

#if defined (__cplusplus)
extern "C" {
#endif

#include "message.h"

/* OpenCL MessageQ message for ARM-DSP communication. The first field in the 
 * message must be a MessageQ_MsgHeader structure
 */
typedef struct {
    MessageQ_MsgHeader  reserved;
    Msg_t               message;
} ocl_msgq_message_t;

static char const *const Ocl_DspMsgQueueName[] = {
#if defined(DEVICE_AM57) || defined(DEVICE_AM572x)
    "OCL:DSP1:MsgQ",
    "OCL:DSP2:MsgQ",
#elif defined(DEVICE_K2G)
    "OCL:CORE0:MsgQ",
#else
    #error Unknown device
#endif
};

static int const Ocl_MaxNumDspMsgQueues =
    sizeof Ocl_DspMsgQueueName / sizeof Ocl_DspMsgQueueName[0];


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
#endif /* _mbox_msgq_shared_h_ */
