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
/**
 *   @file  osal.c
 *
 *   @brief   
 *      This is the OS abstraction layer for the CPPI and QMSS.
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stddef.h>

/* CSL Semaphore module includes */
#include <ti/csl/csl_semAux.h>

/* CSL Cache Include Files. */
#include <ti/csl/csl_cache.h>
#include <ti/csl/csl_cacheAux.h>

/**********************************************************************
 ****************************** Defines *******************************
 **********************************************************************/
#define QMSS_HW_SEM     2

#define OSAL_POOL_SIZE      8192

/**********************************************************************
 ************************** Global Variables **************************
 **********************************************************************/
uint32_t  qmssMallocCounter  = 0;
uint32_t  qmssFreeCounter    = 0;
uint32_t  cppiMallocCounter  = 0;
uint32_t  cppiFreeCounter    = 0;
uint32_t  index              = 0;

uint8_t   globalMemoryPool[OSAL_POOL_SIZE];

/**********************************************************************
 *************************** OSAL Functions **************************
 **********************************************************************/

/**
 *  @b Description
 *  @n  
 *      The function is used to allocate a memory block of the specified size.
 *
 *  @param[in]  num_bytes
 *      Number of bytes to be allocated.
 *
 *  @retval
 *      Allocated block address
 */
void *
Osal_qmssMalloc (
    uint32_t num_bytes
)
{
//	Error_Block	errorBlock;

    /*
     * Increment the allocation counter. 
     */
    qmssMallocCounter++;

    return NULL;

    /*
     * Allocate memory. 
     */
//	return Memory_alloc(NULL, num_bytes, 0, &errorBlock);
}

/**
 *  @b Description
 *  @n  
 *      The function is used to free a memory block of the specified size.
 *
 *  @param[in]  ptr
 *      Pointer to the memory block to be cleaned up.
 *
 *  @param[in]  size
 *      Size of the memory block to be cleaned up.
 *
 *  @retval
 *      Not Applicable
 */
void
Osal_qmssFree (
    void *ptr,
    uint32_t size
)
{
    /*
     * Increment the free counter. 
     */
    qmssFreeCounter++;	
//	Memory_free(NULL, ptr, size);
}

/**
 *  @b Description
 *  @n  
 *      The function is used to enter a critical section.
 *      Function protects against 
 *      
 *      access from multiple cores 
 *      and 
 *      access from multiple threads on single core
 *
 *  @retval
 *      Handle used to lock critical section
 */
void *
Osal_qmssCsEnter (
    void
)
{
    /*
     * Get the hardware semaphore 
     */
    while ((CSL_semAcquireDirect (QMSS_HW_SEM)) == 0);

    /*
     * Create Semaphore for protection against access from multiple threads 
     * * Not created here becasue application is not multithreaded 
     * * 
     */
    return NULL;
}

/**
 *  @b Description
 *  @n  
 *      The function is used to exit a critical section 
 *      protected using Osal_qmssCsEnter() API.
 *
 *  @param[in]  CsHandle
 *      Handle for unlocking critical section.
 *
 *  @retval
 *      Not Applicable
 */
void
Osal_qmssCsExit (
    void *CsHandle
)
{
    /*
     * Release Semaphore using handle 
     */
    /* Release Semaphore using handle */
    /*
     * Release the hardware semaphore 
     */
    CSL_semReleaseSemaphore (QMSS_HW_SEM);

    return;
}

/**
 *  @b Description
 *  @n  
 *      The function is used to enter a critical section.
 *      Function protects against 
 *      access from multiple threads on single core
 *
 *  @retval
 *      Handle used to lock critical section
 */
void *
Osal_qmssMtCsEnter (
    void
)
{
    /*
     * Create Semaphore for protection against access from multiple threads 
     * * Not created here becasue application is not multithreaded 
     * * 
     */
    return NULL;
}

/**
 *  @b Description
 *  @n  
 *      The function is used to exit a critical section
 *      protected using Osal_qmssMtCsEnter() API.
 *
 *  @param[in]  CsHandle
 *      Handle for unlocking critical section.
 *
 *  @retval
 *      Not Applicable
 */
void
Osal_qmssMtCsExit (
    void *CsHandle
)
{
    /*
     * Release Semaphore using handle 
     */
        
    return;
}

/**
 *  @b Description
 *  @n  
 *      The function is the QMSS OSAL Logging API which logs 
 *      the messages on the console.
 *
 *  @param[in]  fmt
 *      Formatted String.
 *
 *  @retval
 *      Not Applicable
 */
void
Osal_qmssLog (
    char *fmt,
    ...
)
{
}

/**
 *  @b Description
 *  @n  
 *      The function is used to indicate that a block of memory is 
 *      about to be accessed. If the memory block is cached then this 
 *      indicates that the application would need to ensure that the 
 *      cache is updated with the data from the actual memory.
 * 
 *  @param[in]  ptr
 *       Address of memory block
 *
 *  @param[in]  size
 *       Size of memory block
 *
 *  @retval
 *      Not Applicable
 */
void
Osal_qmssBeginMemAccess (
    void *ptr,
    uint32_t size
)
{
    CACHE_invL1d (ptr, size, CACHE_WAIT);
    return;
}

/**
 *  @b Description
 *  @n  
 *      The function is used to indicate that the block of memory has 
 *      finished being accessed. If the memory block is cached then the 
 *      application would need to ensure that the contents of the cache 
 *      are updated immediately to the actual memory. 
 *
 *  @param[in]  ptr
 *       Address of memory block
 *
 *  @param[in]  size
 *       Size of memory block

 *  @retval
 *      Not Applicable
 */
void
Osal_qmssEndMemAccess (
    void *ptr,
    uint32_t size
)
{
    CACHE_wbL1d (ptr, size, CACHE_WAIT);
    return;
}

