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
#include "driver.h"
#include "cmem.h"
#include <deque>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <string>

#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }
#define BOOT_ENTRY_LOCATION_ADDR   0x87FFFC
#define BOOT_MAGIC_ADDR(core) (0x10000000 | (core << 24) | 0x87FFFC)

Driver* Driver::pInstance = 0;

/******************************************************************************
* Thread safe instance function for singleton behavior
******************************************************************************/
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

/******************************************************************************
* Convert pci data into a recognizable board name for a device
******************************************************************************/
const char *get_board(unsigned switch_device)
{
    switch (switch_device)
    {
        case 0x8624: return "dspc8681";
        case 0x8748: return "dspc8682";
        default    : ERR(1, "Unsupported device"); return "unknown";
    }
}

std::string Driver::dsp_monitor(int dsp)
{
    std::string get_ocl_dsp();

    char *installation = getenv("TI_OCL_INSTALL");
    if (! installation)  ERR(1, "TI_OCL_INSTALL env variable not set");

    return installation + get_ocl_dsp() + "/dsp.out";
}

int Driver::cores_per_dsp(int dsp)
{
    return 8;
}


/******************************************************************************
* wait_for_ready
******************************************************************************/
bool Driver::wait_for_ready(int chip)
{
    int execution_wait_count = 0;
    int n_cores = cores_per_dsp(chip);

    while (1)
    {
        int core;
        for (core = 0; core < n_cores; core++)
        {
            uint32_t boot_entry_value;
            int ret = pciedrv_dsp_read(chip, 
                              ((0x10 + core) << 24) + BOOT_ENTRY_LOCATION_ADDR,
                              (unsigned char *) &boot_entry_value, 4);
            ERR(ret, "pciedrv_dsp_read failed");

            if (boot_entry_value != 0) break;
        }

        if (core == n_cores) return true; 
        if (++execution_wait_count > 1000)    return false;

        usleep(1000);
    }
}

// char *get_ocl_install();
void Driver::reset_and_load(int chip)
{
    // char *installation = get_ocl_install();
    char *installation = getenv("TI_OCL_INSTALL");
    if (! installation)  ERR(1, "TI_OCL_INSTALL env variable not set");

    /*------------------------------------------------------------------------
    * Determine DSP speed. 1 Ghz by default. Set Env Var for  1.25Ghz Oper
    *-----------------------------------------------------------------------*/
    uint32_t pll_multiplier = 0x00000014; // 1.00 Ghz by default
    if (getenv("TI_OCL_DSP_1_25GHZ")) pll_multiplier = 0x00000019; 

    /*-------------------------------------------------------------------------
    * Configure boot config 
    *------------------------------------------------------------------------*/
    uint32_t   bootcfg_words[]= { 0xBABEFACE, pll_multiplier };
    boot_cfg_t bootcfg = { 0x86FF00, sizeof(bootcfg_words), bootcfg_words};

    /*-------------------------------------------------------------------------
    * reset the devices
    *------------------------------------------------------------------------*/
    int ret = dnldmgr_reset_dsp(chip, 0, NULL, 0 , NULL);
    ERR (ret, "DSP putting in reset failed");

    const char *board  = get_board(pDevices_info[chip].switch_device);
    std::string init(installation);
    init += "/usr/share/ti/opencl/init_";
    init += board;
    init += ".out";
 
    void    *init_image_handle;
    uint32_t init_entry;
    ret = dnldmgr_get_image(init.c_str(), &init_image_handle, &init_entry);
    ERR(ret, "Get reset image failed");
 
    ret = dnldmgr_reset_dsp(chip, 1, init_image_handle, init_entry, &bootcfg);
    ERR (ret, "DSP out of reset failed");
 
    dnldmgr_free_image(init_image_handle);

    /*---------------------------------------------------------------------
    * wait for reset to complete
    *--------------------------------------------------------------------*/
    ERR(!wait_for_ready(chip), "Reset Failed due to timeout");

    /*-------------------------------------------------------------------------
    * Load monitor on the devices
    *------------------------------------------------------------------------*/
    void *image_handle;
    uint32_t entry;
    ret = dnldmgr_get_image(dsp_monitor(chip).c_str(), &image_handle, &entry);
    ERR(ret, "Get DSP image failed");

    ret = dnldmgr_load_image(chip, 0xFFFF, image_handle, entry, NULL);
    ERR(ret, "Download image failed");

    dnldmgr_free_image(image_handle);
}

void* Driver::create_image_handle(int chip)
{
    std::string monitor = dsp_monitor(chip);

    void *image_handle;
    uint32_t entry;

    int ret = dnldmgr_get_image(monitor.c_str(), &image_handle, &entry);
    ERR(ret, "Get DSP image failed");

    return image_handle;
}

/******************************************************************************
* Driver::open
******************************************************************************/
int32_t Driver::open()         
{ 
    Lock lock(this); 

    memset((void*)&config, 0, sizeof(pciedrv_open_config_t));
    config.dsp_outbound_reserved_mem_size = 0;
    config.start_dma_chan_num             = 0;
    config.num_dma_channels               = 4;
    config.start_param_set_num            = 0;
    config.num_param_sets                 = 32;
    config.dsp_outbound_block_size        = 0x400000;
    config.max_dma_transactions           = 256;

    int status = pciedrv_open(&config);  
    ERR(status, "PCIe Driver Open Error");

    pNum_dsps = pciedrv_get_num_devices();

    /*-------------------------------------------------------------------------
    * Allocate space for and retrieve device info 
    *------------------------------------------------------------------------*/
    pDevices_info = (pciedrv_device_info_t*)
                   malloc(pNum_dsps * sizeof(pciedrv_device_info_t));
    ERR (!pDevices_info, "malloc failed pciedrv_devices_info_t");
  
    int ret = pciedrv_get_pci_info(pDevices_info);
    ERR(ret, "get pci info failed");

    Cmem::instance(); // Prime the setup of cmem
    return 0;
}

