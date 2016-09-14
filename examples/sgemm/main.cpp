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
#include <math.h>
#include <signal.h>
#include <ocl_util.h>
#include "kernel.dsp_h"

#ifdef _TI_RTOS
#include <ti/sysbios/posix/_time.h>
#include "../rtos_main.c"
#endif

extern "C" {
#include "cblas.h"
}

using namespace std;
using namespace cl;

static double clock_diff (struct timespec *t1, struct timespec *t2)
{ return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec) / 1e9; }

void PrintMatrix(float *mat, int rows, int cols, enum CBLAS_ORDER mem_order);
void MatmulHost_ATLAS(enum CBLAS_ORDER mem_order,
                const float* A, const float *B, float *C, int M, int N, int K,
                float alpha, float beta);
void MatmulHost_loopnest(enum CBLAS_ORDER mem_order,
                const float* A, const float *B, float *C, int M, int N, int K,
                float alpha, float beta);
int  CheckForErrors(const float *Mat, const float *Golden, int M, int N, int K,
                    enum CBLAS_ORDER mem_order);

/* ======================================================================== */
/*  Global Variables                                                        */
/* ======================================================================== */
float alpha                         = 1.0f;
float beta                          = 0.0f;
enum CBLAS_ORDER order              = CblasColMajor;
#ifndef _TI_RTOS
bool check                          = true;
#else
bool check                          = false;
#endif
bool random_in                      = false;
bool calc_check                     = false;

int M                               = 0;
int N                               = 0;
int K                               = 0;
int L2_BUF_SIZE                     = 0;
int MSMC_BUF_SIZE                   = 0;
int NUMAPANELS                      = 0;
int NUMBPANELS                      = 0;
int NUMCOMPUNITS                    = 0;

/* ======================================================================== */
/*  Function Headers                                                        */
/* ======================================================================== */
void PrintUsageAndExit();
void HandleOptions(int argc, char* argv[]);
bool SetSgemmParams(Device& device);

