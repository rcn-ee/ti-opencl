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
#include <cstdlib>
#include "ocl_util.h"
#include "kernel.dsp_h"

#ifdef _TI_RTOS
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

const int num_chunks = 1024;
const int chunk_size = 1024;
const int bytes      = num_chunks * chunk_size;

cl_uchar src   [bytes];
cl_uchar dst   [bytes];

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
   cl_int err     = CL_SUCCESS;
   int    bufsize = sizeof(src);

   for (int i=0; i < bytes; ++i) { src[i] = 0xAB; dst[i] = 0; }

   try 
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufSrc (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     Program::Binaries   binary(1, std::make_pair(kernel_dsp_bin, sizeof(kernel_dsp_bin)));
     Program             program = Program(context, devices, binary);
     program.build(devices);

     Kernel kernel(program, "oclEcpy");
     kernel.setArg(0, bufSrc);
     kernel.setArg(1, bufDst);
     kernel.setArg(2, chunk_size);

     CommandQueue Q(context, devices[0]);

     Q.enqueueWriteBuffer(bufSrc, CL_TRUE, 0, bufsize, src);
     Q.enqueueNDRangeKernel(kernel,NullRange,NDRange(num_chunks),NDRange(1));
     Q.enqueueReadBuffer (bufDst, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
   }

   for (int i=0; i < bytes; ++i)
       if (dst[i] != 0x000000AB) 
           { cout << "Failed at Element " << i << endl; RETURN(-1); }

   cout << "Success!" << endl; 

   RETURN(0);
}
