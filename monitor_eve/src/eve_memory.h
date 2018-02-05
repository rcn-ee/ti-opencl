/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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

#ifndef _EVE_MEMORY_H_
#define _EVE_MEMORY_H_

/* global DMEM scratch memory for creating memory handle in user algorithms */
/* multiple libraries can share the use of this fast memory, however, each  */
/* library will use it exclusively */
#define DMEM0_SIZE (20*1024)
/* to work around an array out-of-bounds problem in TIDL */
#define DMEM0_PAD_SIZE (256)
/* Need at least 7092B for evelib_imagePyramid_u8.c */
#define TASK_STACK_SIZE 0x2C00
#define DMEM1_SIZE (144*1024)

//#pragma DATA_SECTION (DMEM0_SCRATCH, ".dmem0Sect");  // mapped to DMEM
extern uint8_t DMEM0_SCRATCH[DMEM0_SIZE+DMEM0_PAD_SIZE];

//#pragma DATA_SECTION (DMEM1_SCRATCH, ".dmem1Sect");  // mapped to EXTDMEM
extern uint8_t DMEM1_SCRATCH[DMEM1_SIZE];

#endif  // _EVE_MEMORY_H_
