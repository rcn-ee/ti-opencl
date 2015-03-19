#include "driver.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include "ocl_debug.h"

#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }
Driver* Driver::pInstance = 0;
Cmem*   Cmem::pInstance = 0;

/*-------------------------------------------------------------------------
* Thread safe instance function for singleton behavior
*------------------------------------------------------------------------*/
Driver* Driver::instance () 
{
    static Mutex Driver_instance_mutex;
    Driver* tmp = pInstance;

    __sync_synchronize();

    if (tmp == 0) 
    {
        ScopedLock lck(Driver_instance_mutex);

        tmp = pInstance;
        if (tmp == 0) 
        {
            tmp = new Driver;
            __sync_synchronize();
            pInstance = tmp;
        }
    }
    return tmp;
}

int32_t Driver::open()         
{ 
    Lock lock(this); 
    pciedrv_open_config_t config;

    config.dsp_outbound_reserved_mem_size = 0;
    config.start_dma_chan_num             = 0;
    config.num_dma_channels               = 4;
    config.start_param_set_num            = 0;
    config.num_param_sets                 = 32;
    config.dsp_outbound_block_size        = 0x400000;
    config.max_dma_transactions           = 256;

    int status = pciedrv_open(&config);  
    ERR(status, "PCIe Driver Open Error");

    /*-------------------------------------------------------------------------
    * Create hex files to init device with
    *------------------------------------------------------------------------*/
#include "init_hex.h"
#include "monitor_hex.h"

    char init_template[]    =  "/tmp/opencl_initXXXXXX";
    char monitor_template[] =  "/tmp/opencl_monitorXXXXXX";
    int iFile = mkstemp(init_template);
    int mFile = mkstemp(monitor_template);

    FILE *fi = fopen (init_template,    "w");
    FILE *fm = fopen (monitor_template, "w");

    fwrite (embed_init_hex_h,    sizeof(embed_init_hex_h), 1, fi);
    fwrite (embed_monitor_hex_h, sizeof(embed_monitor_hex_h), 1, fm);

    fclose(fi);
    fclose(fm);

    /*---------------------------------------------------------------------
    * ASW TODO - should not hard code 4 dsps here.  Perhaps API reconfig.
    *--------------------------------------------------------------------*/
#if 0
    for (int dsp_id = 0; dsp_id < 4; dsp_id++)
    {
        status = dnldmgr_reset_dsp(dsp_id, FLAG_DSP_RESET, NULL, 0);
        ERR(status, "PCIe Driver Reset Error");

        status = dnldmgr_reset_dsp(dsp_id, FLAG_DSP_OUTOF_RESET, 
        //"/home/al/ti/desktop-linux-sdk_01_00_00_02/demos/dsp_images/init.hex",
         init_template, 
         0x00860000);
        ERR(status, "PCIe Driver Out Of Reset Error");
    }

    for (int dsp_id = 0; dsp_id < 4; dsp_id++)
    {
        status = dnldmgr_load_image_uniform(dsp_id, 
                "/home/al/clover/dsp_monitor/monitor.hex",
                //monitor_template, 
                0x81000000);
        ERR(status, "PCIe DSP Monitor Load Error");
    }
#endif

    /*-------------------------------------------------------------------------
    * Remove init hex files from system
    *------------------------------------------------------------------------*/
    struct stat statbuf;
    if (stat(init_template, &statbuf) == 0)
        if (remove(init_template) != 0)
            printf("File \"%s\" cannot be removed", init_template);

    if (stat(monitor_template, &statbuf) == 0)
        if (remove(monitor_template) != 0)
            printf("File \"%s\" cannot be removed", monitor_template);

    Cmem::instance(); // Prime the setup of cmem

    ocl_debug("End DSP Setup");

    return 0;
}

int32_t Driver::close()         
{
    Lock lock(this); 
    int status = pciedrv_close();  
    ERR(status, "PCIe Driver Close Error");
    return 0;
}

int32_t Driver::write(int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf,uint32_t size)
{ 
    Lock lock(this); 
    int status = pciedrv_dsp_write(dsp_id, addr, buf, size);
    ERR(status, "PCIe Driver Write Error");
    return 0;
}

int32_t Driver::read(int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf, uint32_t size)
{ 
    Lock lock(this); 
    int status = pciedrv_dsp_read(dsp_id, addr, buf, size);
    ERR(status, "PCIe Driver Read Error");
    return 0;
}

