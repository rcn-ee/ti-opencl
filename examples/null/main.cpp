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
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include <ti/sysbios/posix/_time.h>
#include "kernel.dsp_h"
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

static unsigned us_diff (struct timespec &t1, struct timespec &t2)
{ return (t2.tv_sec - t1.tv_sec) * 1e6 + (t2.tv_nsec - t1.tv_nsec) / 1e3; }

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

   try 
   {
     Context             context (CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     CommandQueue        Q(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

#ifndef _TI_RTOS
     ifstream t("kernel.cl");
     std::string         kSrc((istreambuf_iterator<char>(t)),
                               istreambuf_iterator<char>());
     Program::Sources    source(1, make_pair(kSrc.c_str(), kSrc.length()));
     Program             program = Program(context, source);
#else
     Program::Binaries   binary(1, make_pair(kernel_dsp_bin,
                                             sizeof(kernel_dsp_bin)));
     Program             program = Program(context, devices, binary);
#endif
     program.build(devices); 

     Kernel kernel(program, "Null");
     KernelFunctor null = kernel.bind(Q, NDRange(1), NDRange(1));

     printf("The OpenCL runtime will lazily load the device program upon the \n");
     printf(" first enqueue of a kernel from the program, so the elapsed time \n");
     printf(" overall from the first enqueue will be longer to account for the \n");
     printf(" loading of the kernel.  Also, subsequent enqueue's of a kernel will \n");
     printf(" also potentially benefit from a warm cache on the device.\n\n");

     clock_gettime(CLOCK_MONOTONIC, &t0);
     null().wait();
     clock_gettime(CLOCK_MONOTONIC, &t1);
     printf("Elapsed (with Load): %d usecs\n", us_diff(t0, t1));

     for (int i = 0; i < 5; ++i)
     {
         clock_gettime(CLOCK_MONOTONIC, &t0);
         Event ev = null();
         ev.wait();
         clock_gettime(CLOCK_MONOTONIC, &t1);
         printf("Elapsed (w/o  Load): %d usecs\n", us_diff(t0, t1));
         ocl_event_times(ev, "Null Kernel Exec");
     }
   }
   catch (Error err) 
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
   }

   cout << "Done!" << endl; 

   RETURN(0);
}
