/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
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
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file ocl_qmss.h
 *       API to access the QMSS sub-system. Wraps the QMSS LLD APIs
 */

#ifndef _OCL_QMSS_H_
#define _OCL_QMSS_H_

#include <stdbool.h>
#include <stdint.h>

// QMSS LLD includes
#include <ti/drv/qmss/qmss_drv.h>


/**
 * OCL QMSS Queues
 */
typedef struct
{
    Qmss_QueueHnd freeQ; // ocl_descriptorAlloc/Free from this queue
    Qmss_QueueHnd wgQ;   // queue for workgroup & flush descriptors 
    Qmss_QueueHnd exitQ; // queue for exit descriptors
} OCLQueues_t;

extern OCLQueues_t     ocl_queues;


/**
 * Allocate a buffer from the free queue
 */
static inline void* ocl_descriptorAlloc(void)
{
    // The popped descriptor may have its size encoded in lower 4 bits.
    // QMSS_DESC_PTR clears these bits out.
    return (void*)QMSS_DESC_PTR(Qmss_queuePop(ocl_queues.freeQ));
}


/**
 * Free a buffer allocated from the free queue
 */
static inline void ocl_descriptorFree(void* d)
{
    Qmss_queuePushDesc (ocl_queues.freeQ, (void*)d);
}

/**
 * Push an event onto the tail of the specified queue
 *
 * @param qHnd  queue to push the event onto
 * @param event event to be pushed
 */
static inline void ocl_descriptorPush(Qmss_QueueHnd qHnd, void *d)
{
    Qmss_queuePushDesc (qHnd, d);
}


/**
 * Pop an event from the head of the specified queue
 *
 * @param qHnd  queue to pop the event from
 * @return popped event
 */
static inline void* ocl_descriptorPop(Qmss_QueueHnd qHnd)
{
    // The popped descriptor may have its size encoded in lower 4 bits.
    // QMSS_DESC_PTR clears these bits out.
    return (void*)QMSS_DESC_PTR(Qmss_queuePop(qHnd));
}


void ocl_exitGlobalQMSS (void);
bool ocl_initGlobalQMSS (void);
bool ocl_initLocalQMSS  (void);

void ocl_dispatch_once (void);

#endif /*  _CIO_QMSS_H_ */