int32_t Driver::dma_write(int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf, 
                          uint32_t size)
{ 
    /*-------------------------------------------------------------------------
    * Regular reads under 8k are faster than DMA reads (may change)
    *------------------------------------------------------------------------*/
    if (size < 24 * 1024) return write(dsp_id, addr, buf, size); 

    Lock lock(this); 
    Cmem::instance()->dma_write(dsp_id, addr, buf, size); 
    return 0;
}

int32_t Driver::dma_read(int32_t dsp_id, DSPDevicePtr addr, uint8_t *buf, 
                         uint32_t size)
{ 
    Lock lock(this); 
    Cmem::instance()->dma_read(dsp_id, addr, buf, size); 
    return 0;
}


/*=============================================================================
* C M E M
*============================================================================*/
#define CEIL_DIVIDE(x,y) (((x) + (y) - 1) / y)

/*-------------------------------------------------------------------------
* Thread safe instance function for singleton behavior
*------------------------------------------------------------------------*/
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

void Cmem::open()         
{ 
    Lock lock(this); 

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

void Cmem::close()        
{ 
    Lock lock(this); 

    bufmgrDelete(&DmaBufPool); // void return;

    int status = cmem_drv_free(MAX_NUM_HOST_DSP_BUFFERS, HOST_BUF_TYPE_DYNAMIC, 
                           buf_desc);
    ERR(status, "DMA Contiguous Memory Driver Free Error");

    status = cmem_drv_close();
    ERR(status, "DMA Contiguous Memory Driver Close Error");
}


void Cmem::dma_write(int32_t dsp_id, uint32_t addr, uint8_t *buf, uint32_t size)
{
    int32_t                ret_val;
    cmem_host_buf_desc_t   host_buf_desc[MAX_CONTIGUOUS_XFER_BUFFERS];
    cmem_host_frame_desc_t host_frame_desc;

    uint32_t remaining_cpy_size, cpy_size, transfer_size, offset;
    uint32_t num_remain_buffers, num_buffers, transfer_buffers, remaining_size;
    uint32_t trans_id;

    int i;

    /*-------------------------------------------------------------------------
     * Calculate total number of host buffers required to fit the data
     *------------------------------------------------------------------------*/
    num_buffers    = CEIL_DIVIDE(size, HOST_CMEM_BUFFER_SIZE);
    remaining_size = size;
    offset         = 0;

    /*-------------------------------------------------------------------------
     * Initiate one transfer at a time based on what fits within the allowed
     * contiguous buffers per DMA transaction
     *------------------------------------------------------------------------*/
    while (num_buffers) 
    {
        if (num_buffers > MAX_CONTIGUOUS_XFER_BUFFERS) 
        {
            transfer_buffers = MAX_CONTIGUOUS_XFER_BUFFERS;
            transfer_size    = transfer_buffers * HOST_CMEM_BUFFER_SIZE;
        }
        else 
        {
            transfer_buffers = num_buffers;
            transfer_size    = remaining_size;
        }

        /*---------------------------------------------------------------------
        * Allocate Host buffer
        *--------------------------------------------------------------------*/
        for (i = 0; i < transfer_buffers; i++) 
        {
            ret_val = bufmgrAlloc(DmaBufPool, 1, &host_buf_desc[i]);
            ERR(ret_val, "dma buffer allocation failed");
        }

        /*---------------------------------------------------------------------
        * Copy from buffer content into DMA buffers
        *--------------------------------------------------------------------*/
        remaining_cpy_size = transfer_size;

        for (i = 0; i < transfer_buffers; i++) 
        {
            cpy_size = std::min(remaining_cpy_size, host_buf_desc[i].length);

            memcpy(host_buf_desc[i].userAddr, buf + offset, cpy_size);
            remaining_cpy_size -= cpy_size;
            offset             += cpy_size;
        }

        /*---------------------------------------------------------------------
        * Populate details of data in frame descriptor
        *--------------------------------------------------------------------*/
        host_frame_desc.bufDescP         = host_buf_desc;
        host_frame_desc.numBuffers       = transfer_buffers;
        host_frame_desc.frameStartOffset = 0;
        host_frame_desc.frameSize        = transfer_size;

        /*---------------------------------------------------------------------
         * Initiate DMA
         *--------------------------------------------------------------------*/
        ret_val = pciedrv_dma_write_initiate(dsp_id, addr, &host_frame_desc, 
                PCIEDRV_DMA_XFER_BLOCKING, &trans_id);
        ERR(ret_val, "DMA initiate failed");

        /*---------------------------------------------------------------------
         * Wait for dma complete
         *   Not used as we are using Blocking call currently
         *--------------------------------------------------------------------*/
        //while(pciedrv_dma_check(dsp_id, trans_id));

        /*---------------------------------------------------------------------
         * Free the Buffer descriptors
         *--------------------------------------------------------------------*/
        for (i = 0; i < transfer_buffers; i++) 
        {
            ret_val = bufmgrFreeDesc(DmaBufPool, &host_buf_desc[i]);
            ERR(ret_val, "dma buffer free failed");
        }

        num_buffers    -= transfer_buffers;
        remaining_size -= transfer_size;
        addr           += transfer_size;
    }
}

void Cmem::dma_read(int32_t dsp_id, uint32_t addr, uint8_t *buf, uint32_t size)
{
    int32_t ret_val;
    cmem_host_buf_desc_t   host_buf_desc[MAX_CONTIGUOUS_XFER_BUFFERS];
    cmem_host_frame_desc_t host_frame_desc;

    uint32_t remaining_cpy_size, cpy_size, transfer_size, offset;
    uint32_t num_remain_buffers, num_buffers, transfer_buffers, remaining_size;
    uint32_t trans_id;
    int i;

    /*-------------------------------------------------------------------------
    * Calculate total number of host buffers required to fit the data
    *------------------------------------------------------------------------*/
    num_buffers    = CEIL_DIVIDE(size, HOST_CMEM_BUFFER_SIZE);
    remaining_size = size;
    offset         = 0;

    /*-------------------------------------------------------------------------
    * Initiate one transfer at a time based on what fits within the allowed
    * contiguous buffers per DMA transaction
    *------------------------------------------------------------------------*/
    while (num_buffers) 
    {
        if (num_buffers > (MAX_CONTIGUOUS_XFER_BUFFERS)) 
        {
            transfer_buffers = (MAX_CONTIGUOUS_XFER_BUFFERS);
            transfer_size    = transfer_buffers * HOST_CMEM_BUFFER_SIZE;
        }
        else 
        {
            transfer_buffers = num_buffers;
            transfer_size    = remaining_size;
        }

        /*---------------------------------------------------------------------
        * Allocate Host buffer
        *--------------------------------------------------------------------*/
        for (i = 0; i < transfer_buffers; i++) 
        {
            ret_val = bufmgrAlloc(DmaBufPool, 1, &host_buf_desc[i]);
            ERR(ret_val, "dma buffer allocation failed");
        }

        /*---------------------------------------------------------------------
        * Populate details of data in frame descriptor
        *--------------------------------------------------------------------*/
        host_frame_desc.bufDescP         = host_buf_desc;
        host_frame_desc.numBuffers       = transfer_buffers;
        host_frame_desc.frameStartOffset = 0;
        host_frame_desc.frameSize        = transfer_size;

        /*---------------------------------------------------------------------
        * Initiate DMA
        *--------------------------------------------------------------------*/
        ret_val = pciedrv_dma_read_initiate(dsp_id, addr, &host_frame_desc, 
                                          PCIEDRV_DMA_XFER_BLOCKING, &trans_id);
        ERR(ret_val, "DMA initiate failed");

        /*---------------------------------------------------------------------
         * Wait for dma complete
         *   Not used as we are using Blocking call currently
         *--------------------------------------------------------------------*/
        //while(pciedrv_dma_check(dsp_id, trans_id));

        /*---------------------------------------------------------------------
        * Copy from dma buffers into buffer
        *--------------------------------------------------------------------*/
        remaining_cpy_size = transfer_size;
        for (i = 0; i < transfer_buffers; i++)
        {
            cpy_size = std::min(remaining_cpy_size, host_buf_desc[i].length);

            memcpy (buf + offset, host_buf_desc[i].userAddr, cpy_size);
            remaining_cpy_size -= cpy_size;
            offset             += cpy_size;
        }

        /*---------------------------------------------------------------------
        * Free Buffer Descriptors
        *--------------------------------------------------------------------*/
        for (i = 0; i< transfer_buffers; i++) 
        {
            ret_val = bufmgrFreeDesc(DmaBufPool, &host_buf_desc[i]);
            ERR(ret_val, "dma buffer free failed");
        }

        num_buffers    -= transfer_buffers;
        remaining_size -= transfer_size;
        addr           += transfer_size;
    }
}


