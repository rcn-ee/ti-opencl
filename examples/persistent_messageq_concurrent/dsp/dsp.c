#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dsp_c.h>
#include <ti/sysbios/knl/Task.h>
#include "heapbuf.h"
#include "messageq.h"
#include "shared.h"

#define MESSAGES 500

/******************************************************************************
* Called from OpenCL kernel wrapper
******************************************************************************/
void ccode(uint32_t qid, uint32_t *completion_code)
{
    HeapBuf_Handle   heap = heapbuf_construct (1, sizeof(MessageQ_MsgHeader));
    if (!heap) { *completion_code = APP_FAIL; return; }

    MessageQ_QueueId msgq = messageq_construct(qid, heap);
    if (!msgq) { *completion_code = APP_FAIL; return; }

    MessageQ_registerHeap(heap, 1);

    int i;
    for (i = 0; i < MESSAGES; i++) 
    { 
        Task_sleep(1);  // simulate 1 ms of computation load
        messageq_send(msgq, i); 
    }
    messageq_send(msgq, 0xFFFFu);

    messageq_destruct(msgq);
    heapbuf_destruct (heap);

    printf("DSP sent %d messages\n", MESSAGES);
    *completion_code = APP_OK;
}
