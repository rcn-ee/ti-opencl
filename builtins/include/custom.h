/******************************************************************************
 * Copyright (c) 2017-2018, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Texas Instruments Incorporated nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef _CUSTOM_H_
#define _CUSTOM_H_

#ifndef __OPENCL_VERSION__
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OCL_TIDL_SUCCESS           (0)
#define OCL_TIDL_ERROR             (1)
#define OCL_TIDL_ALLOC_FAIL        (2)
#define OCL_TIDL_MEMREC_ALLOC_FAIL (3)
#define OCL_TIDL_PROCESS_FAIL      (4)

#define OCL_TIDL_TRACE_OFF  (0)
#define OCL_TIDL_TRACE_MIN  (1)
#define OCL_TIDL_TRACE_MED  (2)
#define OCL_TIDL_TRACE_ALL  (3)

/* NOTE: uint64_t is 64bit aligned on ARM, 32bit aligned on EVE.
 * The cycles field must be placed at a 64bit aligned offset in the
 * structs below to avoid inconsistences when they are read/written
 * by ARM and EVE
 */

typedef struct
{
    uint32_t tidlHeapSize;
    uint32_t l2HeapSize;
    uint32_t l1HeapSize;
    uint32_t errorCode;
    uint64_t cycles;
    uint32_t enableTrace;
} OCL_TIDL_InitializeParams;

typedef struct
{
    uint32_t frameIdx;
    uint32_t bytesWritten;
    uint64_t cycles;
    uint32_t errorCode;
    uint32_t enableTrace;
} OCL_TIDL_ProcessParams;

typedef struct
{
    uint32_t noZeroCoeffsPercentage;
    uint32_t networkParamHeapSize;
    uint64_t cycles;
    uint32_t errorCode;
    uint32_t enableTrace;
} OCL_TIDL_SetupParams;

#ifndef __OPENCL_VERSION__
void ocl_tidl_setup(uint8_t*              createParamsV,
                    const uint8_t*        networkParamBuffer,
                    uint8_t*              networkParamHeap,
                    OCL_TIDL_SetupParams* setupParams);

void ocl_tidl_initialize(const uint8_t*                  createParamsV,
                         const uint8_t*                  networkParamBuffer,
                         uint8_t*                        externalMemoryHeapBase,
                         OCL_TIDL_InitializeParams*      initializeParams,
                         uint8_t*                        l2HeapBase);

void ocl_tidl_process(OCL_TIDL_ProcessParams* processParams,
                      const uint8_t*          inputFrame,
                      uint8_t*                outputData);

void ocl_tidl_cleanup();
#endif



#ifdef __cplusplus
}
#endif

#endif // _CUSTOM_H_
