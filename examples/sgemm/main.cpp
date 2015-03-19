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
#include <ocl_util.h>
#include "kernel.dsp_h"

using namespace std;

static double clock_diff (struct timespec *t1, struct timespec *t2)
{ return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec) / 1e9; }

void PrintMatrix(float *mat, int rows, int cols, int type);
void MatmulHost(const float* A, const float *B, float *C, int M, int N, int P);
void CheckForErrors(const float *Mat, const float *Golden, int M, int N, int P, int type);


/* ======================================================================== */
/*  Global Variables                                                        */
/* ======================================================================== */
#define ROW_MAJOR                   0
#define COL_MAJOR                   1
#define L2_BUF_SIZE                 (768 << 10)
// #define MSMC_SIZE_BYTES             (4 << 20)
#define MSMC_SIZE_BYTES             (2 << 20)
int M                               = 4096;
int N                               = 4096;
int P                               = 4096;
bool check                          = true;
bool random_in                      = false;

/* ======================================================================== */
/*  Function Headers                                                        */
/* ======================================================================== */
void PrintUsageAndExit();
void HandleOptions(int argc, char* argv[]);

/* ======================================================================== */
/*  MAIN                                                                    */
/* ======================================================================== */
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
    printf("C[%d,%d] = A[%d,%d] * B[%d,%d]\n\n", M,P,M,N,N,P);

    double dsp_elapsed = 0;
    double total_GFLOP = 2.0*M*N*P*1.0e-9;

    try 
    {
       Context context(CL_DEVICE_TYPE_ACCELERATOR);
       std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
       CommandQueue Q(context, devices[0]);

       /*---------------------------------------------------------------------
       * Compile the Kernel Source for the devices
       *--------------------------------------------------------------------*/
       Program::Binaries binary(1, make_pair(kernel_dsp_bin,
                                             sizeof(kernel_dsp_bin)));
       Program             program = Program(context, devices, binary);
       program.build(devices);

       /* ---------------------------------------------------------------- */
       /*  Allocate Buffers:                                               */
       /* ---------------------------------------------------------------- */
       Buffer bufA   (context, CL_MEM_READ_ONLY,  M*N*sizeof(float));
       Buffer bufB   (context, CL_MEM_READ_ONLY,  N*P*sizeof(float));
       Buffer bufC   (context, CL_MEM_READ_WRITE, M*P*sizeof(float));
       Buffer bufMsmc(context, CL_MEM_READ_WRITE|CL_MEM_USE_MSMC_TI, MSMC_SIZE_BYTES);

       /* ---------------------------------------------------------------- */
       /*  Initialized input arrays with random test data.                 */
       /* ---------------------------------------------------------------- */
       float *A = (float*) Q.enqueueMapBuffer(bufA, CL_TRUE, CL_MAP_WRITE, 0, M*N*sizeof(float));
       float *B = (float*) Q.enqueueMapBuffer(bufB, CL_TRUE, CL_MAP_WRITE, 0, N*P*sizeof(float));
       float *C = (float*) Q.enqueueMapBuffer(bufC, CL_TRUE, CL_MAP_WRITE, 0, M*P*sizeof(float));

       srand(time(NULL));
       for (int i = 0; i < M*N; ++i) A[i] = random_in? (double)(rand() % 5 + 1): i & 7; 
       for (int i = 0; i < N*P; ++i) B[i] = random_in? (double)(rand() % 5 + 1): i & 7; 
       for (int i = 0; i < M*P; ++i) C[i] = 0;

       PrintMatrix(A,M,N,COL_MAJOR);
       PrintMatrix(B,N,P,COL_MAJOR);

       Q.enqueueUnmapMemObject(bufA, A);
       Q.enqueueUnmapMemObject(bufB, B);
       Q.enqueueUnmapMemObject(bufC, C);
       Q.finish();

       /*----------------------------------------------------------------------
       * Device: Do A*B = C
       *---------------------------------------------------------------------*/
       Kernel kernel (program, "ocl_matmpy");
       KernelFunctor matmul = kernel.bind(Q, NDRange(8), NDRange(1));

       struct timespec t0,t1;
       clock_gettime(CLOCK_MONOTONIC, &t0);

       matmul(bufA, bufB, bufC, M, N, P, bufMsmc, __local(L2_BUF_SIZE), MSMC_SIZE_BYTES).wait();

       clock_gettime(CLOCK_MONOTONIC, &t1);
       dsp_elapsed = clock_diff (&t0, &t1);

       double gflops = total_GFLOP/dsp_elapsed;
       printf("DSP: %.3f Gflops (%.6f s) \n", gflops, dsp_elapsed);

       /*----------------------------------------------------------------------
       * If checking results against a host matmul.  
       * This can be time consuming for large sizes.
       *---------------------------------------------------------------------*/
       C = (float*) Q.enqueueMapBuffer(bufC, CL_TRUE, CL_MAP_READ, 0, M*P*sizeof(float));

       if (check)
       {
           A = (float*) Q.enqueueMapBuffer(bufA, CL_TRUE, CL_MAP_READ, 0, M*N*sizeof(float));
           B = (float*) Q.enqueueMapBuffer(bufB, CL_TRUE, CL_MAP_READ, 0, N*P*sizeof(float));
	   float *gold = (float*)malloc(M*P*sizeof(float));
           memset(gold, 0, M*P*sizeof(float));
           clock_gettime(CLOCK_MONOTONIC, &t0);

           MatmulHost(A, B, gold, M, N, P);

           clock_gettime(CLOCK_MONOTONIC, &t1);
           dsp_elapsed = clock_diff (&t0, &t1);

           double gflops = total_GFLOP/dsp_elapsed;
           printf("CPU: %.3f Gflops (%.6f s) \n\n", gflops, dsp_elapsed);
           Q.enqueueUnmapMemObject(bufA, A);
           Q.enqueueUnmapMemObject(bufB, B);
	   PrintMatrix(gold,M,P,ROW_MAJOR); 
	   CheckForErrors(C, gold, M, N, P, COL_MAJOR);
       }

       PrintMatrix(C,M,P,COL_MAJOR); 
       Q.enqueueUnmapMemObject(bufC, C); Q.finish();
   }
   catch (Error err)
   {
       cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
       exit(-1);
   }

}

