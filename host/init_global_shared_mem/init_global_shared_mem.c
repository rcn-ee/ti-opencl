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
#include <string.h>
#include <stdio.h>
#include "stdint.h"
#include "pciedrv.h"
#include "cmem_drv.h"

int main()
{
    cmem_host_buf_desc_t   buf_desc[32];
    uint32_t               dsp_start_addr;
    int                    size_of_buffer = 0x400000;

    int ret = cmem_drv_open();
    if (ret) { printf("\nERROR: dma mem driver open failed \n"); return(-1); }

    ret = cmem_drv_alloc(0, size_of_buffer, HOST_BUF_TYPE_PERSISTENT, buf_desc);
    if (ret) { printf("ERROR: contiguous memory alloc failed\n"); return -1; }

    /*-------------------------------------------------------------------------
    * configuration for pcie driver
    *------------------------------------------------------------------------*/
    pciedrv_open_config_t pciedrv_open_config;
    memset(&pciedrv_open_config, 0 , sizeof(pciedrv_open_config_t));
    pciedrv_open_config.dsp_outbound_block_size = size_of_buffer;

    ret = pciedrv_open(&pciedrv_open_config);
    if (ret) { printf("ERROR: pciedrv could not opened\n"); return -1; }

    /*-------------------------------------------------------------------------
    * Allocate dsp memory range
    *------------------------------------------------------------------------*/
    ret = pciedrv_dsp_memrange_alloc(0, 0,  &dsp_start_addr);
    if (ret) { printf("ERROR: memrange alloc failed \n"); return -1; }

    /*-------------------------------------------------------------------------
    * Map host buffer to dsp memory range
    *------------------------------------------------------------------------*/
    ret = pciedrv_map_hostbufs_to_dsp_memrange(0, 0, buf_desc, dsp_start_addr);
    if (ret) { printf("ERROR: map dsp mem range failed \n"); return -1; }

    ret = cmem_drv_close();
    if (ret) { printf("ERROR: dma mem driver could not closed\n"); return -1; }

    ret = pciedrv_close();
    if (ret) { printf("ERROR: pciedrv could not closed\n"); return -1; }
}
