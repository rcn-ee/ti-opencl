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
#include <CL/cl.hpp>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <signal.h>
#include "ocl_util.h"
#ifndef _TI_RTOS
#include "omp.h"
#endif

#ifdef _TI_RTOS
#include <ti/sysbios/posix/_time.h>
#include "kernel.dsp_h"
#include <assert.h>
#include "../rtos_main.c"
#endif

/******************************************************************************
* C[N][M] = A[N][K] * B[K][M];
******************************************************************************/
using namespace cl;
using namespace std;
using std::cout;
using std::cerr;
using std::endl;

#define DIM 256
const int mat_N     = DIM;     
const int mat_K     = DIM;     
const int mat_M     = DIM;     

#ifndef _TI_RTOS
float A       [mat_N * mat_K];
float B       [mat_K * mat_M];
float C       [mat_N * mat_M];
float Golden  [mat_N * mat_M];
#endif

static double clock_diff (struct timespec *t1, struct timespec *t2);
static void   print_mat(float *mat, int rows, int cols);
static void   print_result(float *mat, float *gold, int rows, int cols);
static float  dotprod(const float * A, const float * B, int n);
static void   cpu_mat_mpy(const float *A, const float *B, float *C, 
                          int N, int K, int M);

/******************************************************************************
* main
******************************************************************************/
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

   struct timespec tp_start, tp_end;

   printf("float C[%d][%d] = float A[%d][%d] x float B[%d][%d]\n",
           mat_N, mat_M, mat_N, mat_K, mat_K, mat_M);

   int mat_size = DIM * DIM * sizeof(cl_float);
#ifdef _TI_RTOS
   float *A      = (float *) __malloc_ddr(mat_size);
   float *B      = (float *) __malloc_ddr(mat_size);
   float *C      = (float *) __malloc_ddr(mat_size);
   float *Golden = (float *) __malloc_ddr(mat_size);
   assert(A != nullptr && B != nullptr && C != nullptr && Golden != nullptr);
#endif

   /*--------------------------------------------------------------------------
   * Initialize the input matrices to random data
   *-------------------------------------------------------------------------*/
   srand(time(NULL));
   for (int i=0; i < mat_N * mat_K; ++i) A[i] = rand() % 5 + 1;
   for (int i=0; i < mat_K * mat_M; ++i) B[i] = rand() % 5 + 1;
   for (int i=0; i < mat_N * mat_M; ++i) C[i] = 0.0;

   try 
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     int                 nDev    = devices.size();

     /*---------------------------------------------------------------------
     * Compile the Kernel Source for the devices
     *--------------------------------------------------------------------*/
#ifndef _TI_RTOS
     char *bin;
     int bin_length = ocl_read_binary("kernel.out", bin);

     Program::Binaries   binary(1, std::make_pair(bin, bin_length));
     Program             program = Program(context, devices, binary);
     program.build(devices);

     delete [] bin;
#else
     Program::Binaries   binary(1, make_pair(kernel_dsp_bin,
                                             sizeof(kernel_dsp_bin)));
     Program             program = Program(context, devices, binary);
     program.build(devices);
