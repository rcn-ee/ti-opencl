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
#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <stdint.h>

typedef enum
{
    READY, EXIT, TASK, NDRKERNEL, WORKGROUP, CACHEINV,
    FREQUENCY, SUCCESS, ERROR, PRINT, BROADCAST, CONFIGURE_MONITOR
} command_codes;

#define MAX_NDR_DIMENSIONS   3
#define MAX_IN_REG_ARGUMENTS 10
#define MAX_ARGS_IN_REG_SIZE (MAX_IN_REG_ARGUMENTS*2)

#define MAX_ARGS_TOTAL_SIZE 1024

#define MAX_XMCSES_MPAXS	7
#define FIRST_FREE_XMC_MPAX	3  // XMC MPAXs available: 3 - F
#define FIRST_FREE_SES_MPAX	1  // SES MPAXs available: 1 - 7
// MPAXH:  20-bit baddr, 7-bit padding, 5-bit segment size
// MPAXL:  24-bit raddr, 8-bit permission (Res, Res, SR, SW, SX, UR, UW, UX)
#define DEFAULT_PERMISSION	0x3F


/******************************************************************************
* Need to ensure that the alignments and therefore the offsets of all fields 
* are consistent between the host and the device. There is a dependency on 
* order here with both the wga xforms and the get_xxx functions in the 
* builtins library.
******************************************************************************/
typedef struct
{
    uint32_t num_dims;
    uint32_t global_size  [MAX_NDR_DIMENSIONS];
    uint32_t local_size   [MAX_NDR_DIMENSIONS];
    uint32_t global_offset[MAX_NDR_DIMENSIONS];
    uint32_t WG_gid_start [MAX_NDR_DIMENSIONS];
    uint32_t WG_id;
    uint32_t WG_alloca_start;
    uint32_t WG_alloca_size;
} kernel_config_t;

typedef struct 
{
    uint32_t need_cache_op;
    uint32_t num_mpaxs;  // TODO: XMC only mpax for kernel alloca memory
    uint32_t mpax_settings[2*MAX_XMCSES_MPAXS];  // (MPAXL, MPAXH) pair
} flush_msg_t;

/*-----------------------------------------------------------------------------
* The dsp_rpc.asm file has a dependency on the exact order of the fields:
* entry_point through args_in_reg. If they are changed, then dsp_rpc.asm will
* also need modification.
*----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t        Kernel_id;
    uint32_t        entry_point;
    uint32_t        data_page_ptr;
    uint32_t        args_in_reg_size;
    uint32_t        args_in_reg[MAX_ARGS_IN_REG_SIZE];
    uint32_t        args_on_stack_addr;
    uint32_t        args_on_stack_size;
} kernel_msg_t;

typedef struct
{
    int n_cores;

    int ocl_qmss_hw_queue_base_idx;
    int ocl_qmss_first_desc_idx_in_linking_ram;
    int ocl_qmss_first_memory_region_idx;
} configure_monitor_t;

typedef struct 
{
    command_codes command;
    uint32_t      trans_id;
    union
    {
        struct
        {
            kernel_config_t config;
            kernel_msg_t    kernel;
            flush_msg_t     flush;
        } k;
        configure_monitor_t configure_monitor;
        char message[sizeof(kernel_config_t) + sizeof(kernel_msg_t) + sizeof(flush_msg_t)];
    } u;
} Msg_t;

static Msg_t exitMsg      = {EXIT};
static Msg_t successMsg   = {SUCCESS};
static Msg_t readyMsg     = {READY};
static Msg_t errorMsg     = {ERROR};
static Msg_t frequencyMsg = {FREQUENCY};
static Msg_t cacheMsg     = {CACHEINV};

// static far Msg_t printMsg  = {PRINT}; // moved to L2 in monitor

static const uint32_t mbox_payload         = sizeof(Msg_t);

#define MBOX_SIZE 0x2000

// InOrder vs OutOfOrder encoding: global_sz_0
#define IN_ORDER_TASK_SIZE	1
#define OUT_OF_ORDER_TASK_SIZE	(IN_ORDER_TASK_SIZE+1)
// Normal vs Debug encoding: WG_gid_start_0
#define NORMAL_MODE_WG_GID_START 0
#define DEBUG_MODE_WG_GID_START  1

#define IS_OOO_TASK(msg) ((msg.command == TASK) && \
                          (msg.u.k.config.global_size[0] != IN_ORDER_TASK_SIZE))

#endif
