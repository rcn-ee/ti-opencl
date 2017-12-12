/*
*
* Copyright (c) 2009-2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/*!
 *****************************************************************************
 * @file      ti_mem_manager.c
 * @brief     Implementation of the Memory Manager for internal memories in TI
              devices which needs to be handled by DSP software
  * @version   0.0 - Sep 2008 : initial version
*****************************************************************************
 */

#include <ti_mem_manager.h>

#pragma CHECK_MISRA ("none")
#include <stdio.h>
#include <assert.h>
#pragma RESET_MISRA ("required")

#define _GUARD_SIZE_      0x00
#define _PAYLOAD_VAL_     0xBB
#define _GUARD_VAL_       0xAA

/*!
  @func   TI_CreateMemoryHandle
  @brief  This function makes a memory handle live
  @param  memhandle [IN]:  Memory Handle
  @param  baseAddr [IN]: Base address of the memory object
  @param  size [IN]:  Size of the total memory being assigned to this
                      memory handle
  @return none
*/
void TI_CreateMemoryHandle(TIMemHandle memhandle,
  unsigned char *baseAddr, unsigned int size)
{
  memhandle->ptrBase          = baseAddr;
  memhandle->u32Totalsize     = size;
  memhandle->u32AvailableSize = size;
  memhandle->ptrCurr          = baseAddr;
  return  ;
}

/*!
  @func   TI_ResetMemoryHandle
  @brief  This function makes reset an existing memory object to its initial state, effectively removing all the previously allcoated memory chunks
  @param  memhandle [IN]:  Memory Handle
  @return none
*/
void TI_ResetMemoryHandle(TIMemHandle memhandle)
{
  memhandle->u32AvailableSize = memhandle->u32Totalsize;
  memhandle->ptrCurr          = memhandle->ptrBase;
  return  ;
}

/*!
  @func   TI_GetMemoryChunk
  @brief  This function provides the requested memory to user
  @param  memhandle [IN]:  Memory Handle
  @param  size [IN]: Requested size of the memory
  @param  alignment[IN]:  alignment of required memory
  @return pointer to the memory
*/
unsigned char* TI_GetMemoryChunk(TIMemHandle memhandle,
  unsigned int size, unsigned int alignment)
{
  unsigned char *mem ;
  unsigned int alignmentBytes  =
    ((((unsigned int)memhandle->ptrCurr + _GUARD_SIZE_) + alignment - 1) & (~(alignment - 1))) -
      ((unsigned int)memhandle->ptrCurr + _GUARD_SIZE_);

  assert(alignment > 0) ;
  if( memhandle->u32AvailableSize < (alignmentBytes + size))
  {
    /* ----------------------------------------------------*/
    /* This scenario arrives if remaining space in SL2     */
    /* is not sufficient for the requested chunk of memory */
    /* Control never hits below  instruction at run-time   */
    /* This while(1) is kept here to catch the SL2 memory  */
    /* insufficiency during development phase              */
    /* ----------------------------------------------------*/
    /* Removing while loop. If a memory is not avilable then
     * test app can take action to allocate this memory from
     * DDR instead of just getting stuck in while(1)
    */
    /*while(1){
        ;
    }*/
    mem = 0x0;
  }
  else
  {
    memhandle->u32AvailableSize -= (alignmentBytes   + (size + 2*_GUARD_SIZE_));
    mem                 = (memhandle->ptrCurr + (alignmentBytes + _GUARD_SIZE_));
    memhandle->ptrCurr += (alignmentBytes     + (size + 2*_GUARD_SIZE_)       ) ;

    MEMSET(mem, _PAYLOAD_VAL_, size);
    MEMSET(mem - _GUARD_SIZE_ , _GUARD_VAL_  , _GUARD_SIZE_) ;
    MEMSET(mem + size         , _GUARD_VAL_  , _GUARD_SIZE_) ;
  }
  assert (mem != 0);
  return mem ;
}
