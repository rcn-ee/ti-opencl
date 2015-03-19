/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#include "cmem.h"
#include <deque>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <string>

#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }

Cmem* Cmem::pInstance   = 0;

/*=============================================================================
* C M E M
*============================================================================*/
#define CEIL_DIVIDE(x,y) (((x) + (y) - 1) / y)

/******************************************************************************
* Thread safe instance function for singleton behavior
******************************************************************************/
Cmem* Cmem::instance() 
{
    static Mutex Cmem_instance_mutex;
    Cmem* tmp = pInstance;

    __sync_synchronize();

    if (tmp == 0) 
    {
        ScopedLock lck(Cmem_instance_mutex);

        tmp = pInstance;
        if (tmp == 0) 
        {
            tmp = new Cmem;
            __sync_synchronize();
            pInstance = tmp;
        }
    }
    return tmp;
}

/******************************************************************************
* Cmem::open()         
******************************************************************************/
void Cmem::open()         
{ 
    int status = cmem_drv_open();
    ERR(status, "DMA Contiguous Memory Driver Open Error");

    status = cmem_drv_free(0, HOST_BUF_TYPE_DYNAMIC, buf_desc);
    ERR(status, "DMA Contiguous Memory Free Error");

    status = cmem_drv_alloc(MAX_NUM_HOST_DSP_BUFFERS, HOST_CMEM_BUFFER_SIZE, 
                   HOST_BUF_TYPE_DYNAMIC, buf_desc);
    ERR(status, "DMA Contiguous Memory Alloc Error");

    status = bufmgrCreate(&DmaBufPool, MAX_NUM_HOST_DSP_BUFFERS, buf_desc);
    ERR(status, "DMA Buffer manager Create Error");
}

/******************************************************************************
* Cmem::close()        
******************************************************************************/
void Cmem::close()        
{ 
    bufmgrDelete(&DmaBufPool); 

    int status = cmem_drv_free(MAX_NUM_HOST_DSP_BUFFERS, HOST_BUF_TYPE_DYNAMIC, 
                           buf_desc);
    ERR(status, "DMA Contiguous Memory Driver Free Error");

    status = cmem_drv_close();
    ERR(status, "DMA Contiguous Memory Driver Close Error");
}


/******************************************************************************
* The dma to the dsp memory system can only occur from contiguous memory, i.e. 
* cmem.  CMEM buffers are currently limited to 4M, the algorithm is to 
* copy the general buffer in 4M chunks into CMEM 4M buffers.  Then we are able 
* to chain 2 4M buffer writes per DMA initiate. As a result, we will have 
* ceil ( size / 8M ) dma transfers initiated by the routine. to make it 
* concrete at 48M buffer dma, will result in:
*       12 memcpy calls of 4M each,
*       12 CMEM buffers allocated of 4M each
*       6 dma_initiates each with 2 - 4M buffers
*
* The algorithm is based one the MAX_CONTIGUOUS_XFER_BUFFERS and 
* HOST_CMEM_BUFFER_SIZE macros.  Currently they are 2 and 4M.
******************************************************************************/
void Cmem::dma_write(int32_t dsp_id, uint32_t addr, uint8_t *buf, uint32_t size)
{
    static uint32_t      trans_id = 0;
    uint32_t             start_trans_id = trans_id;
    int32_t              ret_val;
    std::deque<uint32_t> dma_ids;

    uint32_t simul_dmas       = 4;
    uint32_t cmem_buffer_size = HOST_CMEM_BUFFER_SIZE;
    uint32_t tot_buffers      = CEIL_DIVIDE(size, cmem_buffer_size);
    uint32_t circ_buffers     = std::min(simul_dmas, tot_buffers);
    uint32_t last_buffer_size = size - ((tot_buffers-1) * cmem_buffer_size);

    cmem_host_buf_desc_t   *host_buf_desc   = 
                                  new cmem_host_buf_desc_t[circ_buffers];

    cmem_host_frame_desc_t *host_frame_desc = 
                                  new cmem_host_frame_desc_t[circ_buffers];

    /*---------------------------------------------------------------------
    * Allocate Host CMEM buffers
    *--------------------------------------------------------------------*/
    for (int i = 0; i < circ_buffers; i++) 
    {
        ret_val = bufmgrAlloc(DmaBufPool, 1, &host_buf_desc[i]);
        ERR(ret_val, "dma buffer allocation failed");
        host_frame_desc[i].bufDescP         = &host_buf_desc[i];
        host_frame_desc[i].numBuffers       = 1;
        host_frame_desc[i].frameStartOffset = 0;
        host_frame_desc[i].frameSize        = cmem_buffer_size;
    }

    /*-------------------------------------------------------------------------
     * Initiate one transfer at a time based on what fits within the allowed
     * contiguous buffers per DMA transaction
     *------------------------------------------------------------------------*/
    for (int i = 0; i < tot_buffers; ++i)
    {
        int circ_i = i % simul_dmas;
        int offset = i * cmem_buffer_size;

        cmem_host_buf_desc_t &buf_desc = host_buf_desc[circ_i];
        uint32_t cpy_size = buf_desc.length;

        if (i == tot_buffers-1) 
            host_frame_desc[circ_i].frameSize = cpy_size = last_buffer_size;

        memcpy(buf_desc.userAddr, buf + offset, cpy_size);

        /*---------------------------------------------------------------------
         * Initiate DMA
         *--------------------------------------------------------------------*/
        ret_val = pciedrv_dma_write_initiate(dsp_id, addr + offset, 
                                    &host_frame_desc[circ_i], 
                                    PCIEDRV_DMA_XFER_NON_BLOCKING, 
                                    &trans_id);
        ERR(ret_val, "DMA initiate failed");

        dma_ids.push_back(trans_id);

        if (dma_ids.size() >= simul_dmas)
        {
            while (pciedrv_dma_check(dsp_id, dma_ids.front()));
            dma_ids.pop_front();
        }
    }

    /*---------------------------------------------------------------------
     * Wait for all dmas to complete
     *--------------------------------------------------------------------*/
    for (int i = 0; i < dma_ids.size(); i++)
        while (pciedrv_dma_check(dsp_id, dma_ids[i]));

    /*---------------------------------------------------------------------
     * Free host CMEM buffers
     *--------------------------------------------------------------------*/
    for (int i = 0; i < circ_buffers; i++) 
    {
        ret_val = bufmgrFreeDesc(DmaBufPool, &host_buf_desc[i]);
        ERR(ret_val, "dma buffer free failed");
    }

    delete [] host_buf_desc;
    delete [] host_frame_desc;
}

