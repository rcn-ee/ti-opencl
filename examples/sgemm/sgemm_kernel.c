/**
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 *  @file   sgemKernel.c
 *  @brief  This file contains functions single precision
 *          matrix multiplication implementation of sizes
 *          8xk by kx4 producing 8x4 output. This is C66 intrinsic
 *          optimized code
 *
 */

#include "c6x.h"
#include "sgemm_kernel.h"

void sgemm_kernel(const float *pA, 
                  const float *pB, 
                  float *pC, 
                  const float a, 
                  const int k, 
                  const int stepC
                  )
{
    __float2_t sum0, sum1, sum2, sum3, sum4, sum5, sum6, sum7;
    __float2_t sum8, sum9, suma, sumb, sumc, sumd, sume, sumf;
    __float2_t * restrict ptrB     = (__float2_t *) pB;
    // The following twin addresses are defined to be volatile so
    // compiler does not optimize them out; This makes the compiler
    // use load instructions for both registers; otherwise
    // the compiler optimizes out one address thereby reducing
    // the usage of D units at the expense of an LS unit; The result
    // was in fact increase of cycle count
    volatile __float2_t * restrict ptrA     = (__float2_t *) pA;
    volatile __float2_t * restrict ptrATwin = (__float2_t *) pA;
    __float2_t * restrict ptrC;
    __float2_t regA2, regC, regS;
    int_least16_t index;
    
    // zero out accumulators
    sum0 = 0.0;
    sum1 = 0.0;
    sum2 = 0.0;
    sum3 = 0.0;
    sum4 = 0.0;
    sum5 = 0.0;
    sum6 = 0.0;
    sum7 = 0.0;
    sum8 = 0.0;
    sum9 = 0.0;
    suma = 0.0;
    sumb = 0.0;
    sumc = 0.0;
    sumd = 0.0;
    sume = 0.0;
    sumf = 0.0;

    #pragma MUST_ITERATE(2,,2)
    for (index = 0; index < k; index++)
    { // loop over k;
        // each iteration performs rank one update of 8x1 by 1x4
        // matrices of x1 and x2 respectively; result is
        // accumulated over 8x4 matrix
        __float2_t b01, b23, b45, b67, a01, a23, a01Twin, a23Twin;
        __x128_t   reg128;

        a01 = *ptrA++;
        a23 = *ptrA++;
        // compiler is using LS units to create a twin register
        // D units are available; force a load (D unit) to use twin register;
        a01Twin = *ptrATwin++;
        a23Twin = *ptrATwin++;

        b01 = *ptrB++;
        b23 = *ptrB++;
        b45 = *ptrB++;
        b67 = *ptrB++;

        reg128 = _cmpysp(b01, a01);
        // accumulate b[0]*a[1] and -b[0]*a[0]
        sum0 = _daddsp(sum0, _lof2_128(reg128));
        // accumulate b[1]*a[0] and b[1]*a[1]
        sum1 = _daddsp(sum1, _hif2_128(reg128));

        reg128 = _cmpysp(b23, a01);
        // accumulate b[2]*a[1] and -b[2]*a[0]
        sum2 = _daddsp(sum2, _lof2_128(reg128));
        // accumulate b[3]*a[0] and b[3]*a[1]
        sum3 = _daddsp(sum3, _hif2_128(reg128));

        reg128 = _cmpysp(b45, a01Twin);
        // accumulate b[4]*a[1] and -b[4]*a[0]
        sum4 = _daddsp(sum4, _lof2_128(reg128));
        // accumulate b[5]*a[0] and b[5]*a[1]
        sum5 = _daddsp(sum5, _hif2_128(reg128));

        reg128 = _cmpysp(b67, a01Twin);
        // accumulate b[6]*a[1] and -b[6]*a[0]
        sum6 = _daddsp(sum6, _lof2_128(reg128));
        // accumulate b[7]*a[0] and b[7]*a[1]
        sum7 = _daddsp(sum7, _hif2_128(reg128));

        reg128 = _cmpysp(b01, a23);
        // accumulate b[0]*a[3] and -b[0]*a[2]
        sum8 = _daddsp(sum8, _lof2_128(reg128));
        // accumulate b[1]*a[2] and b[1]*a[3]
        sum9 = _daddsp(sum9, _hif2_128(reg128));

        reg128 = _cmpysp(b23, a23);
        // accumulate b[2]*a[3] and -b[2]*a[2]
        suma = _daddsp(suma, _lof2_128(reg128));
        // accumulate b[3]*a[2] and b[3]*a[3]
        sumb = _daddsp(sumb, _hif2_128(reg128));

#if __TI_COMPILER_VERSION__ != 7006000
        reg128 = _cmpysp(b67, a23Twin);
        // accumulate b[6]*a[3] and -b[6]*a[2]
        sume = _daddsp(sume, _lof2_128(reg128));
        // accumulate b[7]*a[2] and b[7]*a[3]
        sumf = _daddsp(sumf, _hif2_128(reg128));

        reg128 = _cmpysp(b45, a23Twin);
        // accumulate b[4]*a[3] and -b[4]*a[2]
        sumc = _daddsp(sumc, _lof2_128(reg128));
        // accumulate b[5]*a[2] and b[5]*a[3]
        sumd = _daddsp(sumd, _hif2_128(reg128));
#else
        reg128 = _cmpysp(b45, a23Twin);
        // accumulate b[4]*a[3] and -b[4]*a[2]
        sumc = _daddsp(sumc, _lof2_128(reg128));
        // accumulate b[5]*a[2] and b[5]*a[3]
        sumd = _daddsp(sumd, _hif2_128(reg128));

        reg128 = _cmpysp(b67, a23Twin);
        // accumulate b[6]*a[3] and -b[6]*a[2]
        sume = _daddsp(sume, _lof2_128(reg128));
        // accumulate b[7]*a[2] and b[7]*a[3]
        sumf = _daddsp(sumf, _hif2_128(reg128));
#endif

    }

    regA2 = _ftof2(a, a);
    index = (stepC >> 1)-2; // (M)/2 -2 elements
    // The subtraction of 2 from the index specifies the 2 
    // __float_2_t writes done before jumping to the start
    // of the next column
    
    ptrC = (__float2_t *) pC;
    // update c[0,0] and c[1,0]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sum0),-_hif2(sum0));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);
    // update c[2,0] and c[3,0]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sum8),-_hif2(sum8));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);

    // move to start of next column
    ptrC += index;
    // update c[0,1] and c[1,1]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sum1), regC);
    // update c[2,1] and c[3,1]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sum9), regC);

    // move to next column
    ptrC += index;
    // update c[0,2] and c[1,2]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sum2),-_hif2(sum2));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);
    // update c[2,2] and c[3,2]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(suma),-_hif2(suma));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);

    // move to next column
    ptrC += index;
    // update c[0,3] and c[1,3]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sum3), regC);
    // update c[2,3] and c[3,3]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sumb), regC);

    // move to next column
    ptrC += index;
    // update c[0,4] and c[1,4]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sum4),-_hif2(sum4));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);
    // update c[2,4] and c[3,4]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sumc),-_hif2(sumc));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);

    // move to next column
    ptrC += index;
    // update c[0,5] and c[1,5]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sum5), regC);
    // update c[2,5] and c[3,5]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sumd), regC);

    // move to next column
    ptrC += index;
    // update c[0,6] and c[1,6]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sum6),-_hif2(sum6));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);
    // update c[2,6] and c[3,6]
    regC = _mem8_f2_const(ptrC);
    regS = _ftof2(_lof2(sume),-_hif2(sume));
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, regS), regC);

    // move to next column
    ptrC += index;
    // update c[0,7] and c[1,7]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sum7), regC);
    // update c[2,7] and c[3,7]
    regC = _mem8_f2_const(ptrC);
    _mem8_f2(ptrC++) = _daddsp(_dmpysp(regA2, sumf), regC);

    return;
} // sgemmKernel()


