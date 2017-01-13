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
#include <c6x.h>

EXPORT uint32_t __clock                (void);
EXPORT uint64_t __clock64              (void);
EXPORT void     __cycle_delay          (uint64_t cyclesToDelay);
EXPORT void     __mfence               (void);
void     cacheWbInvAllL2        (void);
void     cacheInvAllL2          (void);
void     cacheWbInvL2           (uint8_t* bufferPtr, uint32_t bufferSize);
void     cacheInvL2             (uint8_t* bufferPtr, uint32_t bufferSize);
uint32_t count_trailing_zeros   (uint32_t x);

void     enableCache    (unsigned start, unsigned next_start);
void     disableCache   (unsigned start, unsigned next_start);


#if defined (DEVICE_K2H) || defined (DEVICE_K2L) || defined (DEVICE_K2E)
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
#endif

#endif /* _util_h_ */