/******************************************************************************
* Cmem::dma_read
******************************************************************************/
void Cmem::dma_read(int32_t dsp_id, uint32_t addr, uint8_t *buf, uint32_t size)
{
    cmem_host_buf_desc_t   host_buf_desc;
    cmem_host_frame_desc_t host_frame_desc;

    /*-------------------------------------------------------------------------
    * Calculate total number of host buffers required to fit the data
    *------------------------------------------------------------------------*/
    uint32_t num_buffers     = CEIL_DIVIDE(size, HOST_CMEM_BUFFER_SIZE);
    uint32_t remaining_size  = size;
    uint32_t offset          = 0;
    uint32_t transfer_size   = HOST_CMEM_BUFFER_SIZE;
    uint32_t trans_id;
    int32_t  ret_val;

    /*---------------------------------------------------------------------
    * Allocate Host buffer
    *--------------------------------------------------------------------*/
    ret_val = bufmgrAlloc(DmaBufPool, 1, &host_buf_desc);
    ERR(ret_val, "dma buffer allocation failed");

    /*---------------------------------------------------------------------
    * Populate details of data in frame descriptor
    *--------------------------------------------------------------------*/
    host_frame_desc.bufDescP         = &host_buf_desc;
    host_frame_desc.numBuffers       = 1;
    host_frame_desc.frameStartOffset = 0;
    host_frame_desc.frameSize        = transfer_size;

    /*-------------------------------------------------------------------------
    * Initiate one transfer at a time based on what fits within the allowed
    *------------------------------------------------------------------------*/
    while (num_buffers) 
    {
        if (num_buffers == 1) 
        {
            transfer_size = remaining_size;
            host_frame_desc.frameSize = transfer_size;
        }

        /*---------------------------------------------------------------------
        * Initiate DMA
        *--------------------------------------------------------------------*/
        ret_val = pciedrv_dma_read_initiate(dsp_id, addr + offset, 
                       &host_frame_desc, PCIEDRV_DMA_XFER_BLOCKING, &trans_id);
        ERR(ret_val, "DMA initiate failed");

        /*---------------------------------------------------------------------
        * Copy from dma buffers into buffer
        *--------------------------------------------------------------------*/
        memcpy (buf + offset, host_buf_desc.userAddr, transfer_size);

        num_buffers--;
        offset         += transfer_size;
        remaining_size -= transfer_size;
    }

    /*---------------------------------------------------------------------
    * Free Buffer Descriptors
    *--------------------------------------------------------------------*/
    ret_val = bufmgrFreeDesc(DmaBufPool, &host_buf_desc);
    ERR(ret_val, "dma buffer free failed");
}
