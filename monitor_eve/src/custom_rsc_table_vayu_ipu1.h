/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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

/*
 *  ======== custom_rsc_table_vayu_ipu1.h ========
 *
 *  Define the resource table entries for IPU core 1. This will be
 *  incorporated into corresponding base images, and used by the remoteproc
 *  on the host-side to allocated/reserve resources.
 *
 */

#ifndef _RSC_TABLE_IPU_H_
#define _RSC_TABLE_IPU_H_

#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <ti/ipc/remoteproc/rsc_types.h>


/* Number of entries in resource table */
#define RSC_NUM_ENTRIES         17

/* IPU Memory Map */

#define L4_PERIPHERAL_L4EMU     0x54000000
#define IPU_PERIPHERAL_L4EMU    0x74000000

#define IPU_REG_SPACE_MAP1_VIRT 0x60100000
#define IPU_REG_SPACE_MAP1_PHYS 0x40100000
#define IPU_REG_SPACE_MAP1_SIZE 0x1FF00000

#define IPU_REG_SPACE_MAP2_VIRT 0x40000000
#define IPU_REG_SPACE_MAP2_PHYS 0x40000000
#define IPU_REG_SPACE_MAP2_SIZE 0x20000000

#define L3_IVAHD_CONFIG         0x5A000000
#define IPU_IVAHD_CONFIG        0x5A000000

#define L3_IVAHD_SL2            0x5B000000
#define IPU_IVAHD_SL2           0x5B000000

#define L3_TILER_MODE_0_1       0x60000000
#define IPU_TILER_MODE_0_1      0xA0000000

#define L3_TILER_MODE_2         0x70000000
#define IPU_TILER_MODE_2        0xB0000000

#define L3_TILER_MODE_3         0x78000000
#define IPU_TILER_MODE_3        0xB8000000

#define IPU_MEM_TEXT            0x0

#define IPU_MEM_CODE            XDC_CFG_IPU1_CODE
#define IPU_MEM_DATA            XDC_CFG_IPU1_DATA
#define IPU_SR0_VIRT            XDC_CFG_SR0_VIRT
#define IPU_SR0                 XDC_CFG_SR0_VIRT
#define IPU_MEM_IPC_DATA        XDC_CFG_IPC_DATA
#define EVE_MEM_VIRT            XDC_CFG_EVE_MEM
#define EVE_MEM                 XDC_CFG_EVE_MEM

#define IPU_MEM_IPC_VRING_SIZE  (SZ_1M)
#define IPU_MEM_TEXT_SIZE       (SZ_1M)

#define IPU_MEM_CODE_SIZE       XDC_CFG_IPU1_CODE_SIZE
#define IPU_MEM_DATA_SIZE       XDC_CFG_IPU1_DATA_SIZE
//#define IPU_MEM_IPC_DATA_SIZE   XDC_CFG_IPC_DATA_SIZE
#define IPU_MEM_IPC_DATA_SIZE   (SZ_1M)
#define IPU_SR0_SIZE            XDC_CFG_SR0_SIZE
#define EVE_MEM_SIZE            XDC_CFG_EVE_MEM_SIZE

#define IPU_MEM_IPC_VRING       0x60000000
#define IPU_PHYS_MEM_IPC_VRING  0x9D000000
#define IPU_MEM_RPMSG_VRING0    0x60000000
#define IPU_MEM_RPMSG_VRING1    0x60004000

/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 */
#define IPU_RPMSG_VQ0_SIZE      256
#define IPU_RPMSG_VQ1_SIZE      256

/* flip up bits whose indices represent features we support */
#define RPMSG_IPU_C0_FEATURES   1

struct my_resource_table {
    struct resource_table base;

    UInt32 offset[RSC_NUM_ENTRIES];  /* Should match 'num' in actual definition */

    /* rpmsg vdev entry */
    struct fw_rsc_vdev rpmsg_vdev;
    struct fw_rsc_vdev_vring rpmsg_vring0;
    struct fw_rsc_vdev_vring rpmsg_vring1;

    /* text carveout entry */
    struct fw_rsc_carveout text_cout;

    /* ipcdata carveout entry */
    struct fw_rsc_carveout ipcdata_cout;

    /* ipcdata carveout entry */
    struct fw_rsc_carveout ipucode_cout;

    /* ipcdata carveout entry */
    struct fw_rsc_carveout ipudata_cout;

    /* trace entry */
    struct fw_rsc_trace trace;

    /* devmem entry */
    struct fw_rsc_devmem devmem0;

    /* devmem entry */
    struct fw_rsc_devmem devmem1;

    /* devmem entry */
    struct fw_rsc_devmem devmem2;

    /* devmem entry */
    struct fw_rsc_devmem devmem3;

    /* devmem entry */
    struct fw_rsc_devmem devmem4;

    /* devmem entry */
    struct fw_rsc_devmem devmem5;

    /* devmem entry */
    struct fw_rsc_devmem devmem6;

    /* devmem entry */
    struct fw_rsc_devmem devmem7;

    /* devmem entry */
    struct fw_rsc_devmem devmem8;

    /* devmem entry */
    struct fw_rsc_devmem devmem9;

    /* devmem entry */
    struct fw_rsc_devmem devmem10;
};

extern char ti_trace_SysMin_Module_State_0_outbuf__A;
#define TRACEBUFADDR (UInt32)&ti_trace_SysMin_Module_State_0_outbuf__A

