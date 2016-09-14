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
#define USE_EDMA 1

#include "data.h"
#include "sgemm_kernel.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "dsp_c.h"
#include "dsp_edmamgr.h"

/*-----------------------------------------------------------------------------
* On KeyStone devices, MSMC is not cached in L2 and this macro could be 
* #define DSP_wbInv_L2  __cache_l1d_flush.  However, the performance delta 
* is negligible.  On Sitara (AM57) devices, the MSMC or (OCMC) is cached in 
* L2 as well as L1D.
*----------------------------------------------------------------------------*/
#define DSP_wbInv_L2  __cache_l2_flush
#define DSP_wbInv_L1D __cache_l1d_flush

// sgemm interface
// Assuming COLUMN MAJOR, NO TRANPOSE ON A AND B
// Computes C[MxN] = alpha * A[MxK] * B[KxN] + beta * C[MxN]
// Requirements: 16KB  of L1 SRAM, passed in as pL1
// Requirements: 128KB of L2 SRAM, passed in as pL2
void sgemm(
           const int m, const int n, const int k,
           const float  alpha,
           float* restrict a, const int lda,
           float* restrict b, const int ldb,
           const float  beta,
           float* restrict c, const int ldc,
           int NUMAPANELS, int NUMBPANELS,
           float* restrict pL1, float* restrict pL2,
           float* restrict pMsmc, int tid
          )
{
    if (m == 0 || n == 0 || ((alpha == 0.0f || k == 0) && beta == 1.0f)) return;

    float __attribute__((aligned(8)))
          ptrCTemp[CORE_PROCESS_ROWS*CORE_PROCESS_COLS];
    int mXferIndex, nXferIndex;
    int kIndex, kCnt, kCntNext;
    int mIndex, mCnt, mCntNext;
    int nIndex, nCnt, nCntNext/*, innerNCnt*/;
#if !(USE_EDMA)
    int kCntPrev, nCntPrev, mCntPrev;
#endif
    int innerIndex_m, innerIndex_n;
    int flagLastK, flagLastM, flagLastN;
    float * restrict ptrA, * restrict ptrB, * restrict ptrC;
    float * restrict ptrASeg1, * restrict ptrASeg2;
    float * restrict ptrBSeg1, * restrict ptrBSeg2;
    short  indexACurrent, indexANext, indexBCurrent, indexBNext;
    float * restrict ptrCInternal;
    int ldcInternal, i, j, nValid, mValid;

    // partition in m dimension
    int MPARTITION = (NUMAPANELS*CORE_PROCESS_ROWS);
    // partition in n dimension
    int NPARTITION = (NUMBPANELS*CORE_PROCESS_COLS);
    
    if (pMsmc)
    {
        // Keep unpacked A panels in MSMC SRAM
        ptrASeg1 = pMsmc+(tid)*MPARTITION*KPARTITION;
    }
    // Move packed A panel in L2 from DDR
    ptrASeg2 = pL2; 
    // Keep B panel ping pong buffers in L2 
    ptrBSeg1 = ptrASeg2+(MPARTITION*KPARTITION);
    ptrBSeg2 = ptrBSeg1+(NPARTITION*KPARTITION); 

    /* Beta scaling of C */
    if(beta != (float) 1.0) // only if scaling of C is needed
    {
        if(beta == (float) 0.0)
        {
            // zero out c column by column
            for(nCnt = 0; nCnt < n; nCnt++)
                memset(c + nCnt*ldc, 0, m * sizeof(float));
        } // if(beta==0.0f)
        else
        {
            // column by column multiplication
            for(nCnt = 0; nCnt < n; nCnt++)
                for(mCnt = 0; mCnt < m; mCnt++)
                    c[nCnt*ldc + mCnt] *= beta;
        } // else
    } // if(beta != 1.0f)

    mXferIndex = 0;
    nXferIndex = 0;

    mCnt = (m < MPARTITION) ? m : MPARTITION;
    kCnt = (k < KPARTITION) ? k : KPARTITION;
    nCnt = (n < NPARTITION) ? n : NPARTITION;

#if USE_EDMA
    /* Initialize EDMA Manager */
    EdmaMgr_Handle chan0, chan1;
    if (pMsmc != NULL)  chan0 = EdmaMgr_alloc(1);
    chan1 = EdmaMgr_alloc(1);
    if ((pMsmc != NULL && !chan0) || !chan1) 
    {  
        printf("Failed to alloc edma handle.\n");
    }
#endif

    if (pMsmc)
    {
        // initiate first transfer of A to MSMC
#if USE_EDMA
        EdmaMgr_copy2D2DSep(chan0,
                            a, /* src */
                            ptrASeg1, /* dst */
                            mCnt*sizeof(float), /* num_bytes */
                            kCnt, /* num_lines */
                            lda*sizeof(float), /* src_pitch */
                            MPARTITION*sizeof(float) /* dst_pitch */
                            );
        DSP_wbInv_L2();  // ptrASeg1
#else
        for (i = 0; i < kCnt; i++)
            memcpy(ptrASeg1 + i * MPARTITION, a + i * lda, mCnt*sizeof(float));
#endif
    }

    // initiate first transfer of B to L2
#if USE_EDMA
    EdmaMgr_copy2D2DSep(chan1,
                        b, /* src */
                        ptrBSeg1, /* dst */
                        kCnt*sizeof(float), /* num_bytes */
                        nCnt, /* num_lines */
                        ldb*sizeof(float), /* src_pitch */
                        KPARTITION*sizeof(float) /* dst_pitch */
                        );
    DSP_wbInv_L1D();  // ptrBSeg1
#else
    for (i = 0; i < nCnt; i++)
        memcpy(ptrBSeg1 + i * KPARTITION, b + i * ldb, kCnt*sizeof(float));
#endif

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
            flagLastM = ((mIndex+MPARTITION)<m) ? 0 : 1;
            mCntNext = ((m-mIndex-MPARTITION) < MPARTITION) ?
                       (m-mIndex-MPARTITION) : MPARTITION;
            if(flagLastM) mCntNext = (m < MPARTITION) ? m : MPARTITION;

            // bring in A into MSMC SRAM (a new parallel transfer)
            indexACurrent = (indexACurrent+1) & 1;
            indexANext = (indexANext+1) & 1;

            // No need to memset invalid rows, because we select results
            // memset((void *) ptrASeg2, 0,
            //        MPARTITION * KPARTITION * sizeof(float));
            if (pMsmc)
            {
#if USE_EDMA
                EdmaMgr_wait(chan0);
#endif
                dataMoveA(ptrASeg2, ptrASeg1, mCnt, kCnt, MPARTITION);
            }
            else
            {
                dataMoveA(ptrASeg2, a+mXferIndex, mCnt, kCnt, lda);
            }

            mXferIndex += mCnt;
            mXferIndex = (!flagLastM) ? mXferIndex: mXferIndex-m+kCnt*lda;

            if (pMsmc)
            {
                if ((!flagLastM) || (!flagLastK))
                {
                    if (mIndex == 0)
                    {
#if USE_EDMA
                        EdmaMgr_copy2D2DSep(chan0,
                                            a+mXferIndex, /* src */
                                            ptrASeg1, /* dst */
                                            mCntNext*sizeof(float), /* num_bytes */
                                            (flagLastM ? kCntNext : kCnt), /* num_lines */
                                            lda*sizeof(float), /* src_pitch */
                                            MPARTITION*sizeof(float) /* dst_pitch */
                                            );
                        DSP_wbInv_L2();  // ptrASeg1
#else
                        kCntPrev = (flagLastM ? kCntNext : kCnt);
                        for (i = 0; i < kCntPrev; i++)
                            memcpy(ptrASeg1 + i * MPARTITION,
                                   a+mXferIndex + i * lda,
                                   mCntNext*sizeof(float));
                        mCntPrev = mCntNext;
#endif
                    }
                    else if (flagLastM)
                    {
#if USE_EDMA
                        EdmaMgr_copy2D2DSep(chan0,
                                            a+mXferIndex, /* src */
                                            ptrASeg1, /* dst */
                                            mCntNext*sizeof(float), /* num_bytes */
                                            kCntNext, /* num_lines */
                                            lda*sizeof(float), /* src_pitch */
                                            MPARTITION*sizeof(float) /* dst_pitch */
                                            );
                        DSP_wbInv_L2();  // ptrASeg1
#else
                        kCntPrev = kCntNext;
                        for (i = 0; i < kCntPrev; i++)
                            memcpy(ptrASeg1 + i * MPARTITION,
                                   a+mXferIndex + i * lda,
                                   mCntNext*sizeof(float));
                        mCntPrev = mCntNext;
#endif
                    }
                    else
                    {
#if USE_EDMA
                        EdmaMgr_copyFast(chan0,
                                         a+mXferIndex, /* src */
                                         ptrASeg1 /* dst */
                                         );
                        DSP_wbInv_L2();  // ptrASeg1
#else
                        for (i = 0; i < kCntPrev; i++)
                            memcpy(ptrASeg1 + i * MPARTITION,
                                   a+mXferIndex + i * lda,
                                   mCntPrev*sizeof(float));
#endif
                    }
                }
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
                
#if USE_EDMA
                EdmaMgr_wait(chan1);
#endif

                if((!flagLastM) || (!flagLastK) || (!flagLastN)) // don't carry out DMA for the last iteration
                {
                    nXferIndex += nCnt*ldb;
                    nXferIndex = (!flagLastN) ? nXferIndex: kIndex;
                    nXferIndex = ((!flagLastN) || (!flagLastM)) ? nXferIndex: (kIndex+kCnt);
                    ptrB = (indexBNext == 0) ? ptrBSeg1: ptrBSeg2;
                    if (nIndex == 0)
                    {
#if USE_EDMA
                        EdmaMgr_copy2D2DSep(chan1,
                                            b+nXferIndex, /* src */
                                            ptrB, /* dst */
                                            ((flagLastM && flagLastN) ? kCntNext : kCnt)*sizeof(float), /* num_bytes */
                                            nCntNext, /* num_lines */
                                            ldb*sizeof(float), /* src_pitch */
                                            KPARTITION*sizeof(float) /* dst_pitch */
                                            );
                        DSP_wbInv_L1D();  // ptrB
#else
                        for (i = 0; i < nCntNext; i++)
                            memcpy(ptrB + i * KPARTITION,
                                   b+nXferIndex + i * ldb,
                                   ((flagLastM && flagLastN)
                                    ? kCntNext : kCnt)*sizeof(float));
                        nCntPrev = nCntNext;
                        kCntPrev = (flagLastM && flagLastN) ? kCntNext : kCnt;
#endif
                    }
                    else if (flagLastM && flagLastN)
                    {
#if USE_EDMA
                        EdmaMgr_copy2D2DSep(chan1,
                                            b+nXferIndex, /* src */
                                            ptrB, /* dst */
                                            kCntNext*sizeof(float), /* num_bytes */
                                            nCntNext, /* num_lines */
                                            ldb*sizeof(float), /* src_pitch */
                                            KPARTITION*sizeof(float) /* dst_pitch */
                                            );
                        DSP_wbInv_L1D();  // ptrB
#else
                        for (i = 0; i < nCntNext; i++)
                            memcpy(ptrB + i * KPARTITION,
                                   b+nXferIndex + i * ldb,
                                   kCntNext*sizeof(float));
                        nCntPrev = nCntNext;
                        kCntPrev = kCntNext;
#endif
                    }
                    else
                    {
#if USE_EDMA
                        EdmaMgr_copyFast(chan1,
                                         b+nXferIndex, /* src */
                                         ptrB /* dst */
                                         );
                        DSP_wbInv_L1D();  // ptrB
#else
                        for (i = 0; i < nCntPrev; i++)
                            memcpy(ptrB + i * KPARTITION,
                                   b+nXferIndex + i * ldb,
                                   kCntPrev*sizeof(float));
#endif
                    }
                }

                // L2 memory assignment for B
                ptrB = (indexBCurrent == 0) ? ptrBSeg1: ptrBSeg2;
                // Corner case, zero out invalid cols in ptrB
                if (flagLastN && (nCnt % CORE_PROCESS_COLS != 0))
                {
                    for (i = 0; i < CORE_PROCESS_COLS -
                                    nCnt%CORE_PROCESS_COLS; i++)
                        memset((void *) (ptrB + (i+nCnt)*KPARTITION), 0,
                               kCnt * sizeof(float));
                }
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
                        if (ldc % 2 != 0 ||
                            innerIndex_n + CORE_PROCESS_COLS > nCnt ||
                            innerIndex_m + CORE_PROCESS_ROWS > mCnt)
                        {
                            nValid = (nCnt - innerIndex_n > CORE_PROCESS_COLS)
                                     ? CORE_PROCESS_COLS : nCnt - innerIndex_n;
                            mValid = (mCnt - innerIndex_m > CORE_PROCESS_ROWS)
                                     ? CORE_PROCESS_ROWS : mCnt - innerIndex_m;
                            for (j = 0; j < nValid; j++)
                                for (i = 0; i < mValid; i++)
                                    ptrCTemp[j*CORE_PROCESS_ROWS + i] =
                                                           ptrC[j*ldc + i];
                            ptrCInternal = ptrCTemp;
                            ldcInternal  = CORE_PROCESS_ROWS;
                        }
                        else
                        {
                            ptrCInternal = ptrC;
                            ldcInternal  = ldc;;
                        }
                        // pre-fetch required A to L1 Cache
                        // 4xk * kx8 core matrix multiplications
                        __touch((const char *)ptrA,
                                CORE_PROCESS_ROWS * kCnt * sizeof(float));

                        sgemm_kernel(ptrA, pL1, ptrCInternal, alpha, kCnt,
                                     ldcInternal);

                        if (ldc % 2 != 0 ||
                            innerIndex_n + CORE_PROCESS_COLS > nCnt ||
                            innerIndex_m + CORE_PROCESS_ROWS > mCnt)
                        {
                            for (j = 0; j < nValid; j++)
                                for (i = 0; i < mValid; i++)
                                    ptrC[j*ldc + i] =
                                         ptrCTemp[j*CORE_PROCESS_ROWS + i];
                        }

                        // address of C to write to
                        ptrC += CORE_PROCESS_ROWS;
                        ptrA += (CORE_PROCESS_ROWS*KPARTITION);

                    } // inner loop m

                } // inner loop n
            } // n loop
        } // m loop
    } // k loop

