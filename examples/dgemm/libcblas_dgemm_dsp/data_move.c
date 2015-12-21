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
#include <c6x.h>
#include <stdio.h>
#include <string.h>

// moving B' from L2 to L1
void dataMoveBT(double * restrict dst, double * restrict src, int k,
                int NPARTITION)
{
  int kCnt;
  int srcAddr;
  int dstAddr;
  _nassert((unsigned int) src%8 == 0);
  _nassert((unsigned int) dst%8 == 0);

  srcAddr = 0;
  dstAddr = 0;
  for(kCnt=0; kCnt<k; kCnt++)
  {
	dst[dstAddr]   = src[srcAddr];
	dst[dstAddr+1] = src[srcAddr+1];
	dst[dstAddr+2] = src[srcAddr+2];
	dst[dstAddr+3] = src[srcAddr+3];

	dstAddr += CORE_PROCESS_COLS;
	srcAddr += NPARTITION;
  }
}

// moving B from L2 to L1 (needs transpose)
void dataMoveB(double * restrict dst, double * restrict src, int k)
{
    int kCnt;
    int srcAddr0, srcAddr1, srcAddr2, srcAddr3;
    int dstAddr;

    srcAddr0 = 0;
    srcAddr1 = KPARTITION/*k*/;
    srcAddr2 = 2*KPARTITION/*k*/;
    srcAddr3 = 3*KPARTITION/*k*/;
    dstAddr  = 0;

    _nassert((unsigned int) src%8 == 0);
    _nassert((unsigned int) dst%8 == 0);
    
    //#pragma MUST_ITERATE(1, ,CORE_PROCESS_COLS)
    for(kCnt=0; kCnt<((k>>1)<<1); kCnt++)
    {
        double f0, f1, f2, f3;
        f0=_amemd8_const(&src[srcAddr0]);
        f1=_amemd8_const(&src[srcAddr1]);
        f2=_amemd8_const(&src[srcAddr2]);
        f3=_amemd8_const(&src[srcAddr3]);

        srcAddr0++;
        srcAddr1++;
        srcAddr2++;
        srcAddr3++;

        _amemd8(&dst[dstAddr]) = f0;
        _amemd8(&dst[dstAddr+1]) = f1;
        _amemd8(&dst[dstAddr+2]) = f2;
        _amemd8(&dst[dstAddr+3]) = f3;

        dstAddr += CORE_PROCESS_COLS;
    }
    if((k&1)==1) // odd k; one more value to take care
    {
        double f0, f1, f2, f3;
        f0=_amemd8_const(&src[srcAddr0]);
        f1=_amemd8_const(&src[srcAddr1]);
        f2=_amemd8_const(&src[srcAddr2]);
        f3=_amemd8_const(&src[srcAddr3]);

        _amemd8(&dst[dstAddr]) = f0;
        _amemd8(&dst[dstAddr+1]) = f1;
        _amemd8(&dst[dstAddr+2]) = f2;
        _amemd8(&dst[dstAddr+3]) = f3;
    }
}

// moving data from A to format needed for kernel
void dataMoveA(double * restrict dst, double * restrict src, int m, int k,
               int ld_src)
{
    int kCnt, mCnt;
    int srcAddr, dstAddr;

#if 1
    int mLeft;
    mLeft=m-((m>>2)<<2);
    if(mLeft!=0) mLeft=1;
#endif

    _nassert((unsigned int) src%8 == 0);
    _nassert((unsigned int) dst%8 == 0);

    #pragma MUST_ITERATE(1, ,CORE_PROCESS_ROWS)
    for(kCnt=0;kCnt<k;kCnt++)
    {
        #pragma MUST_ITERATE(1, ,CORE_PROCESS_ROWS)
        for(mCnt=0;mCnt<((m>>2)+mLeft);mCnt++)
        {
            double f0, f1, f2, f3;
            srcAddr = (kCnt*ld_src/*m*/)+(mCnt<<2);
            dstAddr = (mCnt*(KPARTITION/*k*/<<2))+(kCnt<<2);
            // load CORE_PROCESS_ROWS doubles sequentially
            f0 = _amemd8_const(&src[srcAddr]);
            f1 = _amemd8_const(&src[srcAddr+1]);
            f2 = _amemd8_const(&src[srcAddr+2]);
            f3 = _amemd8_const(&src[srcAddr+3]);

            // push CORE_PROCESS_ROWS doubles to desired loc
            _amemd8(&dst[dstAddr]) = f0;
            _amemd8(&dst[dstAddr+1]) = f1;
            _amemd8(&dst[dstAddr+2]) = f2;
            _amemd8(&dst[dstAddr+3]) = f3;
        }
#if 0
        // leftover in m dimension of A
        srcAddr = kCnt*ld_src/*m*/+((m>>2)<<2);
        dstAddr = ((m>>2)*(KPARTITION/*k*/<<2))+(kCnt<<2);
        for(mCnt=0;mCnt<mLeft;mCnt++)
        {
            _amemd8(&dst[dstAddr+mCnt]) = _amemd8_const(&src[srcAddr+mCnt]);
        }
#endif 
    }
}

