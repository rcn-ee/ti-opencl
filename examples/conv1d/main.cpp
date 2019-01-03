/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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
#include <CL/TI/cl.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <time.h>
#include "ocl_util.h"
#include "ti_kernels.dsp_h"

#ifdef _TI_RTOS
#include "../rtos_main.c"
    #if ti_sysbios_BIOS_version <= (0x65200)
    #include <ti/sysbios/posix/time.h>
    #else
    #include <ti/posix/gcc/time.h>
    #endif
#endif

// input image is COLSxROWS, filtered output image is COLSxROWS
// while filtering is applied to each row in image
#define COLS  1920
#define ROWS  1080
#define COLORDEPTH  12
// 1-D size 5 Gaussion Kernel, code is generic for non-symmetric kernel
#define FILTERSIZE  5
float FILTER[FILTERSIZE] = { 0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136f };
#define EPISOLON 0.00001f

using namespace cl;
using namespace std;

static unsigned us_diff (struct timespec &t1, struct timespec &t2)
{ return (t2.tv_sec - t1.tv_sec) * 1e6 + (t2.tv_nsec - t1.tv_nsec) / 1e3; }

static int VerifyResults(CommandQueue &Q, Buffer &pOutput,
                         float *pGolden, int cols, int rows);

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
  struct timespec t0, t1;
  int num_errors = 0;
  int input_numcompunits = 0;
  if (argc > 1)  input_numcompunits = atoi(argv[1]);  // valid: 1, 2, 4, 8

  try 
  {
    Context             context (CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    CommandQueue        Q(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
    int NUMCOMPUNITS;
    cl_ulong LOCALMEMSIZE;
    devices[0].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &NUMCOMPUNITS);
    devices[0].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &LOCALMEMSIZE);
    if (input_numcompunits > 0 && NUMCOMPUNITS > input_numcompunits)
        NUMCOMPUNITS = input_numcompunits;

    Program::Binaries   binary(1, make_pair(ti_kernels_dsp_bin,
                                            sizeof(ti_kernels_dsp_bin)));
    Program             program = Program(context, devices, binary);
    program.build(devices);

    // The OpenCL runtime will lazily load the device program upon the first
    // enqueue of a kernel from the program, so the elapsed time overall from
    // the first enqueue will be longer to account for the loading of the
    // program.  To remove program loading overhead from kernel performance,
    // enqueue a null kernel before running other kernels.
    Kernel kernel(program, "null");
    KernelFunctor null = kernel.bind(Q, NDRange(1), NDRange(1));
    clock_gettime(CLOCK_MONOTONIC, &t0);
    null().wait();
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Elapsed (loading program): %d usecs\n", us_diff(t0, t1));

    // Prepare testing data, compute golden output on host
    int bufSize = COLS * ROWS * sizeof(float);
    Buffer bufInput( context, CL_MEM_READ_ONLY, bufSize);
    Buffer bufOutput(context, CL_MEM_WRITE_ONLY, bufSize);
    Buffer bufFilter(context, CL_MEM_READ_ONLY, FILTERSIZE * sizeof(float));
#ifndef _TI_RTOS
    float *pGolden = (float *) malloc(bufSize);
#else
    float *pGolden = (float *) __malloc_ddr(bufSize);
#endif
    if (pGolden == NULL) 
    {
      printf("Failed to allocate memory for golden results\n");
      exit(0);
    }
    float *pInput = (float *) Q.enqueueMapBuffer(bufInput, CL_TRUE,
                                                 CL_MAP_WRITE, 0, bufSize);
    int VALRANGE = (1 << COLORDEPTH);
    for (int i = 0; i < COLS * ROWS; i++)
      pInput[i] = (float) (rand() % VALRANGE);
    srand(time(NULL));
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int j = 0; j < ROWS; j++)
      for (int i = 0; i < COLS; i++)
      {
        float left_2  = pInput[j * COLS + (i-2 < 0 ? 0 : i-2)];
        float left_1  = pInput[j * COLS + (i-1 < 0 ? 0 : i-1)];
        float self    = pInput[j * COLS + i];
        float right_1 = pInput[j * COLS + (i+1 >= COLS ? COLS-1 : i+1)];
        float right_2 = pInput[j * COLS + (i+2 >= COLS ? COLS-1 : i+2)];

        pGolden[j*COLS + i] = left_2 * FILTER[0] + left_1 * FILTER[1]
                            + self * FILTER[2]
                            + right_1 * FILTER[3] + right_2 * FILTER[4];
      }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Elapsed (host_compute): %d usecs\n", us_diff(t0, t1));
    Q.enqueueUnmapMemObject(bufInput, pInput);
    Q.finish();
    Q.enqueueWriteBuffer(bufFilter, CL_TRUE, 0, FILTERSIZE * sizeof(float),
                         FILTER);

    Event ev;
    // straight-forward kernel (unoptimized, baseline)
    Kernel k_base(program, "k_conv1d_5x1");
    k_base.setArg(0, bufInput);
    k_base.setArg(1, bufOutput);
    k_base.setArg(2, bufFilter);
    k_base.setArg(3, COLS);
    k_base.setArg(4, ROWS);
    k_base.setArg(5, COLS);
    k_base.setArg(6, COLS);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Q.enqueueNDRangeKernel(k_base, NullRange, NDRange(COLS, ROWS),
                           NDRange(COLS, ROWS/NUMCOMPUNITS), NULL, &ev);
    ev.wait();
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Elapsed (k_baseline): %d usecs\n", us_diff(t0, t1));
    num_errors += VerifyResults(Q, bufOutput, pGolden, COLS, ROWS);

    Kernel k_loop(program, "k_loop");
    k_loop.setArg(0, bufInput);
    k_loop.setArg(1, bufOutput);
    k_loop.setArg(2, bufFilter);
    k_loop.setArg(3, COLS);
    k_loop.setArg(4, ROWS);
    k_loop.setArg(5, COLS);
    k_loop.setArg(6, COLS);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Q.enqueueNDRangeKernel(k_loop, NullRange, NDRange(ROWS),
                           NDRange(ROWS/NUMCOMPUNITS), NULL, &ev);
    ev.wait();
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Elapsed (k_loop): %d usecs\n", us_diff(t0, t1));
    num_errors += VerifyResults(Q, bufOutput, pGolden, COLS, ROWS);

    Kernel k_loop_simd(program, "k_loop_simd");
    k_loop_simd.setArg(0, bufInput);
    k_loop_simd.setArg(1, bufOutput);
    k_loop_simd.setArg(2, bufFilter);
    k_loop_simd.setArg(3, COLS);
    k_loop_simd.setArg(4, ROWS);
    k_loop_simd.setArg(5, COLS);
    k_loop_simd.setArg(6, COLS);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Q.enqueueNDRangeKernel(k_loop_simd, NullRange, NDRange(ROWS),
                           NDRange(ROWS/NUMCOMPUNITS), NULL, &ev);
    ev.wait();
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Elapsed (k_loop_simd): %d usecs\n", us_diff(t0, t1));
    num_errors += VerifyResults(Q, bufOutput, pGolden, COLS, ROWS);

    int BLOCK_HEIGHT = LOCALMEMSIZE / (2 * 2 * COLS * sizeof(float));
    // set the double buffer pipeline to at least 8 times
    if (BLOCK_HEIGHT > (ROWS / NUMCOMPUNITS / 8))
      BLOCK_HEIGHT = (ROWS / NUMCOMPUNITS / 8) + 1;
    if (BLOCK_HEIGHT > 0)
    {
      Kernel k_loop_db(program, "k_loop_db");
      k_loop_db.setArg(0, bufInput);
      k_loop_db.setArg(1, bufOutput);
      k_loop_db.setArg(2, bufFilter);
      k_loop_db.setArg(3, COLS);
      k_loop_db.setArg(4, ROWS);
      k_loop_db.setArg(5, COLS);
      k_loop_db.setArg(6, COLS);
      k_loop_db.setArg(7, BLOCK_HEIGHT);
      k_loop_db.setArg(8, __local(BLOCK_HEIGHT*2*COLS*sizeof(float)));
      k_loop_db.setArg(9, __local(BLOCK_HEIGHT*2*COLS*sizeof(float)));
      clock_gettime(CLOCK_MONOTONIC, &t0);
      Q.enqueueNDRangeKernel(k_loop_db, NullRange,
                             NDRange(NUMCOMPUNITS), NDRange(1), NULL, &ev);
      ev.wait();
      clock_gettime(CLOCK_MONOTONIC, &t1);
      printf("Elapsed (k_loop_db): %d usecs\n", us_diff(t0, t1));
      num_errors += VerifyResults(Q, bufOutput, pGolden, COLS, ROWS);

      Kernel k_loop_simd_db(program, "k_loop_simd_db");
      k_loop_simd_db.setArg(0, bufInput);
      k_loop_simd_db.setArg(1, bufOutput);
      k_loop_simd_db.setArg(2, bufFilter);
      k_loop_simd_db.setArg(3, COLS);
      k_loop_simd_db.setArg(4, ROWS);
      k_loop_simd_db.setArg(5, COLS);
      k_loop_simd_db.setArg(6, COLS);
      k_loop_simd_db.setArg(7, BLOCK_HEIGHT);
      k_loop_simd_db.setArg(8, __local(BLOCK_HEIGHT*2*COLS*sizeof(float)));
      k_loop_simd_db.setArg(9, __local(BLOCK_HEIGHT*2*COLS*sizeof(float)));
      clock_gettime(CLOCK_MONOTONIC, &t0);
      Q.enqueueNDRangeKernel(k_loop_simd_db, NullRange,
                             NDRange(NUMCOMPUNITS), NDRange(1), NULL, &ev);
      ev.wait();
      clock_gettime(CLOCK_MONOTONIC, &t1);
      printf("Elapsed (k_loop_simd_db): %d usecs\n", us_diff(t0, t1));
      num_errors += VerifyResults(Q, bufOutput, pGolden, COLS, ROWS);

      Kernel k_loop_simd_db_extc(program, "k_loop_simd_db_extc");
      k_loop_simd_db_extc.setArg(0, bufInput);
      k_loop_simd_db_extc.setArg(1, bufOutput);
      k_loop_simd_db_extc.setArg(2, bufFilter);
      k_loop_simd_db_extc.setArg(3, COLS);
      k_loop_simd_db_extc.setArg(4, ROWS);
      k_loop_simd_db_extc.setArg(5, COLS);
      k_loop_simd_db_extc.setArg(6, COLS);
      k_loop_simd_db_extc.setArg(7, BLOCK_HEIGHT);
      k_loop_simd_db_extc.setArg(8, __local(BLOCK_HEIGHT*2*COLS*sizeof(float)));
      k_loop_simd_db_extc.setArg(9, __local(BLOCK_HEIGHT*2*COLS*sizeof(float)));
      clock_gettime(CLOCK_MONOTONIC, &t0);
      Q.enqueueNDRangeKernel(k_loop_simd_db_extc, NullRange,
                             NDRange(NUMCOMPUNITS), NDRange(1), NULL, &ev);
      ev.wait();
      clock_gettime(CLOCK_MONOTONIC, &t1);
      printf("Elapsed (k_loop_simd_db_extc): %d usecs\n", us_diff(t0, t1));
      num_errors += VerifyResults(Q, bufOutput, pGolden, COLS, ROWS);
    }