/******************************************************************************
* Supporting Functions
******************************************************************************/

void PrintUsageAndExit()
{
    cout << "Matrix A[M,N] x B [N,P] = C [M,P]" << endl
         << "Default value of M,N,P is " << M << endl 
         << "Usage: matmul [options] " << endl
         << "Options: " << endl
         << "-M arg : Number of rows for array A" << endl
         << "-N arg : Number of cols for array A, rows for array B" << endl
         << "-P arg : Number of cols for array B" << endl
         << "-c     : Check results" << endl
         << "-r     : Generate random inputs" << endl
         << "-h     : Show help message" 
         << endl;
    exit(0);
}

void HandleOptions(int argc, char* argv[])
{
    int c;

    if (argc == 1) return;
    
    while ((c = getopt (argc, argv, "M:N:P:hcr")) != -1)
        switch(c)
        {
            case 'M': M = abs(atoi(optarg)); break;
            case 'N': N = abs(atoi(optarg)); break;
            case 'P': P = abs(atoi(optarg)); break;
            case 'h': PrintUsageAndExit();   break;
            case 'c': check = true;          break;
            case 'r': random_in = true;      break;
            default:  PrintUsageAndExit();
        }
}

void PrintMatrix(float *mat, int rows, int cols, int type)
{
    if (rows > 64) return;
    if (cols > 16) return;

    int index;
    for (int i=0; i<rows; i++)
    {
        for (int j=0; j<cols; j++)
        {
            if (type == ROW_MAJOR) index = i*cols + j;
            else                   index = j*rows + i;

            cout << setprecision(9) << setw(10)
                 << mat[index] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

#define EPISILON 0.00001
void CheckForErrors(const float *Mat, const float *Golden, int M, int N, int P, int type)
{
    int       num_errors = 0, i, j;
    const int print_nerrors = 13;
    int       index;

    for (i=0; i<M; i++)
        for (j=0; j<P; j++)
        {
            if (type == ROW_MAJOR) index = i*P + j;
            else                   index = j*M + i;

            float delta = Golden[i*P + j] - Mat[index];

            if (delta < -EPISILON || delta > EPISILON)
                if ((num_errors += 1) < print_nerrors)
                    printf("Error [%d,%d]: %f <==> %f\n", i, j,
                           Golden[i*P + j], Mat[index]);
        }

    if (num_errors > 0)
         cout << "FAIL with " << num_errors << " errors!\n";
    else cout << "PASS!" << endl;
}

/*-----------------------------------------------------------------------------
* If ATLAS sgemm is available, use it for the correctness test as it is faster
*----------------------------------------------------------------------------*/
#ifdef ATLAS
extern "C" 
{ 
#include "cblas.h" 
}

void MatmulHost(const float*A, const float *B, float *C, int M, int N, int P)
{
    float alpha = 1.0, beta = 0.0;
    cblas_sgemm(CblasRowMajor, CblasTrans, CblasTrans,
                 M, P, N, alpha,
                 A, /* lda = */ M,
                 B, /* ldb = */ N,
                 beta,
                 C, /* ldc = */ P
                 );
}
#else
void MatmulHost(const float*A, const float *B, float *C, int M, int N, int P)
{
    #pragma omp parallel for collapse(2) firstprivate(M,N,P) shared(A,B,C)
    for (int i = 0; i < M; ++i) // M rows
        for (int j = 0; j < P; ++j) // P columns
        {
            float sum = 0;
            for (int k = 0; k < N; ++k)
            {
                sum +=   A[i + k*M] // Column Major
                       * B[k + j*N]; // Column Major
            }
            C[i*P + j] = sum; // Store gold in Row Major
        }
}
#endif