/* ======================================================================== */
/*  MAIN                                                                    */
/* ======================================================================== */
#ifdef _TI_RTOS
void ocl_main(UArg arg0, UArg arg1)
{
   int    argc = (int)     arg0;
   char **argv = (char **) arg1;
#else
#define RETURN(x) return x
int main(int argc, char *argv[])
{
#endif
   /*-------------------------------------------------------------------------
   * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
   *------------------------------------------------------------------------*/
   signal(SIGABRT, exit);
   signal(SIGTERM, exit);

   int errs = 0;

    /* ---------------------------------------------------------------- */
    /*  Handle command line options to get M,N,K                        */
    /* ---------------------------------------------------------------- */
    HandleOptions(argc,argv);

    try 
    {
       Context context(CL_DEVICE_TYPE_ACCELERATOR);
       std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
       CommandQueue Q(context, devices[0]);

       /*---------------------------------------------------------------------
       * Determine platform, set sgemm blocking/tiling parameters
       *--------------------------------------------------------------------*/
       SetSgemmParams(devices[0]);
       if (NUMCOMPUNITS == 0)  RETURN(-1);

       int VALRANGE = 17;
       if (random_in) 
       {
           srand(time(NULL));
           alpha = (float) (rand() % VALRANGE + 1);
           beta  = (float) (rand() % VALRANGE + 1);
       }

       printf("C[%d,%d] = alpha * A[%d,%d] * B[%d,%d] + beta * C[%d,%d], %s\n",
              M,N,M,K,K,N, M,N,
              (order == CblasRowMajor ? "use row-major storage"
                                      : "use col-major storage"));
       printf("alpha=%f, beta=%f\n\n", alpha, beta);

       double dsp_elapsed = 0;
       double total_GFLOP = 2.0*M*N*K*1.0e-9;

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
       Buffer bufA   (context, CL_MEM_READ_ONLY,  M*K*sizeof(float));
       Buffer bufB   (context, CL_MEM_READ_ONLY,  K*N*sizeof(float));
       Buffer bufC   (context, CL_MEM_READ_WRITE, M*N*sizeof(float));
       Buffer *bufMsmc = NULL;
       if (MSMC_BUF_SIZE != 0)
           bufMsmc = new Buffer(context, CL_MEM_READ_WRITE|CL_MEM_USE_MSMC_TI,
                                MSMC_BUF_SIZE);
       else
           bufMsmc = new Buffer(context, CL_MEM_READ_WRITE, 4); // dummy one

       /* ---------------------------------------------------------------- */
       /*  Initialized input arrays with random test data.                 */
       /* ---------------------------------------------------------------- */
       float *A = (float*) Q.enqueueMapBuffer(bufA, CL_TRUE, CL_MAP_WRITE, 0,
                                              M*K*sizeof(float));
       float *B = (float*) Q.enqueueMapBuffer(bufB, CL_TRUE, CL_MAP_WRITE, 0,
                                              K*N*sizeof(float));
       float *C = (float*) Q.enqueueMapBuffer(bufC, CL_TRUE, CL_MAP_WRITE, 0,
                                              M*N*sizeof(float));
       float *gold;

       cout << "Generating Input Data ..." << flush;
       for (int i = 0; i < M*K; ++i)
           A[i] = random_in ? (float)(rand() % VALRANGE + 1) : 1 + (i & 7); 
       for (int i = 0; i < K*N; ++i)
           B[i] = random_in ? (float)(rand() % VALRANGE + 1) : 1 + (i & 11); 
       for (int i = 0; i < M*N; ++i)
           C[i] = random_in ? (float)(rand() % VALRANGE + 1) : 1 + (i & 5);
       cout << "Complete" << endl;

       if (check)
       {
#ifndef _TI_RTOS
           if ((gold = (float*) malloc(M*N*sizeof(float))) == NULL)
#else
           if ((gold = (float*) __malloc_ddr(M*N*sizeof(float))) == NULL)
#endif
           {
               printf("Unable to allocate memory to verify results\n");
               exit(-1);
           }
           memcpy(gold, C, M*N*sizeof(float));
       }

       PrintMatrix(A,M,K,order);
       PrintMatrix(B,K,N,order);
       if (random_in)  PrintMatrix(C,M,N,order);

       Q.enqueueUnmapMemObject(bufA, A);
       Q.enqueueUnmapMemObject(bufB, B);
       Q.enqueueUnmapMemObject(bufC, C);
       Q.finish();

       /*----------------------------------------------------------------------
       * Device: Do A*B = C
       *---------------------------------------------------------------------*/
       Kernel kernel (program, "K_ocl_sgemm_dsp");
       KernelFunctor matmul = kernel.bind(Q, NDRange(NUMCOMPUNITS), NDRange(1));

       struct timespec t0,t1;
       clock_gettime(CLOCK_MONOTONIC, &t0);

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
       * ld_Transpose(a)_col = lda_row = k, 
       * ld_Transpose(b)_col = ldb_row = n, 
       * ld_Transpose(c)_col = ldc_row = n, 
       *---------------------------------------------------------------------*/
       if (order == CblasRowMajor)
           matmul(N, M, K, alpha, bufB, N, bufA, K, beta, bufC, N,
                  NUMAPANELS, NUMBPANELS,
                  __local(L2_BUF_SIZE), *bufMsmc).wait();
       else
           matmul(M, N, K, alpha, bufA, M, bufB, K, beta, bufC, M,
                  NUMAPANELS, NUMBPANELS,
                  __local(L2_BUF_SIZE), *bufMsmc).wait();

       clock_gettime(CLOCK_MONOTONIC, &t1);
       dsp_elapsed = clock_diff (&t0, &t1);

       double gflops = total_GFLOP/dsp_elapsed;
       printf("%4d DSPs: %.3f Gflops (%.6f s) \n",
              NUMCOMPUNITS, gflops, dsp_elapsed);

       if (bufMsmc != NULL)  delete bufMsmc;

       /*----------------------------------------------------------------------
       * If checking results against a host matmul.  
       * This can be time consuming for large sizes.
       *---------------------------------------------------------------------*/
       C = (float*) Q.enqueueMapBuffer(bufC, CL_TRUE, CL_MAP_READ, 0,
                                       M*N*sizeof(float));

       if (check)
       {
           A = (float*) Q.enqueueMapBuffer(bufA, CL_TRUE, CL_MAP_READ, 0,
                                           M*K*sizeof(float));
           B = (float*) Q.enqueueMapBuffer(bufB, CL_TRUE, CL_MAP_READ, 0,
                                           K*N*sizeof(float));
           clock_gettime(CLOCK_MONOTONIC, &t0);

           MatmulHost_ATLAS(order, A, B, gold, M, N, K, alpha, beta);

           clock_gettime(CLOCK_MONOTONIC, &t1);
           dsp_elapsed = clock_diff (&t0, &t1);

           double gflops = total_GFLOP/dsp_elapsed;
           printf("   1 CPU : %.3f Gflops (%.6f s) with ATLAS library\n",
                  gflops, dsp_elapsed);
           Q.enqueueUnmapMemObject(bufA, A);
           Q.enqueueUnmapMemObject(bufB, B);
	   PrintMatrix(gold,M,N,order); 
	   errs = CheckForErrors(C, gold, M, N, K, order);
#ifndef _TI_RTOS
           free(gold);
#else
           __free_ddr(gold);
#endif
       }

       PrintMatrix(C,M,N,order); 
       Q.enqueueUnmapMemObject(bufC, C); Q.finish();
   }
   catch (Error err)
   {
       cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
            << ocl_decode_error(err.err()) << ")" << endl;
       exit(-1);
   }

   RETURN(errs);
}

/******************************************************************************
* Supporting Functions
******************************************************************************/
void PrintUsageAndExit()
{
    cout << "Matrix C[M,N] = A[M,K] * B [K,N]" << endl
         << "Default value of M,N,K is " << M << endl 
         << "Usage: sgemm [options] " << endl
         << "Options: " << endl
         << "-M arg : Number of rows for array C and A" << endl
         << "-K arg : Number of cols for array A, rows for array B" << endl
         << "-N arg : Number of cols for array C and B" << endl
         << "-d     : Do not check results against host computation" << endl
         << "-r     : Generate random inputs" << endl
         << "-or    : Use Row-Major storage (default is Col-Major)" << endl
         << "-h     : Show help message" 
         << endl;
    exit(0);
}

void HandleOptions(int argc, char* argv[])
{
    int c;

    if (argc == 1) return;
    
    while ((c = getopt (argc, argv, "o:M:K:N:hdrx")) != -1)
        switch(c)
        {
            case 'o': order = (*optarg == 'r') ? CblasRowMajor
                                               : CblasColMajor; break;
            case 'M': M = abs(atoi(optarg)); break;
            case 'K': K = abs(atoi(optarg)); break;
            case 'N': N = abs(atoi(optarg)); break;
            case 'h': PrintUsageAndExit();   break;
            case 'd': check = false;         break;
            case 'r': random_in = true;      break;
            case 'x': calc_check = true;     break;
            default:  PrintUsageAndExit();
        }
}

void PrintMatrix(float *mat, int rows, int cols, enum CBLAS_ORDER mem_order)
{
    if (rows > 64) return;
    if (cols > 16) return;

    int index;
    for (int i=0; i<rows; i++)
    {
        for (int j=0; j<cols; j++)
        {
            if (mem_order == CblasRowMajor) index = i*cols + j;
            else                            index = j*rows + i;

            cout << setprecision(9) << setw(10)
                 << mat[index] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

#define EPISILON 0.00001
int CheckForErrors(const float *Mat, const float *Golden, int M, int N, int K,
                    enum CBLAS_ORDER mem_order)
{
    int       num_errors = 0, i, j;
    const int print_nerrors = 13;
    int       index;

    for (i=0; i<M; i++)
        for (j=0; j<N; j++)
        {
            if (mem_order == CblasRowMajor) index = i*N + j;
            else                            index = j*M + i;

            float delta = Golden[index] - Mat[index];

            if (delta < -EPISILON || delta > EPISILON)
                if ((num_errors += 1) < print_nerrors)
                    printf("Error [%d,%d]: %f <==> %f\n", i, j,
                           Golden[index], Mat[index]);
        }

    if (num_errors > 0)
         cout << "FAIL with " << num_errors << " errors!" << endl;
    else cout << "PASS!" << endl;
    return num_errors;
}

void MatmulHost_ATLAS(enum CBLAS_ORDER mem_order,
                const float*A, const float *B, float *C, int M, int N, int K,
                float alpha, float beta)
{
#ifndef _TI_RTOS
    if (mem_order == CblasRowMajor)
    {
        cblas_sgemm(mem_order, CblasNoTrans, CblasNoTrans,
                    M, N, K, alpha,
                    A, /* lda = */ K,
                    B, /* ldb = */ N,
                    beta,
                    C, /* ldc = */ N
                   );
    } else {
        cblas_sgemm(mem_order, CblasNoTrans, CblasNoTrans,
                    M, N, K, alpha,
                    A, /* lda = */ M,
                    B, /* ldb = */ K,
                    beta,
                    C, /* ldc = */ M
                   );
    }
#endif
}

static ulong roundDownPower2(ulong value)
{ return (value == 0) ? 0 :  1 << ilogb(value); }

/*-----------------------------------------------------------------------------
* Check platform name, set sgemm blocking/tiling parameters accordingly
*----------------------------------------------------------------------------*/
bool SetSgemmParams(Device& device)
{
   int APanelSz        = 8  << 10;
   int BPanelSz        = 16 << 10;
   cl_ulong global_mem = 0;
   cl_ulong l2_mem     = 0;
   cl_ulong msmc_mem   = 0;

   device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &NUMCOMPUNITS);
   device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE,   &global_mem);
   device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE,    &l2_mem);

#ifdef CL_DEVICE_MSMC_MEM_SIZE_TI
   device.getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI,  &msmc_mem);
#endif

   global_mem    = roundDownPower2(global_mem);
   L2_BUF_SIZE   = roundDownPower2(l2_mem);
   MSMC_BUF_SIZE = roundDownPower2(msmc_mem);

   /*----------------------------------------------------------------------
   * How big of a square matrix can we use.  Need 3 BTW
   *---------------------------------------------------------------------*/
   if (!M && !N && !K)
   {
       M = N = K = roundDownPower2(sqrt(global_mem / 3 / sizeof(float)));
#ifdef _TI_RTOS
       if (M >= 2048) M = N = K = 2048;
#endif
   }

   NUMAPANELS    = L2_BUF_SIZE / 2 / APanelSz;
   NUMBPANELS    = L2_BUF_SIZE / 4 / BPanelSz;

   if ((NUMCOMPUNITS * APanelSz * NUMAPANELS) > MSMC_BUF_SIZE)
        MSMC_BUF_SIZE = 0;
   else MSMC_BUF_SIZE = NUMCOMPUNITS * APanelSz * NUMAPANELS;

   if (calc_check)
   {
       cout << "M,N,K         = " << M << ", " 
            << N << ", "
            << K << endl;
       cout << "MSMC_BUF_SIZE = " << MSMC_BUF_SIZE << endl;
       cout << "L2_BUF_SIZE   = " << L2_BUF_SIZE << endl;
       cout << "NUMAPANELS    = " << NUMAPANELS << endl;
       cout << "NUMBPANELS    = " << NUMBPANELS << endl;
   }
}
