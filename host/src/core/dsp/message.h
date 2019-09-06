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
    EXIT, TASK, NDRKERNEL, CACHEINV,
    FREQUENCY, PRINT, CONFIGURE_MONITOR,
    SETUP_DEBUG
} command_codes;

#define MAX_NUM_CORES        (8)
#define MAX_NDR_DIMENSIONS   3
#define MAX_IN_REG_ARGUMENTS 10
#define MAX_ARGS_IN_REG_SIZE (MAX_IN_REG_ARGUMENTS*2)
#define EVE_MAX_ARGS_IN_REG_SIZE 3          // number of registers
#define EVE_MAX_ARGS_ON_STACK_SIZE 128      // bytes
#define EVE_MSG_COMMAND_MASK (0x00008000)   // used by IPU and EVE internally
#define DSP_MAX_NUM_BUILTIN_KERNELS 256

#define MAX_ARGS_TOTAL_SIZE 1024

#define MAX_XMCSES_MPAXS	7
#define FIRST_FREE_XMC_MPAX	4  // XMC MPAXs available: 4 - F
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
    uint32_t L2_scratch_start;
    uint32_t L2_scratch_size;
} kernel_config_t;

typedef struct 
{
    uint32_t need_cache_op;
    uint32_t num_mpaxs;  // TODO: XMC only mpax for kernel alloca memory
    uint32_t mpax_settings[2*MAX_XMCSES_MPAXS];  // (MPAXL, MPAXH) pair
} flush_msg_t;

/*-----------------------------------------------------------------------------
* DSP kernel profiling parameters
* event_type:  0: disabled, 1: stall event, 2: memory event
* event_number: offset to the first event, negative to disable
* stall_cycle_threshold: # stall cycles required to trigger one stall count
*----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t        stall_cycle_threshold;
    int8_t          event_type;
    int8_t          event_number1;
    int8_t          event_number2;
} profiling_t;

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
    uint32_t        timeout_ms;
    profiling_t     profiling;
    uint8_t         from_sub_device;
    uint8_t         num_cores;
    uint8_t         master_core;
} kernel_msg_t;

typedef struct
{
    uint8_t         n_cores;
    uint8_t         master_core;
    uint32_t        ocl_global_mem_addr;
    uint32_t        ocl_global_mem_size;
} configure_monitor_t;

typedef struct
{
    int retcode;

    int8_t          profiling_status;
    uint32_t        profiling_counter0_val;
    uint32_t        profiling_counter1_val;
} command_retcode_t;

/*-----------------------------------------------------------------------------
* Kernel message to Eve
*----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t        Kernel_id;            // host-assigned id for this call
    uint32_t        builtin_kernel_index; // index into function table
    uint32_t        args_on_stack_size;   // number of bytes on stack
    uint32_t        args_in_reg[EVE_MAX_ARGS_IN_REG_SIZE];
    uint8_t         args_on_stack[EVE_MAX_ARGS_ON_STACK_SIZE];
} kernel_eve_t;

typedef struct 
{
    uint16_t      command;   // enum command_codes, use uint16_t in message
    uint16_t      core_id;   // e.g. to which eve, logical index
    uint32_t      trans_id;
    uint32_t      pid;
    union
    {
        struct
        {
            kernel_config_t config;
            kernel_msg_t    kernel;
            flush_msg_t     flush;
        } k;
        kernel_eve_t        k_eve;
        configure_monitor_t configure_monitor;
        command_retcode_t   command_retcode;
        char message[sizeof(kernel_config_t) + sizeof(kernel_msg_t) + 
                     sizeof(flush_msg_t)];
    } u;
} Msg_t;

static Msg_t exitMsg      = {EXIT};
static Msg_t frequencyMsg = {FREQUENCY};
static Msg_t cacheMsg     = {CACHEINV};
static Msg_t debugMsg     = {SETUP_DEBUG};

// static far Msg_t printMsg  = {PRINT}; // moved to L2 in monitor

static const uint32_t mbox_payload         = sizeof(Msg_t);

#define MBOX_SIZE 0x2000

// InOrder vs OutOfOrder encoding: global_sz_0
#define IN_ORDER_TASK_SIZE	1
#define OUT_OF_ORDER_TASK_SIZE	(IN_ORDER_TASK_SIZE+1)
// Normal vs Debug encoding: WG_gid_start_0
#define NORMAL_MODE_WG_GID_START 0
#define DEBUG_MODE_WG_GID_START  1

// Need to be consistent with error code in cl_ext.h
#define CL_SUCCESS                                  0
#define CL_ERROR_KERNEL_ABORT_TI                    -7101
#define CL_ERROR_KERNEL_EXIT_TI                     -7102
#define CL_ERROR_KERNEL_TIMEOUT_TI                  -7103


#define IS_OOO_TASK(msg) ((msg.command == TASK) && \
                          (msg.u.k.config.global_size[0] != IN_ORDER_TASK_SIZE))

#define IS_DEBUG_MODE(msg) (msg.u.k.config.WG_gid_start[0] == \
                            DEBUG_MODE_WG_GID_START)
#endif
