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
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include "kernel.dsp_h"
#include "../rtos_main.c"
#endif

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
using namespace cl;
using namespace std;

void tw_gen(float *w, int n);

/* ======================================================================== */
/*  Initialized arrays with fixed test data.                                */
/* ======================================================================== */
#define  FFTSZ (64*1024)
#define  PAD   0

float x[2*FFTSZ];
float y[2*FFTSZ];
float w[2*FFTSZ];

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
    int rad = 4;
    
    /* ---------------------------------------------------------------- */
    /* Initialize input vector temporarily.                             */
    /* ---------------------------------------------------------------- */
    for (i = 0; i < FFTSZ; i++)
    {
        x[PAD + 2*i]     = sin (2 * 3.1415 * 50 * i / (double) FFTSZ);
        x[PAD + 2*i + 1] = 0;
    }

    memset (y, 0xA5, sizeof (y));       // fix value of output
    tw_gen (w, FFTSZ);               // Generate twiddle factors

   try
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     int d = 0;
     int bufsize = sizeof(float) * (2*FFTSZ + PAD + PAD);

     cout << "Offloading FFT (SP,Complex) of " << (FFTSZ / 1024);
     cout << "K elements..." << endl << endl;

     Buffer bufX(context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufY(context, CL_MEM_WRITE_ONLY, bufsize);
     Buffer bufW(context, CL_MEM_READ_ONLY,  bufsize);

#ifndef _TI_RTOS
     ifstream t("kernel.cl");
     std::string         kSrc((istreambuf_iterator<char>(t)),
                               istreambuf_iterator<char>());
     Program::Sources    source(1, make_pair(kSrc.c_str(), kSrc.length()));
     Program             program = Program(context, source);
     program.build(devices, "./dsplib.ae66");
#else
     Program::Binaries   binary(1, make_pair(kernel_dsp_bin,
                                             sizeof(kernel_dsp_bin)));
     Program             program = Program(context, devices, binary);
     program.build(devices);
#endif

     Kernel fft(program, "ocl_DSPF_sp_fftSPxSP");
     fft.setArg(0, FFTSZ);
     fft.setArg(1, bufX);
     fft.setArg(2, bufW);
     fft.setArg(3, bufY);
     fft.setArg(4, rad);
     fft.setArg(5, 0);
     fft.setArg(6, FFTSZ);

     CommandQueue Q(context, devices[d], CL_QUEUE_PROFILING_ENABLE);

     Event e3,e4;
     std::vector<Event> evs(2);
     Q.enqueueWriteBuffer(bufX, CL_FALSE, 0, bufsize, x, 0,    &evs[0]);
     Q.enqueueWriteBuffer(bufW, CL_FALSE, 0, bufsize, w, 0,    &evs[1]);
     Q.enqueueTask       (fft,                           &evs, &e3);
     Q.enqueueReadBuffer (bufY, CL_TRUE, 0, bufsize, y,  0,    &e4);

     ocl_event_times(evs[0], "Write X");
     ocl_event_times(evs[1], "Twiddle");
     ocl_event_times(e3,     "FFT");
     ocl_event_times(e4,     "Read Y");

   }
   catch (Error err)
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
   }

   std::cout << "Done!" << std::endl;

   RETURN(0);
}

/******************************************************************************
* Function for generating Specialized sequence of twiddle factors
******************************************************************************/
void tw_gen(float *w, int n)
{
    int i, j, k;
    const double PI = 3.141592654;

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
