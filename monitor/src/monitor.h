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

#ifndef  _monitor_h_
#define  _monitor_h_

#include <stdint.h>
#include <ti/csl/csl_cacheAux.h>
#include "message.h"

extern int n_cores;

extern cregister volatile unsigned int DNUM;

#define OCL_HW_SEM_IDX                          (3)


#define RETURN_FAIL 0
#define RETURN_OK   1

#define MAX_NUM_CORES (8)
#define ROUNDUP(val, pow2)   (((val) + (pow2) - 1) & ~((pow2) - 1))

#define EXPORT __attribute__((visibility("protected")))

/******************************************************************************
* flush_msg_id:
*   >= 0         : epilog for regular NDRKernel with unique transaction id
*   CACHEINV     : cahe invalidation after loading each new program
*   KERNEL_PROLOG: prolog for NDRKernel and in-order task (OpenMP)
*   IOTASK_EPILOG: epilog for in-order task (OpenMP)
******************************************************************************/
#define FLUSH_MSG_CACHEINV      0xFFFFFFFE  // cache invalidation
#define FLUSH_MSG_KERNEL_PROLOG 0xFFFFFFFD  // kernel/iotask prolog
#define FLUSH_MSG_IOTASK_EPILOG 0xFFFFFFFC  // iotask epilog


/******************************************************************************
* Macro used to define variables that will be aligned to a cache linesize and 
* will also be in shared memory that is both fast and noncached
******************************************************************************/
#define FAST_SHARED(type, var) type var \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".fast_shared_noncached")))

#define FAST_SHARED_1D(type, var, size) type var[size] \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".fast_shared_noncached")))

#define FAST_SHARED_2D(type, var, sz1, sz2) type var[sz1][sz2] \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".fast_shared_noncached")))

#define PRIVATE(type, var) type var \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".private")))

#define PRIVATE_NOALIGN(type, var) type var \
                   __attribute((section(".private")))

#define PRIVATE_1D(type, var, size) type var[size] \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".private")))

#define PRIVATE_2D(type, var, sz1, sz2) type var[sz1][sz2] \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".private")))

#define DDR(type, var) type var \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".ddr")))

#define DDR_1D(type, var, size) type var[size] \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".ddr")))

#define DDR_2D(type, var, sz1, sz2) type var[sz1][sz2] \
                   __attribute__((aligned(CACHE_L2_LINESIZE))) \
                   __attribute((section(".ddr")))

/******************************************************************************
* Device functions
******************************************************************************/
void initialize_memory(void);
unsigned dsp_speed();

#endif  //_monitor_h_
