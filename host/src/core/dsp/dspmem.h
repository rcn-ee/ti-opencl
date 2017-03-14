/******************************************************************************
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
#include <stdint.h>
#ifndef _DSPMEM_H
#define _DSPMEM_H

#include "../tiocl_types.h"


/*****************************************************************************
 * Keystone (II?) - DSP Device Memory Physical Addreess (8GB)
 * 0x8:0000_0000 - 0x8:1FFF_FFFF: Linux reserved
 * 0x8:2000_0000 - 0x8:21FF_FFFF: OCL runtime reserved
 *                                using default MPAX translation, map to
 *                                DSP virtual address 0xA000_0000 - 0xA1FF_FFFF
 * 0x8:2200_0000 - 0x8:3FFF_FFFF: using default MPAX translation, map to
 *                                DSP virtual address 0xA200_0000 - 0xBFFF_FFFF
 *                                used for kernel code, user app small buffers
 * 0x8:4000_0000 - 0x9:FFFF_FFFF: using custom MPAX translation settings, map
 *                                to unused DSP virtual address spaces
 *                                used for user app big buffers
 *****************************************************************************/
#define DSP_36BIT_ADDR			0x800000000ULL
#define MPAX_USER_MAPPED_DSP_ADDR	0x840000000ULL
#define ALL_PERSISTENT_MAX_DSP_ADDR	0x880000000ULL

/*****************************************************************************
 * AM57 - DSP Device Memory Physical Addreess
 * 0x0:A000_0000 - 0x0:A00F_FFFF: 16MB shared heap
 * 0x0:A010_0000 - 0x0:A01F_FFFF: 16MB DDR no cache
 * 0x0:A020_0000 - 0x0:B020_0000: 256MB - 32MB General Purpose CMEM memory
 *
 * The first 32MB of CMEM are reserved for the monitor.
 *****************************************************************************/
#define RESERVED_CMEM_SIZE      (0x02000000)

#if !defined (DEVICE_AM57)
#define MSMC_OCL_START_ADDR		0x0C000000
#define MSMC_OCL_END_ADDR		0x0C600000
#else
#define MSMC_OCL_START_ADDR		0x40300000
#define MSMC_OCL_END_ADDR		0x40600000
#endif


#define ROUNDUP(val, pow2)   (((val) + (pow2) - 1) & ~((pow2) - 1))
#define MIN_BLOCK_SIZE                 128
#define MIN_CMEM_ONDEMAND_BLOCK_SIZE  4096
#define MIN_CMEM_MAP_ALIGN            4096
#define MAX_CMEM_MAP_ALIGN            (512*1024*1024)

#endif // _DSPMEM_H
