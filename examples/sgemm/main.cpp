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
void CheckForErrors(const float *Mat, const float *Golden, int M, int N, int K,
                    enum CBLAS_ORDER mem_order);

/* ======================================================================== */
/*  Global Variables                                                        */
/* ======================================================================== */
int M                               = 4096;
int N                               = 4096;
int K                               = 4096;
float alpha                         = 1.0f;
float beta                          = 0.0f;
enum CBLAS_ORDER order              = CblasColMajor;
bool check                          = false;
bool random_in                      = false;

int L2_BUF_SIZE                     = 0;
int MSMC_BUF_SIZE                   = 0;
int NUMAPANELS                      = 0;
int NUMBPANELS                      = 0;
int NUMCOMPUNITS                    = 0;
bool use_host_ATLAS                 = true;

/* ======================================================================== */
/*  Function Headers                                                        */
/* ======================================================================== */
void PrintUsageAndExit();
void HandleOptions(int argc, char* argv[]);
bool SetSgemmParams(std::string& platform_name);

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
    /*  Handle command line options to get M,N,K                        */
    /* ---------------------------------------------------------------- */
    HandleOptions(argc,argv);

    int VALRANGE = 37;
    if (random_in) 
    {
        srand(time(NULL));
        alpha = (float) (rand() % VALRANGE + 1);
        beta  = (float) (rand() % VALRANGE + 1);
        printf("C[%d,%d] = alpha * A[%d,%d] * B[%d,%d] + beta * C[%d,%d], %s\n",
               M,N,M,K,K,N, M,N,
               (order == CblasRowMajor ? "use row-major storage"
                                       : "use col-major storage"));
        printf("alpha=%f, beta=%f\n\n", alpha, beta);
    }
    else
        printf("C[%d,%d] = A[%d,%d] * B[%d,%d], %s\n\n", M,N,M,K,K,N,
               (order == CblasRowMajor ? "use row-major storage"
                                       : "use col-major storage"));

    double dsp_elapsed = 0;
    double total_GFLOP = 2.0*M*N*K*1.0e-9;

    try 
    {
       Context context(CL_DEVICE_TYPE_ACCELERATOR);
       std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
       CommandQueue Q(context, devices[0]);

       /*---------------------------------------------------------------------
       * Determine platform, set sgemm blocking/tiling parameters
       *--------------------------------------------------------------------*/
       std::vector<Platform> platforms;
       Platform::get(&platforms);
       std::string platform_name;
       platforms[0].getInfo(CL_PLATFORM_NAME, &platform_name);
       devices[0].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &NUMCOMPUNITS);
       if (! SetSgemmParams(platform_name) || NUMCOMPUNITS == 0)  return -1;

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

       for (int i = 0; i < M*K; ++i)
           A[i] = random_in ? (float)(rand() % VALRANGE + 1) : i & 7; 
       for (int i = 0; i < K*N; ++i)
           B[i] = random_in ? (float)(rand() % VALRANGE + 1) : i & 7; 
       for (int i = 0; i < M*N; ++i)
           C[i] = random_in ? (float)(rand() % VALRANGE + 1) : 0;
       if (check)
       {
           if ((gold = (float*) malloc(M*N*sizeof(float))) == NULL)
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

           bool atlas_called = false;
           if (use_host_ATLAS)
           {
#ifdef ATLAS
               MatmulHost_ATLAS(order, A, B, gold, M, N, K, alpha, beta);
               atlas_called = true;
#else
               MatmulHost_loopnest(order, A, B, gold, M, N, K, alpha, beta);
#endif
           } else {
               MatmulHost_loopnest(order, A, B, gold, M, N, K, alpha, beta);
           }

           clock_gettime(CLOCK_MONOTONIC, &t1);
           dsp_elapsed = clock_diff (&t0, &t1);

           double gflops = total_GFLOP/dsp_elapsed;
           printf("Host CPUs: %.3f Gflops (%.6f s) with %s\n\n",
                  gflops, dsp_elapsed,
                  (atlas_called ? "ATLAS libray" : "3-level loop nest"));
           Q.enqueueUnmapMemObject(bufA, A);
           Q.enqueueUnmapMemObject(bufB, B);
	   PrintMatrix(gold,M,N,order); 
	   CheckForErrors(C, gold, M, N, K, order);
       }

       PrintMatrix(C,M,N,order); 
       Q.enqueueUnmapMemObject(bufC, C); Q.finish();
   }
   catch (Error err)
   {
       cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
       exit(-1);
   }

   return 0;
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
         << "-c     : Check results against host computation" << endl
         << "-l     : Host use 3-level loop nest to compute" << endl
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
    
    while ((c = getopt (argc, argv, "o:M:K:N:hclr")) != -1)
        switch(c)
        {
            case 'o': order = (*optarg == 'r') ? CblasRowMajor
                                               : CblasColMajor; break;
            case 'M': M = abs(atoi(optarg)); break;
            case 'K': K = abs(atoi(optarg)); break;
            case 'N': N = abs(atoi(optarg)); break;
            case 'h': PrintUsageAndExit();   break;
            case 'c': check = true;          break;
            case 'l': use_host_ATLAS = false;break;
            case 'r': random_in = true;      break;
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
void CheckForErrors(const float *Mat, const float *Golden, int M, int N, int K,
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
         cout << "FAIL with " << num_errors << " errors!\n";
    else cout << "PASS!" << endl;
}

/*-----------------------------------------------------------------------------
* If ATLAS sgemm is available, use it for the correctness test as it is faster
*----------------------------------------------------------------------------*/
#ifdef ATLAS
void MatmulHost_ATLAS(enum CBLAS_ORDER mem_order,
                const float*A, const float *B, float *C, int M, int N, int K,
                float alpha, float beta)
{
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
    
}
#endif


void MatmulHost_loopnest(enum CBLAS_ORDER mem_order,
                const float*A, const float *B, float *C, int M, int N, int K,
                float alpha, float beta)
{
    if (mem_order == CblasRowMajor)
    {
        // Compute gold in Row Major
        #pragma omp parallel for collapse(2) firstprivate(M,N,K) shared(A,B,C)
        for (int i = 0; i < M; ++i) // M rows
            for (int j = 0; j < N; ++j) // N columns
            {
                float sum = beta * C[i*N + j];
                for (int k = 0; k < K; ++k)
                {
                    sum +=   alpha
                           * A[i*K + k]  // Row Major
                           * B[k*N + j]; // Row Major
                }
                C[i*N + j] = sum; // Store gold in Row Major
            }
    } else {
        // Compute gold in Column Major
        #pragma omp parallel for collapse(2) firstprivate(M,N,K) shared(A,B,C)
        for (int j = 0; j < N; ++j) // N columns
            for (int i = 0; i < M; ++i) // M rows
            {
                float sum = beta * C[j*M + i];
                for (int k = 0; k < K; ++k)
                {
                    sum +=   alpha
                           * A[i + k*M]  // Column Major
                           * B[k + j*K]; // Column Major
                }
                C[j*M + i] = sum; // Store gold in Column Major
            }
    }
}


/* Check platform name, set sgemm blocking/tiling parameters accordingly
*/
bool SetSgemmParams(std::string& platform_name)
{
    // TODO: query the device about available L2 and MSMC, compute A/B Panels
    if (platform_name == "TI KeyStone II")
    {
        L2_BUF_SIZE    = (512 << 10);  // 512KB L2 SRAM for scratch
        MSMC_BUF_SIZE  = (2 << 20);    // 2MB MSMC for scratch, 256K each core
        NUMAPANELS     = 32;
        NUMBPANELS     = 8;
#ifndef ATLAS
        use_host_ATLAS = false;
#endif
        return true;
    }
    else if (platform_name == "TI AM57x")
    {
        L2_BUF_SIZE    = (128 << 10);  // 128KB L2 SRAM for scratch
        MSMC_BUF_SIZE  = 0;
        NUMAPANELS     = 8;
        NUMBPANELS     = 2;
        use_host_ATLAS = false;
        return true;
    }

    printf("Unsupported platform: %s, for this sgemm example, exiting\n",
           platform_name.c_str());
    return false;
}

