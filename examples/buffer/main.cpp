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
#include <cstdlib>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include "../rtos_main.c"
#endif

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const int NumElements = 32;

cl_int src  [NumElements];
cl_int dst  [NumElements];

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

   for (int i=0; i < NumElements; ++i) { src[i] = i; dst[i] = 0; }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer buf    (context, CL_MEM_READ_ONLY,  bufsize);
     CommandQueue Q(context, devices[0]);

     Q.enqueueWriteBuffer(buf, CL_TRUE, 0, bufsize, src);
     Q.enqueueReadBuffer (buf, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
   }

   if (memcmp(dst, src, bufsize) != 0) { cout << "Failed!" << endl;
                                         RETURN(-1); }
   else                                cout << "Passed!" << endl; 

   RETURN(0);
}
