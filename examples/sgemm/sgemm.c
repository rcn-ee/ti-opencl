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
#include "data.h"
#include "edmamgr.h"
#include "sgemm_kernel.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "dsp_c.h"

#define DSP_wbInv_L1D __cache_l1d_flush
/*extern cregister volatile unsigned int DNUM;*/

// sgemm interface
void sgemm(int m, int n, int k,
		   float* restrict a, int lda,
           float* restrict b, int ldb,
           float* restrict c, int ldc,
           float* restrict pL1, float* restrict pL2, float* restrict MSMC_buf, int tid
           )
{
        int mXferIndex, nXferIndex;
        int kIndex, kCnt, kCntNext;
        int mIndex, mCnt, mCntNext;
        int nIndex, nCnt, nCntNext/*, innerNCnt*/;
        int innerIndex_m, innerIndex_n;
        int flagLastK, flagLastM, flagLastN;
        float * restrict ptrA, * restrict ptrB, * restrict ptrC;
        float * restrict ptrASeg1, * restrict ptrASeg2;
        float * restrict ptrBSeg1, * restrict ptrBSeg2;
        short  indexACurrent, indexANext, indexBCurrent, indexBNext;
        
        EdmaMgr_Handle chan0;
        EdmaMgr_Handle chan1;
        
        // Keep unpacked A panels in MSMC SRAM
        /*ptrASeg1 = MSMC_buf+((int) DNUM)*MPARTITION*KPARTITION;*/
        ptrASeg1 = MSMC_buf+(tid)*MPARTITION*KPARTITION;
        // Move packed A panel in L2 from MSMC SRAM
        ptrASeg2 = pL2; 
        // Keep B panel ping pong buffers in L2 
        ptrBSeg1 = ptrASeg2+(MPARTITION*KPARTITION);
        ptrBSeg2 = ptrBSeg1+(NPARTITION*KPARTITION); 

        mXferIndex = 0;
        nXferIndex = 0;

        mCnt = (m < MPARTITION) ? m : MPARTITION;
        kCnt = (k < KPARTITION) ? k : KPARTITION;
        nCnt = (n < NPARTITION) ? n : NPARTITION;
        
        /* Initialize EDMA Manager */
        chan0 = EdmaMgr_alloc(1);
        chan1 = EdmaMgr_alloc(1);
        if (!chan0 || !chan1) 
        {  
            printf("Failed to alloc edma handle.\n");
        }

        // initiate first transfer of A to MSMC
        DSP_wbInv_L1D();
        EdmaMgr_copy2D2DSep(chan0,
                            a, /* src */
                            ptrASeg1, /* dst */
                            mCnt*sizeof(float), /* num_bytes */
                            kCnt, /* num_lines */
                            lda*sizeof(float), /* src_pitch */
                            MPARTITION*sizeof(float) /* dst_pitch */
                            );
        // initiate first transfer of B to L2
        DSP_wbInv_L1D();
        EdmaMgr_copy2D2DSep(chan1,
                            b, /* src */
                            ptrBSeg1, /* dst */
                            kCnt*sizeof(float), /* num_bytes */
                            nCnt, /* num_lines */
                            ldb*sizeof(float), /* src_pitch */
                            KPARTITION*sizeof(float) /* dst_pitch */
                            );
        indexACurrent=1;
        indexANext=0;
        indexBCurrent=1;
        indexBNext=0;

        for(kIndex=0; kIndex<k; kIndex+=KPARTITION)  // partition in k dimension
        {

            nXferIndex = kIndex;
            kCnt = ((k-kIndex) < KPARTITION) ? (k-kIndex) : KPARTITION;
            kCntNext = ((k-kIndex-KPARTITION) < KPARTITION) ? (k-kIndex-KPARTITION) : KPARTITION;
            flagLastK = ((kIndex+KPARTITION) < k) ? 0 : 1;

            for(mIndex = 0; mIndex<m; mIndex+=MPARTITION)  // partition in m dimension
            {
                mCnt = ((m-mIndex) < MPARTITION) ? (m-mIndex) : MPARTITION;
                mCntNext = ((m-mIndex-MPARTITION) < MPARTITION) ? (m-mIndex-MPARTITION) : MPARTITION;
                flagLastM = ((mIndex+MPARTITION)<m) ? 0 : 1;

                if(flagLastM) mCntNext = (m < MPARTITION) ? m : MPARTITION;

                // bring in A into MSMC SRAM (a new parallel transfer)
                indexACurrent = (indexACurrent+1) & 1;
                indexANext = (indexANext+1) & 1;
                EdmaMgr_wait(chan0);
               
                mXferIndex += mCnt;
                mXferIndex = (!flagLastM) ? mXferIndex: mXferIndex-m+kCnt*lda;
                // move data in desired contiguous location
                dataMoveA(ptrASeg2, ptrASeg1, mCnt, kCnt);
                DSP_wbInv_L1D();
                if ((!flagLastM) || (!flagLastK))
                {
                    if (mIndex == 0)
                        EdmaMgr_copy2D2DSep(chan0,
                                            a+mXferIndex, /* src */
                                            ptrASeg1, /* dst */
                                            mCntNext*sizeof(float), /* num_bytes */
                                            (flagLastM ? kCntNext : kCnt), /* num_lines */
                                            lda*sizeof(float), /* src_pitch */
                                            MPARTITION*sizeof(float) /* dst_pitch */
                                            );
                    else if (flagLastM)
                        EdmaMgr_copy2D2DSep(chan0,
                                            a+mXferIndex, /* src */
                                            ptrASeg1, /* dst */
                                            mCntNext*sizeof(float), /* num_bytes */
                                            kCntNext, /* num_lines */
                                            lda*sizeof(float), /* src_pitch */
                                            MPARTITION*sizeof(float) /* dst_pitch */
                                            );
                    else
                        EdmaMgr_copyFast(chan0,
                                         a+mXferIndex, /* src */
                                         ptrASeg1 /* dst */
                                         );
                }

                for(nIndex = 0; nIndex<n; nIndex+=NPARTITION)  // partition in n dimension
                {
                    nCnt = ((n-nIndex) < NPARTITION) ? (n-nIndex) : NPARTITION;
                    nCntNext = ((n-nIndex-NPARTITION) < NPARTITION) ? (n-nIndex-NPARTITION) : NPARTITION;
                    flagLastN = ((nIndex+NPARTITION)<n) ? 0 : 1;
                    if(flagLastN) nCntNext = (n < NPARTITION) ? n : NPARTITION;

                    // bring in B into L1 SRAM (a new parallel transfer)
                    indexBCurrent = (indexBCurrent+1) & 1;
                    indexBNext = (indexBNext+1) & 1;
                    
                    EdmaMgr_wait(chan1);
                    if((!flagLastM) || (!flagLastK) || (!flagLastN)) // don't carry out DMA for the last iteration
                    {
                        nXferIndex += nCnt*ldb;
                        nXferIndex = (!flagLastN) ? nXferIndex: kIndex;
                        nXferIndex = ((!flagLastN) || (!flagLastM)) ? nXferIndex: (kIndex+kCnt);
                        ptrB = (indexBNext == 0) ? ptrBSeg1: ptrBSeg2;
                        DSP_wbInv_L1D();
                        if (nIndex == 0)
                            EdmaMgr_copy2D2DSep(chan1,
                                                b+nXferIndex, /* src */
                                                ptrB, /* dst */
                                                ((flagLastM && flagLastN) ? kCntNext : kCnt)*sizeof(float), /* num_bytes */
                                                nCntNext, /* num_lines */
                                                ldb*sizeof(float), /* src_pitch */
                                                KPARTITION*sizeof(float) /* dst_pitch */
                                                );
                        else if (flagLastM && flagLastN)
                            EdmaMgr_copy2D2DSep(chan1,
                                                b+nXferIndex, /* src */
                                                ptrB, /* dst */
                                                kCntNext*sizeof(float), /* num_bytes */
                                                nCntNext, /* num_lines */
                                                ldb*sizeof(float), /* src_pitch */
                                                KPARTITION*sizeof(float) /* dst_pitch */
                                                );
                        else
                            EdmaMgr_copyFast(chan1,
                                             b+nXferIndex, /* src */
                                             ptrB /* dst */
                                             );
                    }
                    // L2 memory assignment for B
                    ptrB = (indexBCurrent == 0) ? ptrBSeg1: ptrBSeg2;
                    for(innerIndex_n = 0; innerIndex_n<nCnt; innerIndex_n+=CORE_PROCESS_COLS)
                    {

                        dataMoveB(pL1, ptrB, kCnt);
                        ptrB += (CORE_PROCESS_COLS*KPARTITION);

                        // L2 memory assignment for B
                        ptrA = ptrASeg2;
                        // output memory assignment
                        ptrC= c + mIndex + (nIndex+innerIndex_n)*ldc;
                        for(innerIndex_m = 0; innerIndex_m<mCnt; innerIndex_m+=CORE_PROCESS_ROWS)
                        {

                            // pre-fetch required A to L1 Cache
                            // 4xk * kx8 core matrix multiplications
                            __touch((const char *)ptrA, CORE_PROCESS_ROWS * kCnt * sizeof(float));
                            sgemm_kernel(ptrA, pL1, ptrC, 1.0, kCnt, ldc);
                            // address of C to write to
                            ptrC += CORE_PROCESS_ROWS;
                            ptrA += (CORE_PROCESS_ROWS*KPARTITION);

                        } // inner loop m
                    } // inner loop n
                } // n loop
            } // m loop
        } // k loop
        EdmaMgr_free(chan0);
        EdmaMgr_free(chan1);
}

