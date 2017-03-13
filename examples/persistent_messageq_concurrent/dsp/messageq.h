#ifndef _MESSAGEQ_H_
#define _MESSAGEQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ti/sysbios/BIOS.h"
#include "ti/ipc/MessageQ.h"

static uint32_t HeapId=1;

static struct 
{
    uint32_t heap_id;
} messageq_data;

/******************************************************************************
* construct_messageq
******************************************************************************/
static MessageQ_QueueId messageq_construct2(String QueueName, HeapBuf_Handle heap)
{
    messageq_data.heap_id = HeapId++;

    MessageQ_registerHeap(heap, messageq_data.heap_id);

    MessageQ_QueueId qid;

    int status = MessageQ_open(QueueName, &qid);
    if (status != MessageQ_S_SUCCESS) { printf("MsgQ_open failed %u\n", status); return 0; }

    return qid;
}

static MessageQ_QueueId messageq_construct(MessageQ_QueueId qid, HeapBuf_Handle heap)
{
    messageq_data.heap_id = HeapId++;
    MessageQ_registerHeap(heap, messageq_data.heap_id);
    return qid;
}

/******************************************************************************
* destruct_messageq
******************************************************************************/
static void messageq_destruct(MessageQ_QueueId qid)
{
    MessageQ_close   (&qid);
    MessageQ_unregisterHeap(messageq_data.heap_id);
}

/******************************************************************************
* send_messageq
******************************************************************************/
static void messageq_send(MessageQ_QueueId qid, int value)
{
    MessageQ_Msg msg = MessageQ_alloc(messageq_data.heap_id, sizeof(MessageQ_MsgHeader));
    if (!msg) { printf("MsgQ_alloc failed\n"); return; }

    MessageQ_setMsgId(msg, value);
    int status = MessageQ_put(qid, msg);

    if (status != MessageQ_S_SUCCESS) printf("MsgQ_put failed %u\n", status); 
}

#endif // _MESSAGEQ_H_
