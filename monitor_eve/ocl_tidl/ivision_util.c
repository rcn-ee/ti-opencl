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

#include <string.h>
#include "ivision_util.h"

/* Algorithm state maintained across OpenCL init/process/cleanup kernels */
OCL_IVISION_State alg_state __attribute__((aligned(128)));

int32_t AllocMemRecords(IALG_MemRec* memRec, int32_t numMemRec,
                        OCL_IVISION_State* state)
{
    if (memRec == NULL || state == NULL)
        return IALG_EFAIL;

    int32_t i;
    TIMemHandle memHdl_DMEM0  = &state->memObj_DMEM0;
    TIMemHandle memHdl_DMEM1  = &state->memObj_DMEM1;
    TIMemHandle memHdl_EXTMEM = &state->memObj_EXTMEM;

    for (i = 0; i < numMemRec; i++)
    {
      if(memRec[i].space == IALG_DARAM0) {
        memRec[i].base = TI_GetMemoryChunk(memHdl_DMEM0, memRec[i].size, 
                                           memRec[i].alignment);
      }
      else if(memRec[i].space == IALG_DARAM1) {
        memRec[i].base = TI_GetMemoryChunk(memHdl_DMEM1, memRec[i].size, 
                                           memRec[i].alignment);
      }
      else {
        memRec[i].base = (void *) TI_GetMemoryChunk(memHdl_EXTMEM,
                                                    memRec[i].size,
                                                    memRec[i].alignment);
        memset(memRec[i].base, 0, memRec[i].size);
      }

      if(memRec[i].base == NULL)
          return IALG_EFAIL;
    }

    return IALG_EOK;
}

int32_t FreeMemRecords(IALG_MemRec* memRec, int32_t numMemRec,
                       OCL_IVISION_State* state)
{
    if (memRec == NULL || state == NULL)
        return IALG_EFAIL;

    TIMemHandle memHdl_DMEM0  = &state->memObj_DMEM0;
    TIMemHandle memHdl_DMEM1  = &state->memObj_DMEM1;
    TIMemHandle memHdl_EXTMEM = &state->memObj_EXTMEM;

    int32_t i;
    for (i = 0; i < numMemRec; i++)
    {
        if(memRec[i].base == NULL)
            return IALG_EFAIL;

        if(memRec[i].space == IALG_DARAM0) 
            TI_ResetMemoryHandle(memHdl_DMEM0);
        else if(memRec[i].space == IALG_DARAM1)
            TI_ResetMemoryHandle(memHdl_DMEM1);
        else 
            TI_ResetMemoryHandle(memHdl_EXTMEM);
    }

  return IALG_EOK;
}


