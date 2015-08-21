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
#ifndef DATA_H_
#define DATA_H_

// core processing sizes
#define CORE_PROCESS_ROWS 4
#define CORE_PROCESS_COLS 8

// partition in k dimension, 512 can fully use 16K L1D SRAM
#define KPARTITION 512

// have to reduce A,B PANLES so that they fit in 128K L2 SRAM on AM57
//                                            or 512K L2 SRAM on Hawking
// #define NUMAPANELS 8
// #define NUMBPANELS 2

// partition in m dimension
// #define MPARTITION (NUMAPANELS*CORE_PROCESS_ROWS)
// partition in n dimension
// #define NPARTITION (NUMBPANELS*CORE_PROCESS_COLS)

void dataMoveB(float * restrict dst, float * restrict src, int k);
void dataMoveA(float * restrict dst, float * restrict src, int m, int k,
               int MPARTITION);
void dataMoveA_DDR2L2(float * restrict dst, float * restrict src, int m, int k,
                      int lda);
#endif