#endif
 
     Buffer bufB   (context, CL_MEM_READ_ONLY,  mat_size);
     Buffer bufGold(context, CL_MEM_READ_ONLY,  mat_size);
     Kernel kernel (program, "ocl_matmpy");

     kernel.setArg(1, bufB);
     kernel.setArg(3, __local(mat_K * sizeof(float)));
     kernel.setArg(4, mat_K);
     kernel.setArg(5, mat_N);

     unsigned AChunk = mat_size / nDev;
     unsigned CChunk = mat_size / nDev;

     std::vector<CommandQueue*> Q (nDev);

     cl_mem_flags use_msmc = CL_MEM_USE_MSMC_TI;
     for (int d = 0; d < nDev; d++)
     {
         std::string dev_exts;
         devices[d].getInfo(CL_DEVICE_EXTENSIONS, &dev_exts);
         if (dev_exts.find("cl_ti_msmc_buffers") != std::string::npos)
         {
            cl_ulong msmc_size = 0;
            devices[d].getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI, &msmc_size);
            if (msmc_size >= AChunk)  continue;
         }
         use_msmc = 0;
         break;
     }
     std::vector<Buffer> bufA(nDev, Buffer(context, 
                                           CL_MEM_READ_ONLY|use_msmc,  AChunk));
     std::vector<Buffer> bufC(nDev, Buffer(context, CL_MEM_WRITE_ONLY, CChunk));
     std::vector<Event>  ev(nDev, Event());

     for (int d = 0; d < nDev; d++) Q[d]= new CommandQueue (context,devices[d]);

     clock_gettime(CLOCK_MONOTONIC, &tp_start);
     for (int d = 0; d < nDev; d++)
     {
         Q[d]->enqueueWriteBuffer(bufA[d], CL_FALSE, 0, AChunk, 
                                  &A[d*AChunk/sizeof(float)]);

         Q[d]->enqueueWriteBuffer(bufB, CL_FALSE, 0, mat_size, B);

         /*--------------------------------------------------------------------
         * One work item per cell in result matrix
         * One work group per column in result matrix
         *-------------------------------------------------------------------*/
         kernel.setArg(0, bufA[d]);
         kernel.setArg(2, bufC[d]);
         Q[d]->enqueueNDRangeKernel(kernel, NullRange, NDRange(mat_M/nDev), 
                                                       NDRange(1));
         Q[d]->enqueueReadBuffer(bufC[d], CL_FALSE, 0, CChunk, 
                                 &C[d*CChunk/sizeof(float)], NULL, &ev[d]);
     }

     for (int d = 0; d < Q.size(); d++) ev[d].wait();
     clock_gettime(CLOCK_MONOTONIC, &tp_end);

     /*---------------------------------------------------------------------
     * Cleanup OpenCL objects
     *--------------------------------------------------------------------*/
     for (int d = 0; d < Q.size(); d++) delete Q[d];

     double elapsed = clock_diff (&tp_start, &tp_end);
     printf("OpenCL dispatching to %d DSP(S): %6.4f secs\n", nDev, elapsed);
   }

   catch (Error err) 
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
     exit(-1);
   }

   clock_gettime(CLOCK_MONOTONIC, &tp_start);
   cpu_mat_mpy(A, B, Golden, mat_N, mat_K, mat_M);
   clock_gettime(CLOCK_MONOTONIC, &tp_end);

   double elapsed = clock_diff (&tp_start, &tp_end);
#ifndef _TI_RTOS
   printf("OpenMP dispatching to 4 CPU(S): %6.4f secs\n", elapsed);
#else
   printf("Host dispatching to 1 CPU(S): %6.4f secs\n", elapsed);
#endif

   print_mat(A,      mat_N, mat_K);
   print_mat(B,      mat_K, mat_M);
   print_mat(Golden, mat_N, mat_M);
   print_mat(C,      mat_N, mat_M);

   print_result(C, Golden, mat_N, mat_M);

   for (int i = 0; i < mat_N * mat_M; i++)
       if (Golden[i] != C[i])
       {
           int x = i / mat_M;
           int y = i % mat_M;

           std::cout << "Error at [" << x << "][" << y << "] : " 
                     << Golden[i] << " != " 
                     << C[i] << std::endl;
           RETURN(-1);
       }

#ifdef _TI_RTOS
   __free_ddr(A);
   __free_ddr(B);
   __free_ddr(C);
   __free_ddr(Golden);
#endif

   std::cout << "Passed!" << std::endl;

   RETURN(0);
}

/******************************************************************************
* cpu_mat_mpy
******************************************************************************/
void cpu_mat_mpy(const float * A, const float * B, float * C, int mat_N, 
                 int mat_K, int mat_M)
{
#pragma omp parallel for
    for (int col = 0; col < mat_M; ++col)
    {
        float b_col[mat_K];

        for (int row = 0; row < mat_K; ++row)
            b_col[row] = B[row*mat_M+col];

        for (int row = 0; row < mat_N; ++row)
            C[row*mat_M+col] = dotprod(A + (row * mat_K), b_col, mat_K);
    }
}

/******************************************************************************
* dotprod
******************************************************************************/
float dotprod(const float * A, const float * B, int n)
{
    float result = 0;
    for (int i = 0; i < n; ++i) result += A[i] * B[i];
    return result;
}

/******************************************************************************
* clock_diff 
******************************************************************************/
static double clock_diff (struct timespec *t1, struct timespec *t2)
       { return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec) / 1e9; }

/******************************************************************************
* print_mat
******************************************************************************/
static void print_mat(float *mat, int rows, int cols)
{
    if (cols > 16) return;

    for (int r = 0 ; r < rows; r++)
    {
      for (int c = 0 ; c < cols; c++)
          printf("%3.0f ", mat[r*(cols)+c]);
      printf("\n");
    }
    printf("\n");
}

static void print_result(float *mat, float *gold, int rows, int cols)
{
    if (cols > 64) return;

    for (int r = 0 ; r < rows; r++)
    {
      for (int c = 0 ; c < cols; c++)
          printf("%c", mat[r*(cols)+c] != gold[r*(cols)+c] ? 'x' : '-');
      printf("\n");
    }
    printf("\n");
}

