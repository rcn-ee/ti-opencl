/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
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
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "device_manager_pcie.h"
#include "../error_report.h"
#include "cmem.h"

#define BOOT_ENTRY_LOCATION_ADDR   0x87FFFC
#define BOOT_MAGIC_ADDR(core) (0x10000000 | (core << 24) | 0x87FFFC)

#define ERR(status, msg) if (status) { printf("ERROR: %s\n", msg); exit(-1); }

static const char* get_board(unsigned switch_device);
static bool        wait_for_ready(int chip);


using namespace tiocl;

pciedrv_open_config_t  DeviceManagerPCIe::config;
pciedrv_device_info_t* DeviceManagerPCIe devices_info = nullptr;

DeviceManagerPCIe::DeviceManagerPCIe(uint8_t device_id, uint8_t num_cores, std::string monitor) :
device_id_(device_id), num_cores_(num_cores), monitor_(monitor)
{
    if (devices_info == nullptr)
        DeviceManagerPCIe::Initialize();
}

// Perform PCIe initialization (once for all devices on DSPC868x)
void DeviceManagerPCIe::Initialize()
{
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

    uint8_t num_devices = pciedrv_get_num_devices();

    /*-------------------------------------------------------------------------
    * Allocate space for and retrieve device info
    *------------------------------------------------------------------------*/
    devices_info = (pciedrv_device_info_t*)
                   malloc(num_devices * sizeof(pciedrv_device_info_t));
    ERR (!devices_info, "malloc failed pciedrv_devices_info_t");

    int ret = pciedrv_get_pci_info(devices_info);
    ERR(ret, "get pci info failed");
}

void DeviceManagerPCIe::Finalize()
{
    free(devices_info);
    int status = pciedrv_close();
    ERR(status, "PCIe Driver Close Error");
}

DeviceManagerPCIe::~DeviceManagerPCIe()
{
    if (devices_info != nullptr)
    {
        DeviceManagerPCIe::Finalize();
        devices_info = nullptr;
    }
}


bool DeviceManagerPCIe::Reset() const
{
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
    int ret = dnldmgr_reset_dsp(device_id_, 0, NULL, 0 , NULL);
    ERR (ret, "DSP putting in reset failed");

    const char *board  = get_board(devices_info[device_id_].switch_device);
    std::string init(installation);
    init += "/usr/share/ti/opencl/init_";
    init += board;
    init += ".out";

    void    *init_image_handle;
    uint32_t init_entry;
    ret = dnldmgr_get_image(
        const_cast<char *>(init.c_str()), &init_image_handle, &init_entry);
    ERR(ret, "Get reset image failed");

    ret = dnldmgr_reset_dsp(device_id_, 1, init_image_handle, init_entry, &bootcfg);
    ERR (ret, "DSP out of reset failed");

    dnldmgr_free_image(init_image_handle);

    /*---------------------------------------------------------------------
    * wait for reset to complete
    *--------------------------------------------------------------------*/
    ERR(!wait_for_ready(device_id_), "Reset Failed due to timeout");

    return true;
}


bool DeviceManagerPCIe::Load() const
{
    void *image_handle;
    uint32_t entry;
    ret = dnldmgr_get_image(
        const_cast<char *>(dsp_monitor(chip).c_str()), &image_handle, &entry);
    ERR(ret, "Get DSP image failed");

    ret = dnldmgr_load_image(chip, 0xFFFF, image_handle, entry, NULL);
    ERR(ret, "Download image failed");

    dnldmgr_free_image(image_handle);
}

bool DeviceManagerPCIe::Run() const
{
    return true;
}

const DeviceManager*
tiocl::DeviceManagerFactory::CreateDeviceManager(uint8_t device_id, uint8_t num_cores,
                                                 const std::string &binary)
{
    return new DeviceManagerPCIe(device_id, num_cores, binary);
}


// Convert pci data into a recognizable board name for a device
static const char *get_board(unsigned switch_device)
{
    switch (switch_device)
    {
        case 0x8624: return "dspc8681";
        case 0x8748: return "dspc8682";
        default    : ERR(1, "Unsupported device"); return "unknown";
    }
}

static bool wait_for_ready(int chip)
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
