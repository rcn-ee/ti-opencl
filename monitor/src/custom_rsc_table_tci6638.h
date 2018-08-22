/*
 * Copyright (c) 2012-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== custom_rsc_table_tci6638.h ========
 *
 *  Include this table in each base image, which is read from remoteproc on
 *  host side.
 *  Original in IPC package: packages/ti/ipc/remoteproc/rsc_table_tci6638.h
 *
 */

#ifndef _RSC_TABLE_TCI6638_H_
#define _RSC_TABLE_TCI6638_H_

#include <ti/ipc/remoteproc/rsc_types.h>

/* virtio ids: keep in sync with the linux "include/linux/virtio_ids.h" */
#define VIRTIO_ID_RPMSG         7 /* virtio remote processor messaging */

/* flip up bits whose indices represent features we support */
#define RPMSG_IPU_C0_FEATURES         1

#define RPMSG_VRING0_DA     0xA0000000
#define RPMSG_VRING1_DA     0xA0004000

/*
 * sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 */
#define RPMSG_VQ0_SIZE          256
#define RPMSG_VQ1_SIZE          256

struct my_resource_table {
    struct resource_table base;

#ifndef TRACE_RESOURCE_ONLY
    UInt32 offset[2];
#else
    UInt32 offset[1];
#endif

#ifndef TRACE_RESOURCE_ONLY
    /* rpmsg vdev entry */
    struct fw_rsc_vdev rpmsg_vdev;
    struct fw_rsc_vdev_vring rpmsg_vring0;
    struct fw_rsc_vdev_vring rpmsg_vring1;
#endif

    /* trace entry */
    struct fw_rsc_trace trace;
};

/* Add trace buffer information to the resource table */
extern char ti_trace_SysMin_Module_State_0_outbuf__A;
#define TRACEBUFADDR (UInt32)&ti_trace_SysMin_Module_State_0_outbuf__A
/* need to be in sync with SysMin.bufSize in cfg file */
#define TRACEBUFSIZE 0x1800

#pragma DATA_SECTION(ti_ipc_remoteproc_ResourceTable, ".resource_table")
#pragma DATA_ALIGN(ti_ipc_remoteproc_ResourceTable, 4096)

struct my_resource_table ti_ipc_remoteproc_ResourceTable = {
    1, /* we're the first version that implements this */
#ifndef TRACE_RESOURCE_ONLY
    2, /* number of entries in the table */
#else
    1,
#endif
    0, 0, /* reserved, must be zero */
    /* offsets to entries */
    {
#ifndef TRACE_RESOURCE_ONLY
        offsetof(struct my_resource_table, rpmsg_vdev),
#endif
        offsetof(struct my_resource_table, trace),
    },

#ifndef TRACE_RESOURCE_ONLY
    /* rpmsg vdev entry */
    {
        TYPE_VDEV, VIRTIO_ID_RPMSG, 0,
        RPMSG_IPU_C0_FEATURES, 0, 0, 0, 2, { 0, 0 },
        /* no config data */
    },
    /* the two vrings */
    { RPMSG_VRING0_DA, 4096, RPMSG_VQ0_SIZE, 1, 0 },
    { RPMSG_VRING1_DA, 4096, RPMSG_VQ1_SIZE, 2, 0 },
#endif

    {
        TYPE_TRACE, TRACEBUFADDR, TRACEBUFSIZE, 0, "trace:dsp",
    },
};

#endif /* _RSC_TABLE_TCI6638_H_ */
