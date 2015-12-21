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
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"
#include "ocl.h"

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

extern "C" {
#include "cblas.h"
}

using namespace std;
using namespace cl;

#define USE_HOST_READ  (CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY)
#define USE_HOST_WRITE (CL_MEM_USE_HOST_PTR|CL_MEM_WRITE_ONLY)
#define USE_HOST_RW    (CL_MEM_USE_HOST_PTR|CL_MEM_READ_WRITE)

extern "C" DLL_PUBLIC
void dsp_cblas_dgemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, 
                 CBLAS_TRANSPOSE transB, int M, int N, int K,
                 double alpha, const double *A, int lda,
                               const double *B, int ldb,
                 double beta,        double *C, int ldc)
{
    if (M==0 || N==0 || ((alpha == 0.0 || K == 0) && beta == 1.0)) return;

    /*-------------------------------------------------------------------------
    * Since our low level gemm kernels expect col major inputs, if row major is
    * required, we compute C' = B' * A' rather than C= A * B.
    *------------------------------------------------------------------------*/
    if (order == CblasRowMajor)
    {
        { const double *  temp = A;      A = B;           B = temp;      }
        { CBLAS_TRANSPOSE temp = transA; transA = transB; transB = temp; }
        { int             temp = M;      M = N;           N = temp;      }
        { int             temp = lda;    lda = ldb;       ldb = temp;    }

        order = CblasColMajor;
    }

    /*-------------------------------------------------------------------------
    * if the matrix will be nil, then simply scale C by beta
    *------------------------------------------------------------------------*/
    if (alpha == 0.0 || K == 0)
    {
        for (int j = 0; j < N; j++)
            for (int i = 0; i < M; i++) C[i+j*ldc] *= beta;

        return;
    }

    /*-------------------------------------------------------------------------
    * Ensure the OpenCL context is active
    *------------------------------------------------------------------------*/
    if (ocl.context == 0) ocl_init(false, NULL);
    Context &ctx = *(ocl.context);
    Kernel *kernel = ocl.K_cblas_dgemm;

    try 
    {
        /*---------------------------------------------------------------------
        * Allocate Buffers: Must alloate beyond MNK for the lda,ldb, and ldc 
        * offsets.  Can probably be smarter here if we copyrectangle, we could 
        * cut a larger lda down to M say for example.
        *--------------------------------------------------------------------*/
        int sizeA = sizeof(double) * 
                   (transA == CblasNoTrans ?  max(lda,M) * K : M * max(lda, K));

        int sizeB = sizeof(double) * 
                   (transB == CblasNoTrans ?  max(ldb,K) * N : K * max(ldb, N));

        int sizeC = sizeof(double) * max(ldc,M) * N;

        Buffer bufA   (ctx, USE_HOST_READ, sizeA, (void*)A);
        Buffer bufB   (ctx, USE_HOST_READ, sizeB, (void*)B);
        Buffer bufC   (ctx, USE_HOST_RW,   sizeC, (void*)C);
        Buffer *bufMsmc = NULL;
        if (dparams.MSMC_BUF_SIZE != 0)
            bufMsmc = new Buffer(ctx, CL_MEM_USE_MSMC_TI|CL_MEM_READ_WRITE,
                                 dparams.MSMC_BUF_SIZE);
        else
            bufMsmc = new Buffer(ctx, CL_MEM_READ_WRITE, 4); // dummy one

        /*----------------------------------------------------------------------
        * Device: Do A*B = C
        *---------------------------------------------------------------------*/
        kernel->setArg(0, order);
        kernel->setArg(1, (int)transA);
        kernel->setArg(2, (int)transB);
        kernel->setArg(3, M);
        kernel->setArg(4, N);
        kernel->setArg(5, K);
        kernel->setArg(6, alpha);
        kernel->setArg(7, bufA);
        kernel->setArg(8, lda);
        kernel->setArg(9, bufB);
        kernel->setArg(10, ldb);
        kernel->setArg(11, beta);
        kernel->setArg(12, bufC);
        kernel->setArg(13, ldc);
        kernel->setArg(14, dparams.NUMAPANELS);
        kernel->setArg(15, dparams.NUMBPANELS);
        kernel->setArg(16, *bufMsmc);
        kernel->setArg(17, __local(dparams.L2_BUF_SIZE));
        kernel->setArg(18, dparams.MSMC_BUF_SIZE);

        std::string kfname;
        kernel->getInfo(CL_KERNEL_FUNCTION_NAME, &kfname);
        int numwgs = (kfname == "K_cblas_dgemm_omp") ? 1 : dparams.NUMCOMPUNITS;

        Event e;
        ocl.queueInOrder->enqueueNDRangeKernel(*kernel, NullRange,
                                               NDRange(numwgs),
                                               NDRange(1), NULL, &e);
        e.wait();
    }
    catch (Error err)
    {
        cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
        exit(-1);
    }  
}
