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
#include <cassert>
#include <signal.h>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include "kernel.dsp_h"
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

const int size   = 1 << 23; 
const int wgsize = 1 << 14;

#ifdef _TI_RTOS
void ocl_main(UArg arg0, UArg arg1)
{
   int    argc = (int)     arg0;
   char **argv = (char **) arg1;
   cl_char *ary = (cl_char *) __malloc_ddr(size);
   assert(ary != nullptr);
#else
#define RETURN(x) return x
cl_char ary [size];
int main(int argc, char *argv[])
{
#endif
   /*-------------------------------------------------------------------------
   * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
   *------------------------------------------------------------------------*/
   signal(SIGABRT, exit);
   signal(SIGTERM, exit);
   memset(ary, 0, size);

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     Buffer              buf (context, CL_MEM_WRITE_ONLY, size);

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

     CommandQueue  Q (context, devices[0]);
     Kernel        K (program, "devset");
     KernelFunctor devset = K.bind(Q, NDRange(size), NDRange(wgsize));

     devset(buf).wait(); // call the kernel and wait for completion

     Q.enqueueReadBuffer(buf, CL_TRUE, 0, size, ary);
   }
   catch (Error err) 
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
     exit(-1);
   }

   for (int i = 0; i < size; ++i) assert(ary[i] == 'x');
#ifdef _TI_RTOS
   __free_ddr(ary);
#endif
   std::cout << "Done!" << std::endl;

   RETURN(0);
}