#if USE_EDMA
    if (pMsmc)  EdmaMgr_free(chan0);
    EdmaMgr_free(chan1);
#endif
}

// sgemm row major interface, calls sgemm in turn
// Requirements: 16KB  of L1 SRAM, passed in as pL1
// Requirements: 128KB of L2 SRAM, passed in as pL2
void sgemm_rowmajor(
                 const int M, const int N, const int K,
                 const float alpha, float* restrict A, const int lda,
                 float* restrict B, const int ldb,
                 const float beta, float* restrict C, const int ldc,
                 int NUMAPANELS, int NUMBPANELS,
                 float* restrict pL1, float* restrict pL2,
                 float* restrict pMsmc, int tid)
{
       /*----------------------------------------------------------------------
       * Convert RowMajor computation to ColumnMajor computation
       * Fact: Mem_Layout(A_RowMajor) === Mem_Layout(Transpose(A)_ColMajor)
       * Therefore: C_RowMajor = A_RowMajor * B_RowMajor
       *            C[mxn] = A[mxk] * B[kxn]
       * can be computed as:
       * Transpose(C)_ColMajor = Transpose(B)_ColMajor * Transpose(A)_ColMajor
       * C'[nxm] = B'[nxk] * A'[kxm],
       * where ptrC' === ptrC, ptrA' === ptrA, ptrB' === ptrB
       * So, all we need to do is to: swap(m, n), swap(a, b)
       * ldA'_col = ldA_row = k, 
       * ldB'_col = ldB_row = n, 
       * ldC'_col = ldC_row = n, 
       *---------------------------------------------------------------------*/
       sgemm(N, M, K, alpha, B, ldb, A, lda, beta, C, ldc, 
             NUMAPANELS, NUMBPANELS, pL1, pL2, pMsmc, tid);
}