#ifndef _TI_RTOS
    free(pGolden);
#else
    __free_ddr(pGolden);
#endif
  }
  catch (Error err) 
  {
    cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
         << ocl_decode_error(err.err()) << ")" << endl;
  }

  if (num_errors != 0)
  {
    cout << "Failed with " << num_errors << " errors" << endl;
    RETURN(-1);
  } else
  {
    cout << "Pass!" << endl; 
    RETURN(0);
  }
}

static int VerifyResults(CommandQueue &Q, Buffer &bufOutput,
                         float *pGolden, int cols, int rows)
{
  float *pOutput = (float *) Q.enqueueMapBuffer(bufOutput, CL_TRUE,
                                                CL_MAP_READ | CL_MAP_WRITE,
                                                0, cols*rows*sizeof(float));
  int num_errors = 0;
  for (int i = 0; i < cols * rows; i++)
  {
    if ((pOutput[i] - pGolden[i] > EPISOLON) ||
        (pGolden[i] - pOutput[i] > EPISOLON))
      if (num_errors++ < 10)
        printf("Result diff at %d: expect %f, got %f\n", i, pGolden[i],
                                                         pOutput[i]);
  }
  if (num_errors != 0)  printf("Total %d errors\n", num_errors);

  // reset output buffer for next test
  Event ev;
  memset(pOutput, 0, cols*rows*sizeof(float));
  Q.enqueueUnmapMemObject(bufOutput, pOutput, NULL, &ev);
  ev.wait();

  return num_errors;
}

