#ifndef _HEAPBUF_H_
#define _HEAPBUF_H_
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ti/sysbios/BIOS.h"
#include "ti/sysbios/heaps/HeapBuf.h"

/******************************************************************************
* construct_heapbuf
******************************************************************************/
static struct 
{
    uint32_t heapsize;
    char *   buf;
} heapbuf_data;

/******************************************************************************
* construct_heapbuf
******************************************************************************/
static HeapBuf_Handle heapbuf_construct(uint32_t num_msgs, uint32_t msg_size)
{
    heapbuf_data.heapsize = num_msgs * msg_size;
    heapbuf_data.buf      = malloc(heapbuf_data.heapsize); 
    if (!heapbuf_data.buf) { printf("Heap create failed\n"); return 0; }

    HeapBuf_Params prms;
    HeapBuf_Params_init(&prms);

    prms.blockSize = msg_size;
    prms.numBlocks = num_msgs;
    prms.bufSize   = heapbuf_data.heapsize;
    prms.buf       = heapbuf_data.buf;

    HeapBuf_Handle heap = HeapBuf_create(&prms, 0);
    if (!heap) { printf("Heap create failed\n"); return 0; }

    return heap;
}

/******************************************************************************
* destruct_heapbuf
******************************************************************************/
static void heapbuf_destruct(HeapBuf_Handle heap)
{
    HeapBuf_delete(&heap);
    free(heapbuf_data.buf);
}

/******************************************************************************
* destruct_heapbuf
******************************************************************************/
static uint32_t heapbuf_size()
{
    return heapbuf_data.heapsize;
}

#endif // _HEAPBUF_H_