#pragma DATA_SECTION(ti_ipc_remoteproc_ResourceTable, ".resource_table")
#pragma DATA_ALIGN(ti_ipc_remoteproc_ResourceTable, 4096)

struct my_resource_table ti_ipc_remoteproc_ResourceTable = {
    1,      /* we're the first version that implements this */
    RSC_NUM_ENTRIES,     /* number of entries in the table */
    0, 0,   /* reserved, must be zero */
    /* offsets to entries */
    {
        offsetof(struct my_resource_table, rpmsg_vdev),
        offsetof(struct my_resource_table, text_cout),
        offsetof(struct my_resource_table, ipcdata_cout),
        offsetof(struct my_resource_table, ipucode_cout),
        offsetof(struct my_resource_table, ipudata_cout),
        offsetof(struct my_resource_table, trace),
        offsetof(struct my_resource_table, devmem0),
        offsetof(struct my_resource_table, devmem1),
        offsetof(struct my_resource_table, devmem2),
        offsetof(struct my_resource_table, devmem3),
        offsetof(struct my_resource_table, devmem4),
        offsetof(struct my_resource_table, devmem5),
        offsetof(struct my_resource_table, devmem6),
        offsetof(struct my_resource_table, devmem7),
        offsetof(struct my_resource_table, devmem8),
        offsetof(struct my_resource_table, devmem9),
        offsetof(struct my_resource_table, devmem10),
    },

    /* rpmsg vdev entry */
    {
        TYPE_VDEV, VIRTIO_ID_RPMSG, 0,
        RPMSG_IPU_C0_FEATURES, 0, 0, 0, 2, { 0, 0 },
        /* no config data */
    },
    /* the two vrings */
    { IPU_MEM_RPMSG_VRING0, 4096, IPU_RPMSG_VQ0_SIZE, 1, 0 },
    { IPU_MEM_RPMSG_VRING1, 4096, IPU_RPMSG_VQ1_SIZE, 2, 0 },

    {
        TYPE_CARVEOUT,
        IPU_MEM_TEXT, 0,
        IPU_MEM_TEXT_SIZE, 0, 0, "IPU_MEM_TEXT",
    },

    {
        TYPE_CARVEOUT,
        IPU_MEM_IPC_DATA, 0,
        IPU_MEM_IPC_DATA_SIZE, 0, 0, "IPU_MEM_IPC_DATA",
    },

    {
        TYPE_CARVEOUT,
        IPU_MEM_CODE, 0,
        IPU_MEM_CODE_SIZE, 0, 0, "IPU_MEM_CODE",
    },

    {
        TYPE_CARVEOUT,
        IPU_MEM_DATA, 0,
        IPU_MEM_DATA_SIZE, 0, 0, "IPU_MEM_DATA",
    },

    {
        TYPE_TRACE, TRACEBUFADDR, 0x8000, 0, "trace:sysm3",
    },

    {
        TYPE_DEVMEM,
        IPU_MEM_IPC_VRING, IPU_PHYS_MEM_IPC_VRING,
        IPU_MEM_IPC_VRING_SIZE, 0, 0, "IPU_MEM_IPC_VRING",
    },

    {
        TYPE_DEVMEM,
        IPU_TILER_MODE_0_1, L3_TILER_MODE_0_1,
        SZ_256M, 0, 0, "IPU_TILER_MODE_0_1",
    },

    {
        TYPE_DEVMEM,
        IPU_TILER_MODE_2, L3_TILER_MODE_2,
        SZ_128M, 0, 0, "IPU_TILER_MODE_2",
    },

    {
        TYPE_DEVMEM,
        IPU_TILER_MODE_3, L3_TILER_MODE_3,
        SZ_128M, 0, 0, "IPU_TILER_MODE_3",
    },

    {
        TYPE_DEVMEM,
        IPU_REG_SPACE_MAP1_VIRT, IPU_REG_SPACE_MAP1_PHYS,
        IPU_REG_SPACE_MAP1_SIZE, 0, 0, "IPU_REG_SPACE_MAP1",
    },

    {
        TYPE_DEVMEM,
        IPU_REG_SPACE_MAP2_VIRT, IPU_REG_SPACE_MAP2_PHYS,
        IPU_REG_SPACE_MAP2_SIZE, 0, 0, "IPU_REG_SPACE_MAP2",
    },

    {
        TYPE_DEVMEM,
        IPU_PERIPHERAL_L4EMU, L4_PERIPHERAL_L4EMU,
        SZ_16M, 0, 0, "IPU_PERIPHERAL_L4EMU",
    },

    {
        TYPE_DEVMEM,
        IPU_IVAHD_CONFIG, L3_IVAHD_CONFIG,
        SZ_16M, 0, 0, "IPU_IVAHD_CONFIG",
    },

    {
        TYPE_DEVMEM,
        IPU_IVAHD_SL2, L3_IVAHD_SL2,
        SZ_16M, 0, 0, "IPU_IVAHD_SL2",
    },

    {
        TYPE_DEVMEM,
        IPU_SR0_VIRT, IPU_SR0,
        IPU_SR0_SIZE, 0, 0, "IPU_SR0",
    },

    {
        TYPE_DEVMEM,
        EVE_MEM_VIRT, EVE_MEM,
        EVE_MEM_SIZE, 0, 0, "EVE_MEM",
    },
};

#endif /* _RSC_TABLE_IPU_H_ */
