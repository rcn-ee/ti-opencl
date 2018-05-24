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

/* Example: FFT batch processing multiple channels of samples
 * - Number of channels: FFTCHS
 * - Size of each channel: FFTSZ samples
 * - Data type: complex single precision input and output
 * - Each channel can be processed independently
 * - Each DSP will
 *   - move input data from DDR (global) into L2 scratch (local) with EDMA,
 *   - apply FFT processing on local data,
 *   - move output data back from L2 scratch (local) to DDR (global) with EDMA,
 *   - double buffer to overlap data movement and processing
 */

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include "ocl_util.h"

#include "kernel.dsp_h"
#ifdef _TI_RTOS
#include "../rtos_main.c"
#include <ti/sysbios/posix/time.h>
#else
#include <time.h>
#endif

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
using namespace cl;
using namespace std;

void init_input(float *x, int num_channels, int n);
void tw_gen(float *w, int n);
bool validate_output(float *y, int num_channels, int n, bool verbose);

/* ======================================================================== */
/*  Initialized arrays with fixed test data.                                */
/* ======================================================================== */
#define FFTCHS (64)      // number of channels
#define FFTSZ  (4*1024)  // max size for double buffering with 128 KB local
#define FFTRADIX 4       // if FFTSZ is power of 4, 2 if FFTSZ is power of 2

const double PI = 3.141592654;

static unsigned us_diff (struct timespec &t1, struct timespec &t2)
{ return (t2.tv_sec - t1.tv_sec) * 1e6 + (t2.tv_nsec - t1.tv_nsec) / 1e3; }


/* ======================================================================== */
/*  MAIN -- Top level driver for the test.                                  */
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
    int i, j;
    struct timespec t0, t1;
    int channel_size = 2 * FFTSZ * sizeof(float);
    bool verbose = argc > 1;
    bool failed = false;;

    // __malloc_ddr() returns 128 bytes aligned memory
    float *x = (float *) __malloc_ddr(FFTCHS * channel_size);
    float *y = (float *) __malloc_ddr(FFTCHS * channel_size);
    // same twiddle factor for all channels of same size
    float *w = (float *) __malloc_ddr(channel_size);
    if (x == nullptr || y == nullptr || w == nullptr)
    {
        std::cout << "Cannot allocate memory!" << std::endl;
        if (x != nullptr)  __free_ddr(x);
        if (y != nullptr)  __free_ddr(y);
        if (w != nullptr)  __free_ddr(w);
        RETURN(-1);
    }

    init_input(x, FFTCHS, FFTSZ);           // initialize input
    memset(y, 0xA5, FFTCHS * channel_size); // clear output
    tw_gen(w, FFTSZ);                       // Generate twiddle factors

 try
 {
    Context             context(CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    CommandQueue        Q(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
    int NUMCOMPUNITS;
    cl_ulong LOCALMEMSIZE;
    devices[0].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &NUMCOMPUNITS);
    devices[0].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &LOCALMEMSIZE);

    cout << "Offloading FFT (SP,Complex) of " << FFTCHS << " channels, each";
    cout << " with " << (FFTSZ / 1024) << "K elements..." << endl << endl;

    Buffer bufX(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
                channel_size * FFTCHS, x);
    Buffer bufY(context, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR,
                channel_size * FFTCHS, y);
    Buffer bufW(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
                channel_size, w);

    Program::Binaries   binary(1, make_pair(kernel_dsp_bin,
                                            sizeof(kernel_dsp_bin)));
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
    printf("loading program: %d usecs\n", us_diff(t0, t1));

    // Simple loop version with data in DDR
    Kernel fft(program, "ocl_DSPF_sp_fftSPxSP");
    fft.setArg(0, FFTSZ);
    fft.setArg(1, bufX);
    fft.setArg(2, bufW);
    fft.setArg(3, bufY);
    fft.setArg(4, FFTRADIX);
    fft.setArg(5, 0);
    fft.setArg(6, FFTSZ);
    fft.setArg(7, FFTCHS);

    Event e1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Q.enqueueNDRangeKernel(fft, NullRange, NDRange(NUMCOMPUNITS),
                           NDRange(1), NULL, &e1);
    e1.wait();
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("fft: %d usecs\n", us_diff(t0, t1));
    failed = validate_output(y, FFTCHS, FFTSZ, verbose);


    init_input(x, FFTCHS, FFTSZ);           // initialize input
    memset(y, 0xA5, FFTCHS * channel_size); // clear output

    // Double-buffering version with EDMAing data from DDR to L2
    Kernel fft_db(program, "ocl_DSPF_sp_fftSPxSP_db");
    fft_db.setArg(0, FFTSZ);
    fft_db.setArg(1, bufX);
    fft_db.setArg(2, bufW);
    fft_db.setArg(3, bufY);
    fft_db.setArg(4, FFTRADIX);
    fft_db.setArg(5, 0);
    fft_db.setArg(6, FFTSZ);
    fft_db.setArg(7, FFTCHS);
    int BLOCK_HEIGHT = 1; // increase BLOCK_HEIGHT if smaller size or bigger L2
    fft_db.setArg(8, BLOCK_HEIGHT);
    fft_db.setArg(9,  __local(BLOCK_HEIGHT*2*2*FFTSZ*sizeof(float)));
    fft_db.setArg(10, __local(BLOCK_HEIGHT*2*2*FFTSZ*sizeof(float)));

    Event e2;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Q.enqueueNDRangeKernel(fft_db, NullRange, NDRange(NUMCOMPUNITS),
                           NDRange(1), NULL, &e2);
    e2.wait();
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("fft_db: %d usecs\n", us_diff(t0, t1));
    failed |= validate_output(y, FFTCHS, FFTSZ, verbose);
  }
  catch (Error err)
  {
      cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
           << ocl_decode_error(err.err()) << ")" << endl;
  }

    __free_ddr(x);
    __free_ddr(y);
    __free_ddr(w);

    if (failed) { std::cout << "Failed!" << std::endl;  RETURN(-1); }
    else        { std::cout << "Success!" << std::endl;  RETURN(0); }
}

