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
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <CL/cl_ext.h>

extern "C"
{
#include "cblas.h"
}
#include "cblas_dgemm_dsp.h"

using namespace std;

/*-----------------------------------------------------------------------------
* Timing Setup
*----------------------------------------------------------------------------*/
struct timespec t0,t1;
#define tick()  clock_gettime(CLOCK_MONOTONIC, &t0);
#define tock() (clock_gettime(CLOCK_MONOTONIC, &t1), \
                        t1.tv_sec - t0.tv_sec + (t1.tv_nsec - t0.tv_nsec) / 1e9)

/*-----------------------------------------------------------------------------
* Global Variables
*----------------------------------------------------------------------------*/
int    M               = 2048;
int    N               = 2048;
int    K               = 2048;
double alpha           = 1.0; 
double beta            = 0.0;
CBLAS_ORDER     order  = CblasColMajor;
CBLAS_TRANSPOSE transA = CblasNoTrans;
CBLAS_TRANSPOSE transB = CblasNoTrans;
bool   calc_check      = false;

/*-----------------------------------------------------------------------------
* Prototypes
*----------------------------------------------------------------------------*/
void PrintUsageAndExit();
void HandleOptions (int argc, char* argv[]);
int  Check         (const double *C1, const double *C2, int M, int N);

/*-----------------------------------------------------------------------------
* Operation printing helpers
*----------------------------------------------------------------------------*/
const char *op(CBLAS_TRANSPOSE t) { return t == CblasTrans ? "'" : ""; }
const char *maj(CBLAS_ORDER o)    { return (o == CblasColMajor) ? "cMaj " : "rMaj "; }

/*-----------------------------------------------------------------------------
* MAIN
*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    /*-------------------------------------------------------------------------
     * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
     *------------------------------------------------------------------------*/
    signal(SIGABRT, exit);
    signal(SIGTERM, exit);

    /* ---------------------------------------------------------------- */
    /*  Handle command line options to get M,N,P                        */
    /* ---------------------------------------------------------------- */
    HandleOptions(argc,argv);

    double total_GFLOP = 2.0*M*N*K*1e-9;
    double secs;

    /*-------------------------------------------------------------------------
    * Allocate space for the matrices.  The matrices that will be passed to 
    * the DSP are allocated using device memory.  The Ccpu array is not passed
    * to the dsp and so can use system memory.
    *------------------------------------------------------------------------*/
    double *A    = (double*) __malloc_ddr(M*K*sizeof(double));
    double *B    = (double*) __malloc_ddr(K*N*sizeof(double));
    double *C    = (double*) __malloc_ddr(M*N*sizeof(double));
    double *Ccpu = (double*) malloc      (M*N*sizeof(double));

    if (!A || !B || !C || !Ccpu)
    {
        cout << "Could not allocate enough space for the arrays!" << endl;
        exit(-1);
    }

    /*-------------------------------------------------------------------------
    * Initialize matrices and print if small enough.
    *------------------------------------------------------------------------*/
    cout << "Generating random data ... " << flush;
    srand(time(NULL));
    for (int i = 0; i < M*K; ++i) A[i] = (double)(rand() % 5 + 1);
    for (int i = 0; i < K*N; ++i) B[i] = (double)(rand() % 5 + 1);
    for (int i = 0; i < M*N; ++i) Ccpu[i] = C[i] = 0;
    cout << "Done. Starting Dgemm. " << endl;

    cout << maj(order) << "C["<<M<<","<<N<<"] = " 
                       << "A"<<op(transA)<<"["<<M<<","<<K<<"] * "
                       << "B"<<op(transB)<<"["<<K<<","<<N<<"]: "
                       << endl
                       << flush ;

    int lda = ((order == CblasColMajor && transA == CblasNoTrans) ||
               (order == CblasRowMajor && transA == CblasTrans)) ? M : K;

    int ldb = ((order == CblasColMajor && transB == CblasNoTrans) ||
               (order == CblasRowMajor && transB == CblasTrans)) ? K : N;

    int ldc = (order == CblasColMajor) ? M : N;

    /*-------------------------------------------------------------------------
    * Calling ocl_init is not required, but it does prime the 1 time OpenCL 
    * context creation, so that the first dsp cblas_dgemm call timing is not 
    * skewed by this setup cost.
    *------------------------------------------------------------------------*/
    int NUMCOMPUNITS;
    ocl_init(calc_check, &NUMCOMPUNITS);

    /*-------------------------------------------------------------------------
    * Time DSP dgemm
    *------------------------------------------------------------------------*/
    tick();
    dsp_cblas_dgemm(order,transA,transB,M,N,K,alpha,A,lda,B,ldb,beta,C,ldc);
    secs = tock();
    printf("%4d DSPs: %.3f Gflops (%.6fs)\n",
           NUMCOMPUNITS, total_GFLOP/secs, secs);
    fflush(stdout);

    /*-------------------------------------------------------------------------
    * Time CPU dgemm
    *------------------------------------------------------------------------*/
    tick();
    cblas_dgemm(order,transA,transB,M,N,K,alpha,A,lda,B,ldb,beta,Ccpu,ldc);
    secs = tock();
    printf("   1 CPU : %.3f Gflops (%.6fs) with ATLAS library\n",
           total_GFLOP/secs, secs);
    fflush(stdout);

    /*-------------------------------------------------------------------------
    * Verify Results
    *------------------------------------------------------------------------*/
    int retval = Check(C, Ccpu, M, N);

    /*-------------------------------------------------------------------------
    * Calling ocl_free to clean up OpenCL resources and temporary files.
    *------------------------------------------------------------------------*/
    ocl_free();

    return retval;
}

