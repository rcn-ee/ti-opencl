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

/*!
 *****************************************************************************
 * @file      ti_mem_manager.h
 * @brief     Implementation of the Memory Manager for internal memories in
              TI devices which needs to be handled by EVE software.
              User can create a object of memory with given base address and
              total size. Later user can get different memory chunks by
              requesting to memory manager
 * @version   0.0 - Sep 2008 : initial version
 *****************************************************************************
 */

#ifndef _TI_MEMMANAGER_H_
#define _TI_MEMMANAGER_H_

/* #define _FILLMEM_ */

#ifdef _FILLMEM_
#define MEMSET(a,b,c) memset(a,b,c)
#else
#define MEMSET(a,b,c)
#endif /* _FILLMEM_ */

/*!
  @struct sMemory_t
  @brief  This structure is memory object structure,
  @param  ptrBase : Base of the memory Object
  @param  ptrCurr : start point of the free memory in pool
          it is a private member of memory object
  @param  u32Totalsize : Total size of the memory object
          it is a private member of memory object
  @param  u32AvailableSize : Total free size of the memory
          it is a private member of memory object

*/
typedef struct _memory
{
  unsigned char *ptrBase;
  unsigned char *ptrCurr;
  unsigned int  u32Totalsize;
  unsigned int  u32AvailableSize;

} sMemory_t ;

typedef sMemory_t  TIMemObject;   /* !< Memory object */
typedef sMemory_t* TIMemHandle;   /* !< Handle of the memory object */

/*!
  @func   TI_CreateMemoryHandle
  @brief  This function makes a memory handle live
  @param  memhandle [IN]:  Memory Handle
  @param  baseAddr [IN]: Base address of the memory object
  @param  size [IN]:  Size of the total memory being assigned to this
                      memory handle
  @return none
*/
void TI_CreateMemoryHandle(
   TIMemHandle memhandle,
   unsigned char *baseAddr,
   unsigned int size) ;

/*!
  @func   TI_ResetMemoryHandle
  @brief  This function makes reset an existing memory object to its initial state, effectively removing all the previously allcoated memory chunks
  @param  memhandle [IN]:  Memory Handle
  @return none
*/
void TI_ResetMemoryHandle(TIMemHandle memhandle);

/*!
  @func   TI_GetMemoryChunk
  @brief  This function provides the requested memory to user
  @param  memhandle [IN]:  Memory Handle
  @param  size [IN]: Requested size of the memory
  @param  alignment[IN]:  alignment of required memory
  @return pointer to the memory
*/
unsigned char* TI_GetMemoryChunk(
    TIMemHandle memhandle,
    unsigned int size,
    unsigned int alignment) ;

#endif /* _TI_MEMMANAGER_H */
