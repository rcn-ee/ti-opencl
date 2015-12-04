void alg_create(void **pAlgHandle) { }

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
#include <stdio.h>
#include <string.h>

// moving B from L2 to L1 (needs transpose)
void dataMoveB(float * restrict dst, float * restrict src, int k)
{
    int kCnt;
    int srcAddr0, srcAddr1, srcAddr2, srcAddr3;
    int srcAddr4, srcAddr5, srcAddr6, srcAddr7;
    int dstAddr;

    // Assuming src is in COL MAJOR
    srcAddr0 = 0;
    srcAddr1 = KPARTITION/*k*/;
    srcAddr2 = 2*KPARTITION/*k*/;
    srcAddr3 = 3*KPARTITION/*k*/;
    srcAddr4 = 4*KPARTITION/*k*/;
    srcAddr5 = 5*KPARTITION/*k*/;
    srcAddr6 = 6*KPARTITION/*k*/;
    srcAddr7 = 7*KPARTITION/*k*/;
    dstAddr  = 0;

    _nassert((unsigned int) src%8 == 0);
    _nassert((unsigned int) dst%8 == 0);
    //#pragma UNROLL(2) // unrolling by 2; valid for even k
    for(kCnt=0; kCnt<((k>>1)<<1); kCnt++)
    {
        float f0, f1, f2, f3, f4, f5, f6, f7;
        f0=src[srcAddr0];
        f1=src[srcAddr1];
        f2=src[srcAddr2];
        f3=src[srcAddr3];
        f4=src[srcAddr4];
        f5=src[srcAddr5];
        f6=src[srcAddr6];
        f7=src[srcAddr7];

        srcAddr0++;
        srcAddr1++;
        srcAddr2++;
        srcAddr3++;
        srcAddr4++;
        srcAddr5++;
        srcAddr6++;
        srcAddr7++;

        dst[dstAddr] = f0;
        dst[dstAddr+1] = f1;
        dst[dstAddr+2] = f2;
        dst[dstAddr+3] = f3;
        dst[dstAddr+4] = f4;
        dst[dstAddr+5] = f5;
        dst[dstAddr+6] = f6;
        dst[dstAddr+7] = f7;
        /*memset(&dst[dstAddr],0, 8*sizeof(float));*/

        /*printf("f0-7: %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f\n", */
                        /*f0, f1, f2, f3, f4, f5, f6, f7);*/

        dstAddr += CORE_PROCESS_COLS;
    }
    if((k&1)==1) // odd k; one more value to take care
    {
        float f0, f1, f2, f3, f4, f5, f6, f7;
        f0=src[srcAddr0];
        f1=src[srcAddr1];
        f2=src[srcAddr2];
        f3=src[srcAddr3];
        f4=src[srcAddr4];
        f5=src[srcAddr5];
        f6=src[srcAddr6];
        f7=src[srcAddr7];

        dst[dstAddr] = f0;
        dst[dstAddr+1] = f1;
        dst[dstAddr+2] = f2;
        dst[dstAddr+3] = f3;
        dst[dstAddr+4] = f4;
        dst[dstAddr+5] = f5;
        dst[dstAddr+6] = f6;
        dst[dstAddr+7] = f7;
    }
}

// moving data from A to format needed for kernel
// from MSMC to L2, or from DDR directly to L2
void dataMoveA(float * restrict dst, float * restrict src, int m, int k,
               int ld_src)
{
    int kCnt, mCnt, mLeft;
    int srcAddr, dstAddr;

    mLeft=m-((m>>2)<<2);

    _nassert((unsigned int) src%8 == 0);
    _nassert((unsigned int) dst%8 == 0);
    for(kCnt=0;kCnt<k;kCnt++){
        for(mCnt=0;mCnt<(m>>2);mCnt++)
        {
            float f0, f1, f2, f3;
            srcAddr = (kCnt*ld_src/*m*/)+(mCnt<<2);
            dstAddr = (mCnt*(KPARTITION/*k*/<<2))+(kCnt<<2);
            // load CORE_PROCESS_ROWS floats sequentially
            f0 = src[srcAddr];
            f1 = src[srcAddr+1];
            f2 = src[srcAddr+2];
            f3 = src[srcAddr+3];

            // push CORE_PROCESS_ROWS floats to desired loc
            dst[dstAddr] = f0;
            dst[dstAddr+1] = f1;
            dst[dstAddr+2] = f2;
            dst[dstAddr+3] = f3;
        }
        // leftover in m dimension of A
        srcAddr = kCnt*ld_src/*m*/+((m>>2)<<2);
        dstAddr = ((m>>2)*(KPARTITION/*k*/<<2))+(kCnt<<2);
        for(mCnt=0;mCnt<mLeft;mCnt++)
        {
            dst[dstAddr+mCnt] = src[srcAddr+mCnt];
        }
    }
}