/*-----------------------------------------------------------------------------
* Check
*----------------------------------------------------------------------------*/
int Check(const double *C1, const double *C2, int M, int N)
{
    const int EPISILON = 0.00001;
    const int NERRORS  = 13;
    int       num_errors = 0, i;

    for (i=0; i<M*N; i++)
    {
        double delta = fabs(C1[i] - C2[i]);

        if (delta > EPISILON)
            if ((num_errors += 1) < NERRORS)
                printf("Error [elem:%d]: %f <==> %f\n", i, C1[i], C2[i]);
    }

    if (num_errors > 0)
    {
         cout << "FAIL with " << num_errors << " errors!" << endl;
         return -1;
    }
    else 
    {
        cout << "PASS!" << endl;
        return 0;
    }
}

/*-----------------------------------------------------------------------------
* void PrintUsageAndExit()
*----------------------------------------------------------------------------*/
void PrintUsageAndExit()
{
   cout << "C[M,N] = A[M,K] * B[K,N]" << endl
        << "Default value of M,N,K is " << M << endl 
        << "Usage: dgemm [options] " << endl
        << "Options: " << endl
        << "-M arg : Number of rows for array C" << endl
        << "-K arg : Number of cols for array A, rows for array B" << endl
        << "-N arg : Number of cols for array C" << endl
        << "-a     : Transpose A matrix" << endl
        << "-b     : Transpose B matrix" << endl
        << "-oc    : Compute as Column Major" << endl
        << "-or    : Compute as Row Major" << endl
        << "-h     : Show help message" << endl;
    exit(0);
}

/*-----------------------------------------------------------------------------
* void HandleOptions(int argc, char* argv[])
*----------------------------------------------------------------------------*/
void HandleOptions(int argc, char* argv[])
{
    int c;

    if (argc == 1) return;

    while ((c = getopt (argc, argv, "o:M:K:N:habx")) != -1)
        switch(c)
        {
            case 'o': *optarg == 'r' ? order = CblasRowMajor : CblasColMajor;
                      break;
            case 'M': M = abs(atoi(optarg));  break;
            case 'K': K = abs(atoi(optarg));  break;
            case 'N': N = abs(atoi(optarg));  break;
            case 'h': PrintUsageAndExit();    break;
            case 'a': transA = CblasTrans;    break;
            case 'b': transB = CblasTrans;    break;
            case 'x': calc_check = true;      break;
            default:  PrintUsageAndExit();
        }
}
