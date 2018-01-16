/******************************************************************************
 * Copyright (c) 2018, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Texas Instruments Incorporated nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef __IVISION_UTIL_H_
#define __IVISION_UTIL_H_

#include <stdint.h>
#include "xdais_types.h"
#include "ivision.h"
#include "ti/xdais/ires.h"
#include "ti_mem_manager.h"

#define MAX_ALG_IN_BUFS  (4)
#define MAX_ALG_OUT_BUFS (4)

typedef struct
{
    IALG_MemRec*     memRec;
    int32_t          numMemRec;

    IVISION_Handle   handle;
    IVISION_BufDesc  inBufDesc[MAX_ALG_IN_BUFS];
    IVISION_BufDesc  outBufDesc[MAX_ALG_OUT_BUFS];
    IVISION_BufDesc* inBufDescList[MAX_ALG_IN_BUFS];
    IVISION_BufDesc* outBufDescList[MAX_ALG_OUT_BUFS];

    IVISION_InBufs   inBufs;
    IVISION_OutBufs  outBufs;

    TIMemObject      memObj_DMEM0;
    TIMemObject      memObj_DMEM1;
    TIMemObject      memObj_EXTMEM;

} OCL_IVISION_State;

int32_t AllocMemRecords(IALG_MemRec* memRec, int32_t numMemRec,
                        OCL_IVISION_State* state);
int32_t FreeMemRecords(IALG_MemRec* memRec, int32_t numMemRec,
                       OCL_IVISION_State* state);


/* Algorithm state maintained across OpenCL init/process/cleanup kernels */
extern OCL_IVISION_State alg_state;

#endif