/******************************************************************************
* Driver::close()         
******************************************************************************/
int32_t Driver::close()         
{
    Lock lock(this); 
    free (pDevices_info);
    int status = pciedrv_close();  
    ERR(status, "PCIe Driver Close Error");
    return 0;
}


/******************************************************************************
* Driver::write
******************************************************************************/
int32_t Driver::write(int32_t dsp_id, DSPDevicePtr64 addr, uint8_t *buf, 
                      uint32_t size)
{ 
    int n_cores = cores_per_dsp(dsp_id);

    /*-------------------------------------------------------------------------
    * if the write is to L2, then write for each core
    *------------------------------------------------------------------------*/
    if ((addr >> 20) == 0x008)
    {
        for (int core = 0; core < n_cores; core++)
        {
#if !defined (DEVICE_AM57)
            write_core(dsp_id, ((0x10 + core) << 24) + addr, buf, size);
#else
            write_core(dsp_id, ((0x80 + core) << (3+20)) + addr, buf, size);
#endif
        }
    }
    else
        write_core(dsp_id, addr, buf, size);
}


/******************************************************************************
* Driver::write
******************************************************************************/
int32_t Driver::write_core(int32_t dsp_id, DSPDevicePtr64 addr64, uint8_t *buf, 
                           uint32_t size)
{ 
    DSPDevicePtr addr = (DSPDevicePtr) addr64;
    /*-------------------------------------------------------------------------
    * Regular writes under 24k are faster than DMA writes (may change)
    *------------------------------------------------------------------------*/
    if (size < 24 * 1024) 
    {
        int status = pciedrv_dsp_write(dsp_id, addr, buf, size);
        ERR(status, "PCIe Driver Write Error");
        return 0;
    }

    Lock lock(this); 
    Cmem::instance()->dma_write(dsp_id, addr, buf, size); 
    return 0;
}

void*   Driver::map(Coal::DSPDevice *device,DSPDevicePtr64 addr, uint32_t sz,
                    bool is_read, bool allow_fail)
{
    void *host_addr = malloc(sz);
    if (host_addr == NULL && !allow_fail) ERR(1, "Unable to map a dsp address");
    if (host_addr && is_read)
        read((int32_t)device->dspID(), addr, (uint8_t *)host_addr, sz);
    return host_addr;
}

int32_t Driver::unmap(Coal::DSPDevice *device, void *host_addr,
                      DSPDevicePtr64 buf_addr, uint32_t sz, bool is_write)
{
    if (host_addr && is_write)
        write((int32_t)device->dspID(), buf_addr, (uint8_t *)host_addr, sz);
    if (host_addr)  free(host_addr);
    return 0;
}

bool Driver::cacheInv(DSPDevicePtr64 addr, void *host_addr, uint32_t sz)
{
    return true;
}

bool Driver::cacheWb(DSPDevicePtr64 addr, void *host_addr, uint32_t sz)
{
    return true;
}

bool Driver::cacheWbInv(DSPDevicePtr64 addr, void *host_addr, uint32_t sz)
{
    return true;
}

bool Driver::cacheWbInvAll()
{
    return true;
}


/******************************************************************************
* Driver::read
******************************************************************************/
int32_t Driver::read(int32_t dsp_id, DSPDevicePtr64 addr64, uint8_t *buf, 
                     uint32_t size)
{ 
    Lock lock(this); 

    DSPDevicePtr addr = (DSPDevicePtr) addr64;
    Cmem::instance()->dma_read(dsp_id, addr, buf, size); 
    return 0;
}

/******************************************************************************
* Driver::get_symbol
******************************************************************************/
DSPDevicePtr Driver::get_symbol(void* image_handle, const char *name)
{
    DSPDevicePtr addr;
    int ret = dnldmgr_get_symbol_address(image_handle, name, &addr);
    if (ret) { printf("ERROR: Get symbol failed\n"); exit(-1); } 

    return addr;
}

/******************************************************************************
* Driver::free_image_handle
******************************************************************************/
void Driver::free_image_handle(void *handle) 
{ 
    dnldmgr_free_image(handle); 
}

/******************************************************************************
* Driver::split_ddr_heap: partition DDR to persistent mapping part (heap1)
*                                       and on demand mapping part (heap2)
******************************************************************************/
void Driver::split_ddr_memory(DSPDevicePtr64  addr,  uint64_t  size,
                              DSPDevicePtr64& addr1, uint64_t& size1,
                              DSPDevicePtr64& addr2, uint64_t& size2,
                                                     uint64_t& size3)
{
    addr1 = addr;
    size1 = size;
    addr2 = 0;
    size2 = 0;
    size3 = 0;
}

/******************************************************************************
* Driver::cmem_setup
* Driver::shmem_configure
******************************************************************************/
void Driver::cmem_init(DSPDevicePtr64 *addr1, uint64_t *size1,
                       DSPDevicePtr   *addr2, uint32_t *size2,
                       DSPDevicePtr64 *addr3, uint64_t *size3)
{
}

void Driver::cmem_exit()
{
}

void Driver::shmem_configure(DSPDevicePtr64 addr, uint64_t size, int cmem_block)
{
}

