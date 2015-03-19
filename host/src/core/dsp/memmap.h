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
#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }

class DSP_MappedMem
{
  public:
    DSP_MappedMem(uint32_t dsp_id, uint32_t size) 
        : p_size(size), p_dsp_id(dsp_id), p_dsp_addr(0)
          p_num_buffers(CEIL_DIVIDE(size, HOST_CMEM_BUFFER_SIZE))
    { 
        p_buffers = new [p_num_buffers] cmem_host_buf_desc_t; 
        ERR(!p_buffers, "Cannot allocate host memory for a DSP Mapped Region");

        int status
        for (int i = 0; i< num_buffers; i++) 
        {
            status = bufmgrAlloc(DmaBufPool, 1, &p_buffers[i]);
            ERR(status, "Cannot allocate CMEM pool for a DSP Mapped Region");
        }

        /*---------------------------------------------------------------------
        * Allocate DSP range
        *--------------------------------------------------------------------*/
        status = pciedrv_dsp_memrange_alloc(dsp_id, size, p_dsp_addr);
        ERR(status, "PCIe driver dsp memrange alloc failed");

        /*---------------------------------------------------------------------
        * Map Input buffers to dsp range
        *--------------------------------------------------------------------*/
        status = pciedrv_map_bufs_to_dsp_memrange(dsp_id, num_buffers, 
                      p_buffers, (uint32_t) p_dsp_addr);
        ERR(status, "PCIe driver dsp map bufs to memrange failed");
    }

    ~DSP_MappedMem() 
    { 
        /*---------------------------------------------------------------------
        * Free DSP range
        *--------------------------------------------------------------------*/
        int status = pciedrv_dsp_memrange_free(dsp_id, size, p_dsp_addr);
        ERR(status, "PCIe driver dsp memrange free failed");

        for (int i = 0; i< num_buffers; i++) 
        {
            status = bufmgrFreeDesc(DmaBufPool, &p_buffers[i]);
            ERR(status, "Cannot free CMEM pool for a DSP Mapped Region");
        }

        delete [p_num_buffers] p_buffers; 
    }

    void copy_in(void* p, uint32_t size) 
    {
        ERR(size > p_size, "DSP Mapped region input overflow");

        uint32_t remaining_size = size;
        uint32_t offset         = 0;

        for (int i = 0; remaining_size; i++)
        {
            int chunk_size = std::min(remaining_size, p_buffers[i].length);

            memcpy(p_buffers[i].user_addr, p + offset, chunk_size);
            
            remaining_size -= chunk_size;
            offset         += chunk_size;
        }
    }

    void copy_out(void* p, uint32_t size) 
    {
        ERR(size > p_size, "DSP Mapped region output underrflow");

        uint32_t remaining_size = size;
        uint32_t offset         = 0;

        for (int i = 0; remaining_size; i++) 
        { 
            int chunk_size = std::min(remaining_size, p_buffers[i].length);

            memcpy(p + offset, p_buffers[i].user_addr, chunk_size);
            
            remaining_size -= chunk_size;
            offset         += chunk_size;
        }
    }

  private:
    uint32_t              p_size;
    uint32_t              p_dsp_id;
    uint32_t              p_dsp_addr;
    uint32_t              p_num_buffers;
    cmem_host_buf_desc_t *p_buffers;
};
