/*
 *
 * Copyright (C) 2012-2014 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/


#ifndef __CMEM_H__
#define __CMEM_H__

#define CMEM_DRVNAME     "cmem"
#define CMEM_MODFILE     "cmem"

#define CMEM_DRIVER_SIGNATURE "/dev/"CMEM_MODFILE

#ifdef __KERNEL__

#endif  /*  __KERNEL__  */
/* Maximum number of buffers allocated per API call*/
#define CMEM_MAX_BUF_PER_ALLOC 64
/**
* ti816x_bar_info - PCI Base Address Register information
* @num: BAR register index - 0 to 5
* @addr: For 'SET' operations, contains ti816x internal address to translate
* @size: Size allocated for this BAR (only usd for GET operation)
* this BAR access to. For 'GET'' operations, contains the (host) physical
* address assigned to this BAR.
*/
/* Basic information about host buffer accessible by DSP through PCIE */
typedef struct   _cmem_host_buf_entry_t {
    uint64_t dmaAddr;                  /* PCIe address */
    uint8_t *virtAddr;                  /* Host Virtual address */
    uint32_t length;           /* Length of host buffer */
} cmem_host_buf_entry_t;


/* List of Buffers  */
typedef struct _cmem_host_buf_info_t {
    unsigned int num_buffers;      /* Number of host buffers */
    unsigned int type;             /* memory type 0; Persistent; 1; Dynamic */
    cmem_host_buf_entry_t *buf_info;
} cmem_host_buf_info_t;

/* List of Buffers  */
typedef struct _cmem_ioctl_host_buf_info_t {
    unsigned int num_buffers;      /* Number of host buffers */
    unsigned int type;             /* memory type 0; Persistent; 1; Dynamic */
    cmem_host_buf_entry_t buf_info[CMEM_MAX_BUF_PER_ALLOC];
} cmem_ioctl_host_buf_info_t;

/** Parameters used for calling IOCTL */
typedef struct _cmem_ioctl_t {
  cmem_ioctl_host_buf_info_t host_buf_info;
} cmem_ioctl_t;

/* IOCTLs defined for the application as well as driver */
#define CMEM_IOCTL_ALLOC_HOST_BUFFERS  _IOWR('P', 1, unsigned int)
#define CMEM_IOCTL_GET_HOST_BUF_INFO   _IOWR('P', 2, unsigned int)
#define CMEM_IOCTL_FREE_HOST_BUFFERS   _IOWR('P', 3, unsigned int)

#endif