/******************************************************************************
* Function for initializing FFT input
******************************************************************************/
void init_input(float *x, int num_chs, int n)
{
    /* ---------------------------------------------------------------- */
    /* Each channels gets a (j+1) Hz sin wave in the sample window.     */
    /* ---------------------------------------------------------------- */
    for (int j = 0; j < num_chs; j++)
    {
        for (int i = 0; i < n; i++)
        {
            x[j*2*n + 2*i]     = sin(2*PI* (j+1) * i / n);
            x[j*2*n + 2*i + 1] = 0.f;
        }
    }
    /* ---------------------------------------------------------------- */
    /* Second half of channels gets first half of channels added.       */
    /* ---------------------------------------------------------------- */
    for (int j = num_chs/2; j < num_chs; j++)
    {
        for (int i = 0; i < n; i++)
        {
            x[j*2*n + 2*i]    += x[(j-num_chs/2)*2*n + 2*i];
        }
    }
}

/******************************************************************************
* Function for generating Specialized sequence of twiddle factors
******************************************************************************/
void tw_gen(float *w, int n)
{
    int i, j, k;

    for (j = 1, k = 0; j <= n >> 2; j = j << 2)
    {
        for (i = 0; i < n >> 2; i += j)
        {
            w[k]     = (float) sin (2 * PI * i / n);
            w[k + 1] = (float) cos (2 * PI * i / n);
            w[k + 2] = (float) sin (4 * PI * i / n);
            w[k + 3] = (float) cos (4 * PI * i / n);
            w[k + 4] = (float) sin (6 * PI * i / n);
            w[k + 5] = (float) cos (6 * PI * i / n);
            k += 6;
        }
    }
}

/******************************************************************************
* Function for validating FFT output
******************************************************************************/
bool validate_output(float *y, int num_chs, int n, bool verbose)
{
    const float EPSILON= (0.000001) * n;
    const int  num_freqs_expected = 4;
    int expected_freqs[num_freqs_expected];
    bool failed = false;

    for (int j = 0; j < num_chs; j++)
    {
        if (verbose) printf("Channel %d:\n", j);
        // first half expecting: (j+1) Hz, -(j+1) Hz (aka (n-j-1))
        expected_freqs[0] = j+1;
        expected_freqs[1] = n - expected_freqs[0];
        // second half get additional freqs from first half
        if (j >= num_chs/2)
        {
            expected_freqs[2] = j+1 - num_chs/2;
            expected_freqs[3] = n - expected_freqs[2];
        }
        else
        {
            expected_freqs[2] = -1;
            expected_freqs[3] = -1;
        }

        for (int i = 0; i < n; i++)
        {
            float re = y[j*2*n + 2*i];
            float im = y[j*2*n + 2*i+1];
            // or use magnitude to check: sqrt(re * re + im * im) / n > 0.001
            if (re < -EPSILON || re > EPSILON || im < -EPSILON || im > EPSILON)
            {
                int f_id;
                for (f_id = 0; f_id < num_freqs_expected; f_id++)
                    if (expected_freqs[f_id] == i)
                    {
                        expected_freqs[f_id] = -1;  // clear it out
                        break;
                    }
                if (f_id == num_freqs_expected)
                {
                    printf(
                     "Channel %d: unexpected freq %d, re=%f im=%f detected!\n",
                           j, i, re, im);
                    failed = true;
                }

                if (verbose) printf("  i=%d re=%f im=%f\n", i, re, im);
            }
        }

        // check if all freqs are detected
        for (int f_id = 0; f_id < num_freqs_expected; f_id++)
        {
            if (expected_freqs[f_id] != -1)
            {
                printf("Channel %d: expected freq %d not detected!\n",
                       j, expected_freqs[f_id]);
                failed = true;
            }
        }
    }

    return failed;
}

