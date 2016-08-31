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
#include "dsp_c.h"
#include "dsp_edmamgr.h"
#include "cblas.h"
#include "dgemm_kernel.h"
#include "c6x.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define  SIZEOF_DOUBLE  8


// dgemm interface
void dgemm(int transa, int transb,
           double alpha, double beta,
           int m, int n, int k,
           double* restrict a, int lda,
           double* restrict b, int ldb,
           double* restrict c, int ldc,
           int NUMAPANELS, int NUMBPANELS,
           double* restrict pL1, double* restrict pL2,
           double* restrict MSMC_buf, int tid
          )
{
    double __attribute__((aligned(8)))
           ptrCSeg[CORE_PROCESS_ROWS*CORE_PROCESS_COLS];
    int aXferIndex, bXferIndex;
    int kIndex, kCnt, kCntNext;
    int mIndex, mCnt, mCntNext;
    int nIndex, nCnt, nCntNext/*, innerNCnt*/;
    int innerIndex_m, innerIndex_n;
    int flagLastK, flagLastM, flagLastN;
    double * restrict ptrA, * restrict ptrB, * restrict ptrC;
    double * restrict ptrASeg1, * restrict ptrASeg2;
    double * restrict ptrBSeg1, * restrict ptrBSeg2;
    double * restrict ptrBSeg;
    short  indexACurrent, indexANext, indexBCurrent, indexBNext;
    int flagTransA = 0, flagTransB = 0;
    short flagUseDMACopyA = 1, flagUseDMACopyB = 1;

    // partition in m dimension
    int MPARTITION = (NUMAPANELS*CORE_PROCESS_ROWS);
    // partition in n dimension
    int NPARTITION = (NUMBPANELS*CORE_PROCESS_COLS);

    EdmaMgr_Handle chan0;
    EdmaMgr_Handle chan1;

    /* Check if transpose required and set flag */
    if(transa == CblasNoTrans) flagTransA=0;
    else if(transa== CblasTrans) flagTransA=1;

    if(transb == CblasNoTrans) flagTransB=0;
    else if(transb == CblasTrans) flagTransB=1;

    if (MSMC_buf != NULL)
    {
        // Keep unpacked A panels in MSMC SRAM
        ptrASeg1 = MSMC_buf+(tid)*MPARTITION*KPARTITION;
    }
    // Move packed A panel in L2 from MSMC SRAM
    ptrASeg2 = pL2;
    ptrBSeg  = pL1;

    // Keep B panel ping pong buffers in L2
    ptrBSeg1 = ptrASeg2+(MPARTITION*KPARTITION);
    ptrBSeg2 = ptrBSeg1+(NPARTITION*KPARTITION);

    /* Beta scaling of C */
    if(beta != (double) 1.0) // only if scaling of C is needed
    {
        if(beta==(double) 0.0)
        {
            for(nCnt=0;nCnt<n;nCnt++)
            {
                memset(c+nCnt*ldc,0,m*SIZEOF_DOUBLE); // zero out c column by column
            }
        } // if(beta==0.0f)
        else
        {
            for(nCnt=0;nCnt<n;nCnt++)
            {
                for(mCnt=0;mCnt<m;mCnt++)
                {
                    c[nCnt*ldc+mCnt] *= beta; // column by column multiplication
                }
            }
        } // else
    } // if(beta != 1.0f)

    aXferIndex = 0;
    bXferIndex = 0;

    mCnt = (m < MPARTITION) ? m : MPARTITION;
    kCnt = (k < KPARTITION) ? k : KPARTITION;
    nCnt = (n < NPARTITION) ? n : NPARTITION;

    /* Initialize EDMA Manager */
    if (MSMC_buf != NULL)  chan0 = EdmaMgr_alloc(1);
    chan1 = EdmaMgr_alloc(1);
    if ((MSMC_buf != NULL && !chan0) || !chan1)
    {
        printf("Failed to alloc edma handle.\n");
    }

    if(lda*SIZEOF_DOUBLE < MAXDMASTRIDE) flagUseDMACopyA = 1; // use DMA
    else flagUseDMACopyA = 0; // cannot use DMA since stride is only 16 bit signed

    if(ldb*SIZEOF_DOUBLE < MAXDMASTRIDE) flagUseDMACopyB = 1; // use DMA
    else flagUseDMACopyB = 0; // cannot use DMA since stride is only 16 bit signed

    // initiate first transfer of A to MSMC
    if (MSMC_buf != NULL)
    {
        if (flagTransA == 0)
        {
            if (flagUseDMACopyA)
            {
                EdmaMgr_copy2D2DSep(chan0,
                        a, /* src */
                        ptrASeg1, /* dst */
                        mCnt*SIZEOF_DOUBLE, /* num_bytes */
                        kCnt, /* num_lines */
                        lda*SIZEOF_DOUBLE, /* src_pitch */
                        MPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                        );
                __cache_l2_flush();
            }else
            {
                for(innerIndex_m=0; innerIndex_m< kCnt; innerIndex_m++)
                    memcpy(ptrASeg1 + innerIndex_m * MPARTITION, a+innerIndex_m*lda, mCnt*SIZEOF_DOUBLE);

            }
        }else // A is in transposed form
        {
            if (flagUseDMACopyA)
            {
                EdmaMgr_copy2D2DSep(chan0,
                        a, /* src */
                        ptrASeg1, /* dst */
                        kCnt*SIZEOF_DOUBLE, /* num_bytes */
                        mCnt, /* num_lines */
                        lda*SIZEOF_DOUBLE, /* src_pitch */
                        KPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                        );
                __cache_l2_flush();
            }else
            {
                for(innerIndex_m=0; innerIndex_m< kCnt; innerIndex_m++)
                    memcpy(ptrASeg1 + innerIndex_m * KPARTITION, a+innerIndex_m*lda, kCnt*SIZEOF_DOUBLE);

            }
        }
    }

    // initiate first transfer of B to L2
    if (flagTransB == 0)
    {
        if (flagUseDMACopyB)
        {
            EdmaMgr_copy2D2DSep(chan1,
                    b, /* src */
                    ptrBSeg1, /* dst */
                    kCnt*SIZEOF_DOUBLE, /* num_bytes */
                    nCnt, /* num_lines */
                    ldb*SIZEOF_DOUBLE, /* src_pitch */
                    KPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                    );
            __cache_l1d_flush();
        }else
        {
            for(innerIndex_m=0;innerIndex_m< nCnt; innerIndex_m++)
                memcpy(ptrBSeg1 + innerIndex_m * KPARTITION, b+innerIndex_m*ldb, kCnt*SIZEOF_DOUBLE);

        }
    }else // B is in transposed form
    {
        if (flagUseDMACopyB)
        {
            EdmaMgr_copy2D2DSep(chan1,
                    b, /* src */
                    ptrBSeg1, /* dst */
                    nCnt*SIZEOF_DOUBLE, /* num_bytes */
                    kCnt, /* num_lines */
                    ldb*SIZEOF_DOUBLE, /* src_pitch */
                    NPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                    );
            __cache_l1d_flush();
        }else
        {
            for(innerIndex_m=0;innerIndex_m< nCnt; innerIndex_m++)
                memcpy(ptrBSeg1 + innerIndex_m * NPARTITION, b+innerIndex_m*ldb, nCnt*SIZEOF_DOUBLE);

        }

    }

    indexACurrent=1;
    indexANext=0;
    indexBCurrent=1;
    indexBNext=0;

    /*#pragma MUST_ITERATE(1, ,KPARTITION)*/
    for(kIndex=0; kIndex<k; kIndex+=KPARTITION)  // partition in k dimension
    {
        // This is GEPP loop
        if (flagTransB == 0) bXferIndex = kIndex;
        else bXferIndex = kIndex*ldb; // B is in transposed form

        kCnt = ((k-kIndex) < KPARTITION) ? (k-kIndex) : KPARTITION;
        kCntNext = ((k-kIndex-KPARTITION) < KPARTITION) ? (k-kIndex-KPARTITION) : KPARTITION;
        flagLastK = ((kIndex+KPARTITION) < k) ? 0 : 1;

        // #pragma MUST_ITERATE(1, ,MPARTITION)
        for(mIndex = 0; mIndex<m; mIndex+=MPARTITION)  // partition in m dimension
        {
            // This is GEPB loop
            mCnt = ((m-mIndex) < MPARTITION) ? (m-mIndex) : MPARTITION;
            mCntNext = ((m-mIndex-MPARTITION) < MPARTITION) ? (m-mIndex-MPARTITION) : MPARTITION;
            flagLastM = ((mIndex+MPARTITION)<m) ? 0 : 1;

            if(flagLastM) mCntNext = (m < MPARTITION) ? m : MPARTITION;

            // bring in A into MSMC SRAM (a new parallel transfer)
            indexACurrent = (indexACurrent+1) & 1;
            indexANext = (indexANext+1) & 1;

            if (MSMC_buf != NULL)
            {
                if (flagUseDMACopyA) EdmaMgr_wait(chan0);
                if (flagTransA == 0)
                {
                    aXferIndex += mCnt;
                    aXferIndex = (!flagLastM) ? aXferIndex: aXferIndex-m+kCnt*lda;
#if 1
                    // zero out memory (06/23/2014);
                    if(flagLastM)
                    {
                        for(innerIndex_m=0;innerIndex_m< kCnt; innerIndex_m++)
                            memset((void *) (ptrASeg1+innerIndex_m*MPARTITION+mCnt), 0,
                                    (((MPARTITION-mCnt) > CORE_PROCESS_ROWS) ? CORE_PROCESS_ROWS : MPARTITION-mCnt) *SIZEOF_DOUBLE);
                    }
#endif
                    // move A to L2 SRAM in desired contiguous location
                    dataMoveA(ptrASeg2, ptrASeg1, mCnt, kCnt, MPARTITION);
                    if ((!flagLastM) || (!flagLastK))
                    {
                        if (flagUseDMACopyA)
                        {
                            if (mIndex == 0)
                                EdmaMgr_copy2D2DSep(chan0,
                                        a+aXferIndex, /* src */
                                        ptrASeg1, /* dst */
                                        mCntNext*SIZEOF_DOUBLE, /* num_bytes */
                                        (flagLastM ? kCntNext : kCnt), /* num_lines */
                                        lda*SIZEOF_DOUBLE, /* src_pitch */
                                        MPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else if (flagLastM)
                                EdmaMgr_copy2D2DSep(chan0,
                                        a+aXferIndex, /* src */
                                        ptrASeg1, /* dst */
                                        mCntNext*SIZEOF_DOUBLE, /* num_bytes */
                                        kCntNext, /* num_lines */
                                        lda*SIZEOF_DOUBLE, /* src_pitch */
                                        MPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else
                                EdmaMgr_copyFast(chan0,
                                        a+aXferIndex, /* src */
                                        ptrASeg1 /* dst */
                                        );
                            __cache_l2_flush();
                        }else
                        {
                            for(innerIndex_m=0;innerIndex_m< (flagLastM ? kCntNext : kCnt); innerIndex_m++)
                                memcpy(ptrASeg1+innerIndex_m*MPARTITION, a+aXferIndex+innerIndex_m*lda, mCntNext*SIZEOF_DOUBLE);

                        }
                    }
                }else // A is in transposed form
                {
                    aXferIndex += mCnt*lda;
                    aXferIndex = (!flagLastM) ? aXferIndex: aXferIndex-m*lda+kCnt;
#if 1
                    // zero out memory (06/23/2014);
                    if(flagLastM)
                    {
                        for(innerIndex_m=0;innerIndex_m< (((MPARTITION-mCnt) > CORE_PROCESS_ROWS) ? CORE_PROCESS_ROWS : MPARTITION-mCnt); innerIndex_m++)
                            memset((void *) (ptrASeg1+(innerIndex_m+mCnt)*KPARTITION), 0, kCnt*SIZEOF_DOUBLE);
                    }
#endif
                    // move A to L2 SRAM in desired contiguous location
                    dataMoveAT(ptrASeg2, ptrASeg1, mCnt, kCnt, KPARTITION);

                    if ((!flagLastM) || (!flagLastK))
                    {
                        if (flagUseDMACopyA)
                        {
                            if (mIndex == 0)
                                EdmaMgr_copy2D2DSep(chan0,
                                        a+aXferIndex, /* src */
                                        ptrASeg1, /* dst */
                                        (flagLastM ? kCntNext : kCnt)*SIZEOF_DOUBLE, /* num_bytes */
                                        mCntNext, /* num_lines */
                                        lda*SIZEOF_DOUBLE, /* src_pitch */
                                        KPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else if (flagLastM)
                                EdmaMgr_copy2D2DSep(chan0,
                                        a+aXferIndex, /* src */
                                        ptrASeg1, /* dst */
                                        kCntNext*SIZEOF_DOUBLE, /* num_bytes */
                                        mCntNext, /* num_lines */
                                        lda*SIZEOF_DOUBLE, /* src_pitch */
                                        KPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else
                                EdmaMgr_copyFast(chan0,
                                        a+aXferIndex, /* src */
                                        ptrASeg1 /* dst */
                                        );
                            __cache_l2_flush();
                        }else
                        {
                            for(innerIndex_m=0;innerIndex_m< mCntNext; innerIndex_m++)
                                memcpy(ptrASeg1+innerIndex_m*KPARTITION, a+aXferIndex+innerIndex_m*lda, (flagLastM ? kCntNext : kCnt)*SIZEOF_DOUBLE);

                        }
                    }
                }
            }
            else
            {
                // move A to L2 SRAM in desired contiguous location
                if (flagTransA == 0)
                {
                    dataMoveA(ptrASeg2, a+aXferIndex, mCnt, kCnt, lda);
                    aXferIndex += mCnt;
                    aXferIndex = (!flagLastM) ? aXferIndex: aXferIndex-m+kCnt*lda;
                }
                else
                {
                    dataMoveAT(ptrASeg2, a+aXferIndex, mCnt, kCnt, lda);
                    aXferIndex += mCnt*lda;
                    aXferIndex = (!flagLastM) ? aXferIndex: aXferIndex-m*lda+kCnt;
                }
            }

            /*#pragma MUST_ITERATE(1, ,NPARTITION)*/
            for(nIndex = 0; nIndex<n; nIndex+=NPARTITION)  // partition in n dimension
            {
                nCnt = ((n-nIndex) < NPARTITION) ? (n-nIndex) : NPARTITION;
                nCntNext = ((n-nIndex-NPARTITION) < NPARTITION) ? (n-nIndex-NPARTITION) : NPARTITION;
                flagLastN = ((nIndex+NPARTITION)<n) ? 0 : 1;
                if(flagLastN) nCntNext = (n < NPARTITION) ? n : NPARTITION;

                // bring in B into L1 SRAM (a new parallel transfer)
                indexBCurrent = (indexBCurrent+1) & 1;
                indexBNext = (indexBNext+1) & 1;

                if(flagUseDMACopyB) EdmaMgr_wait(chan1);

                if((!flagLastM) || (!flagLastK) || (!flagLastN)) // don't carry out DMA for the last iteration
                {
                    if (flagTransB == 0)
                    {
                        bXferIndex += nCnt*ldb;
                        bXferIndex = (!flagLastN) ? bXferIndex: kIndex;
                        bXferIndex = ((!flagLastN) || (!flagLastM)) ? bXferIndex: (kIndex+kCnt);
                        ptrB = (indexBNext == 0) ? ptrBSeg1: ptrBSeg2;
                        if (flagUseDMACopyB)
                        {
                            if (nIndex == 0)
                                EdmaMgr_copy2D2DSep(chan1,
                                        b+bXferIndex, /* src */
                                        ptrB, /* dst */
                                        ((flagLastM && flagLastN) ? kCntNext : kCnt)*SIZEOF_DOUBLE, /* num_bytes */
                                        nCntNext, /* num_lines */
                                        ldb*SIZEOF_DOUBLE, /* src_pitch */
                                        KPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else if (flagLastM && flagLastN)
                                EdmaMgr_copy2D2DSep(chan1,
                                        b+bXferIndex, /* src */
                                        ptrB, /* dst */
                                        kCntNext*SIZEOF_DOUBLE, /* num_bytes */
                                        nCntNext, /* num_lines */
                                        ldb*SIZEOF_DOUBLE, /* src_pitch */
                                        KPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else
                                EdmaMgr_copyFast(chan1,
                                        b+bXferIndex, /* src */
                                        ptrB /* dst */
                                        );
                            __cache_l1d_flush();
                        }else
                        {
                            for(innerIndex_m=0;innerIndex_m< nCntNext; innerIndex_m++)
                                memcpy(ptrB + innerIndex_m * KPARTITION, b+bXferIndex+innerIndex_m*ldb, ((flagLastM && flagLastN) ? kCntNext : kCnt)*SIZEOF_DOUBLE);
                        }
                    }else // B is in transposed form
                    {
                        bXferIndex += nCnt;
                        bXferIndex = (!flagLastN) ? bXferIndex: kIndex*ldb;
                        bXferIndex = ((!flagLastN) || (!flagLastM)) ? bXferIndex: (kIndex+kCnt)*ldb;
                        ptrB = (indexBNext == 0) ? ptrBSeg1: ptrBSeg2;
                        if (flagUseDMACopyB)
                        {
                            if (nIndex == 0)
                                EdmaMgr_copy2D2DSep(chan1,
                                        b+bXferIndex, /* src */
                                        ptrB, /* dst */
                                        nCntNext*SIZEOF_DOUBLE, /* num_bytes */
                                        ((flagLastM && flagLastN) ? kCntNext : kCnt), /* num_lines */
                                        ldb*SIZEOF_DOUBLE, /* src_pitch */
                                        NPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else if (flagLastM && flagLastN)
                                EdmaMgr_copy2D2DSep(chan1,
                                        b+bXferIndex, /* src */
                                        ptrB, /* dst */
                                        nCntNext*SIZEOF_DOUBLE, /* num_bytes */
                                        kCntNext, /* num_lines */
                                        ldb*SIZEOF_DOUBLE, /* src_pitch */
                                        NPARTITION*SIZEOF_DOUBLE /* dst_pitch */
                                        );
                            else
                                EdmaMgr_copyFast(chan1,
                                        b+bXferIndex, /* src */
                                        ptrB /* dst */
                                        );
                            __cache_l1d_flush();
                        }else
                        {
                            for(innerIndex_m=0;innerIndex_m< ((flagLastM && flagLastN) ? kCntNext : kCnt); innerIndex_m++)
                                memcpy(ptrB + innerIndex_m * NPARTITION, b+bXferIndex+innerIndex_m*ldb, nCntNext*SIZEOF_DOUBLE);
                        }
                    }
                }
                // L2 memory assignment for B
                ptrB = (indexBCurrent == 0) ? ptrBSeg1: ptrBSeg2;
#if 1
                // zero out memory (06/23/2014);
                if(flagTransB == 0)
                {
                    if(flagLastN)
                    {
                        for(innerIndex_m=0;innerIndex_m< (((NPARTITION-nCnt) > CORE_PROCESS_COLS) ? CORE_PROCESS_COLS : NPARTITION-nCnt); innerIndex_m++)
                            memset(ptrB + (innerIndex_m + nCnt) * KPARTITION, 0, kCnt*SIZEOF_DOUBLE);
                    }
                }
                else // B is in transposed form
                {
                    if(flagLastN)
                    {
                        for(innerIndex_m=0;innerIndex_m< kCnt; innerIndex_m++)
                            memset(ptrB + innerIndex_m * NPARTITION + nCnt, 0, (((NPARTITION-nCnt) > CORE_PROCESS_COLS) ? CORE_PROCESS_COLS : NPARTITION-nCnt)*SIZEOF_DOUBLE);
                    }
                }
#endif
                /*#pragma MUST_ITERATE(1,NPARTITION,CORE_PROCESS_COLS)*/
                for(innerIndex_n = 0; innerIndex_n<nCnt; innerIndex_n+=CORE_PROCESS_COLS)
                {
                    // Move B to L1 SRAM in desired contiguous arrangement
                    if (flagTransB == 0)
                    {
                        dataMoveB(ptrBSeg, ptrB, kCnt);
                        ptrB += (CORE_PROCESS_COLS*KPARTITION);
                    }else // B is in transposed form
                    {
                        dataMoveBT(ptrBSeg, ptrB, kCnt, NPARTITION);
                        ptrB += (CORE_PROCESS_COLS);
                    }

                    // L2 memory assignment for B
                    ptrA = ptrASeg2;
                    // output memory assignment
                    ptrC= c + mIndex + (nIndex+innerIndex_n)*ldc;
                    /*#pragma MUST_ITERATE(1,MPARTITION,CORE_PROCESS_ROWS)*/
                    for(innerIndex_m = 0; innerIndex_m<mCnt; innerIndex_m+=CORE_PROCESS_ROWS)
                    {
#if 1
                        int cornerCase, nCntInternal, mCntInternal,
                            newIndex_m, newIndex_n, ldcInternal;
                        double * restrict ptrCInternal;

                        // check if we are in corner case i.e.,
                        // in region not mutliple of inner kernel needs
                        // if so we read into internal memory
                        // and write back later
                        cornerCase = (((innerIndex_n+CORE_PROCESS_COLS) > nCnt) || ((innerIndex_m+CORE_PROCESS_ROWS) > mCnt));
                        if(cornerCase)
                        {
                            nCntInternal = (nCnt -innerIndex_n) > CORE_PROCESS_COLS ? CORE_PROCESS_COLS : nCnt -innerIndex_n;
                            mCntInternal = (mCnt -innerIndex_m) > CORE_PROCESS_ROWS ? CORE_PROCESS_ROWS : mCnt -innerIndex_m;
                            for(newIndex_n=0;newIndex_n<nCntInternal;newIndex_n++)
                                for(newIndex_m=0;newIndex_m<mCntInternal;newIndex_m++)
                                    ptrCSeg[newIndex_n*CORE_PROCESS_ROWS+newIndex_m]=ptrC[newIndex_n*ldc+newIndex_m];
                            ldcInternal = CORE_PROCESS_ROWS;
                            ptrCInternal = ptrCSeg;

                        }
                        else
                        {
                            ldcInternal = ldc;
                            ptrCInternal = ptrC;
                        }
#endif

                        // pre-fetch required A to L1 Cache
                        __touch((char*)ptrA, CORE_PROCESS_ROWS * kCnt * SIZEOF_DOUBLE);
                        dgemm_kernel(ptrA, ptrBSeg, ptrCInternal, alpha, kCnt, ldcInternal);
#if 1
                        if(cornerCase)
                        {
                            for(newIndex_n=0;newIndex_n<nCntInternal;newIndex_n++)
                                for(newIndex_m=0;newIndex_m<mCntInternal;newIndex_m++)
                                    ptrC[newIndex_n*ldc+newIndex_m]=ptrCInternal[newIndex_n*CORE_PROCESS_ROWS+newIndex_m];

                        }
#endif
                        // address of C to write to
                        ptrC += CORE_PROCESS_ROWS;
                        ptrA += (CORE_PROCESS_ROWS*KPARTITION);

                    } // inner loop m
                } // inner loop n
            } // n loop
        } // m loop
    } // k loop
    if (MSMC_buf != NULL)  EdmaMgr_free(chan0);
    EdmaMgr_free(chan1);
}

