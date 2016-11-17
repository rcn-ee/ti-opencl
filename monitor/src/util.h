/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPL
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABL
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICE
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AN
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF T
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef _util_h_
#define _util_h_

#include <monitor.h>
#include <stdint.h>
#include <ti/csl/csl_semAux.h>

extern cregister volatile uint32_t DNUM;

void     startEmifPerfCounters  (void);
void     stopEmifPerfCounters   (uint32_t* count1Ptr, uint32_t* count2Ptr);
int      initClockGlobal        (void);
int      initClockLocal         (void);
uint32_t readClockGlobal        (void);
EXPORT uint32_t __clock                (void);
EXPORT uint64_t __clock64              (void);
EXPORT void     __cycle_delay          (uint64_t cyclesToDelay);
EXPORT void     __mfence               (void);
void     waitAtCoreBarrier      (void);
void     cacheWbInvAllL2        (void);
void     cacheInvAllL2          (void);
void     cacheWbInvL2           (uint8_t* bufferPtr, uint32_t bufferSize);
void     cacheInvL2             (uint8_t* bufferPtr, uint32_t bufferSize);
uint32_t get_dsp_id             (void);
uint32_t count_trailing_zeros   (uint32_t x);

void     enableCache    (unsigned start, unsigned next_start);
void     disableCache   (unsigned start, unsigned next_start);
void     set_MPAX       (            int index, uint32_t bAddr, uint8_t segSize,
                                                uint32_t rAddr, uint8_t perm);
void     set_MSMC_MPAX  (int privId, int index, uint32_t bAddr, uint8_t segSize,
                                                uint32_t rAddr, uint8_t perm);
void     reset_MPAX     (            int index);
void     reset_MSMC_MPAX(int privId, int index);
void     set_kernel_MPAXs  (int num_mpaxs, uint32_t *settings);
void     reset_kernel_MPAXs(int num_mpaxs);
void     clear_mpf           (void);
void     report_and_clear_mpf(void);

uint32_t makeAddressGlobal(uint32_t coreIdx, uint32_t address);

static inline void ocl_mfence(void)
{
    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
}


/******************************************************************************
* Locking Mechanism
******************************************************************************/
static inline void sem_lock(void)
{
    while (!CSL_semAcquireDirect(OCL_HW_SEM_IDX));
}

static inline void sem_unlock(void)
{
    ocl_mfence();
    CSL_semReleaseSemaphore(OCL_HW_SEM_IDX);
}

static inline void sem_reset(void)
{
    ocl_mfence();
    if (!CSL_semIsFree(OCL_HW_SEM_IDX)) CSL_semReleaseSemaphore(OCL_HW_SEM_IDX);

}


#endif /* _util_h_ */
