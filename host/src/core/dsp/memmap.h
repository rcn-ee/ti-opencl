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
