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
#define CORE_PROCESS_COLS 4

// Maximum EDMA stride length
#define MAXDMASTRIDE 0x7FFF

// partition in k dimension
#define KPARTITION 512

// number of panels of A to keep in MSMC SRAM
// #define NUMAPANELS 32
// #define NUMAPANELS 16
// #define NUMAPANELS 4
// number of panels of B to keep in L2 SRAM
// #define NUMBPANELS 8
// #define NUMBPANELS 4

// partition in m dimension
// #define MPARTITION (NUMAPANELS*CORE_PROCESS_ROWS)
// partition in n dimension
// #define NPARTITION (NUMBPANELS*CORE_PROCESS_COLS)
/**
 *  @brief Move and re-arrange B as needed by kernel for dgemm.
 *         src point to B. dst will have B arranged in 4 x k (column major)
 *
 *  @param[in]  dst             pointer to destination address
 *  @param[in]  src             pointer to source address
 *  @param[in]  k               number of columns to move
 *
 */
void dataMoveB(double * restrict dst, double * restrict src, int k);

/**
 *  @brief Move and re-arrange B' as needed by kernel for dgemm.
 *         src point to B. dst will have B arranged in 4 x k (column major)
 *
 *  @param[in]  dst             pointer to destination address
 *  @param[in]  src             pointer to source address
 *  @param[in]  k               number of columns to move
 *  @param[in]  NPARTITION      blocks of columns to move
 *
 */
void dataMoveBT(double * restrict dst, double * restrict src, int k,
                int NPARTITION);


/**
 *  @brief Move and re-arrange A as needed by kernel for dgemm.
 *         src point to A. dst will have blocks of A, each block
 *         arranged in 4 x k (column major)
 *
 *  @param[in]  dst             pointer to destination address
 *  @param[in]  src             pointer to source address
 *  @param[in]  m               number of rows to move
 *  @param[in]  k               number of columns to move
 *  @param[in]  ld_src          lead dimension size of src
 *
 */
void dataMoveA(double * restrict dst, double * restrict src, int m, int k,
               int ld_src);

/**
 *  @brief Move and re-arrange A' as needed by kernel for dgemm.
 *         src point to A. dst will have blocks of A, each block
 *         arranged in 4 x k (column major)
 *
 *  @param[in]  dst             pointer to destination address
 *  @param[in]  src             pointer to source address
 *  @param[in]  m               number of rows to move
 *  @param[in]  k               number of columns to move
 *  @param[in]  ld_src          lead dimension size of src
 *
 */
void dataMoveAT(double * restrict dst, double * restrict src, int m, int k,
                int ld_src);

#endif