// moving data from A' to format needed for kernel
void dataMoveAT(double * restrict dst, double * restrict src, int m, int k,
                int ld_src)
{
  int mCnt, kCnt, mLeft;
  int srcAddr, dstAddr;

  mLeft=m-((m>>2)<<2);
  if(mLeft!=0) mLeft=1;

  _nassert((unsigned int) src%8 == 0);
  _nassert((unsigned int) dst%8 == 0);
  for(kCnt=0;kCnt<(k>>1);kCnt++){
	for(mCnt=0;mCnt<(m>>2)+mLeft;mCnt++)
	{
	  double f00, f01, f02, f03;
	  double f10, f11, f12, f13;
	  srcAddr = (mCnt<<2)*ld_src/*k*/+(kCnt<<1);
	  dstAddr = (mCnt*(KPARTITION/*k*/<<2))+(kCnt<<3);

	  // load 2 x CORE_PROCESS_ROWS doubles
	  f00 = src[srcAddr];
	  f10 = src[srcAddr+1];
	  f01 = src[srcAddr+ld_src/*k*/];
	  f11 = src[srcAddr+ld_src/*k*/+1];
	  f02 = src[srcAddr+2*ld_src/*k*/];
	  f12 = src[srcAddr+2*ld_src/*k*/+1];
	  f03 = src[srcAddr+3*ld_src/*k*/];
	  f13 = src[srcAddr+3*ld_src/*k*/+1];

	  // push 2 x CORE_PROCESS_ROWS doubles to desired loc
          dst[dstAddr]   = f00;
          dst[dstAddr+1] = f01;
          dst[dstAddr+2] = f02;
          dst[dstAddr+3] = f03;
          dst[dstAddr+4] = f10;
          dst[dstAddr+5] = f11;
          dst[dstAddr+6] = f12;
          dst[dstAddr+7] = f13;
	}
#if 0
	// leftover in m dimension of A
  	srcAddr = ((m>>2)<<2)*ld_src/*k*/+(kCnt<<1);
   	dstAddr = ((m>>2)*(KPARTITION_D/*k*/<<2))+(kCnt<<3);
    for(mCnt=0;mCnt<mLeft;mCnt++)
    {
      dst[dstAddr+mCnt] = src[srcAddr+mCnt*ld_src/*k*/];
      dst[dstAddr+mCnt+4] = src[srcAddr+mCnt*ld_src/*k*/+1];
    }
#endif
  }

  if((k&1)==1) // one more left in k dimension
  {
	kCnt = k-1;
	for(mCnt=0;mCnt<(m>>2)+mLeft;mCnt++)
	{
	  double f00, f01, f02, f03;
	  int srcAddr = (mCnt<<2)*ld_src/*k*/+kCnt;
	  int dstAddr = (mCnt*(KPARTITION/*k*/<<2))+(kCnt<<2);

	  // load 1 x CORE_PROCESS_ROWS doubles
	  f00 = src[srcAddr];
	  f01 = src[srcAddr+ld_src/*k*/];
	  f02 = src[srcAddr+2*ld_src/*k*/];
	  f03 = src[srcAddr+3*ld_src/*k*/];

	  // push 2 x CORE_PROCESS_COLS doubles to desired loc
	  dst[dstAddr]   = f00;
	  dst[dstAddr+1] = f01;
	  dst[dstAddr+2] = f02;
	  dst[dstAddr+3] = f03;
	}
#if 0
	// leftover in m dimension of A
	// leftover in m dimension of A
  	srcAddr = ((m>>2)<<2)*ld_src/*k*/+(kCnt);
   	dstAddr = ((m>>2)*(KPARTITION_D/*k*/<<2))+(kCnt<<2);
    for(mCnt=0;mCnt<mLeft;mCnt++)
    {
      dst[dstAddr+mCnt] = src[srcAddr+mCnt*ld_src/*k*/];
      dst[dstAddr+mCnt+4] = src[srcAddr+mCnt*ld_src/*k*/+1];
    }
#endif
  }
}

