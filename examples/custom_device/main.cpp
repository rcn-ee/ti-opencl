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
#include "kernel.dsp_h"

using namespace cl;
using namespace std;

const int size   = 1 << 20;

#define MALLOC __malloc_ddr
#define FREE   __free_ddr

#define RETURN(x) return x
int main(int argc, char *argv[])
{
   /*-------------------------------------------------------------------------
   * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
   *------------------------------------------------------------------------*/
   signal(SIGABRT, exit);
   signal(SIGTERM, exit);

   int num_errors = 0;
   cl_char *A = (cl_char *) MALLOC(size);
   cl_char *B = (cl_char *) MALLOC(size);
   cl_char *C = (cl_char *) MALLOC(size);
   memset(A, 'a', size);
   memset(B, 'b', size);
   memset(C, 'c', size);

   try
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR |
                                 CL_DEVICE_TYPE_CUSTOM);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     std::vector<Device> dsp_devices;
     std::vector<Device> eve_devices;
     std::vector<bool>   is_dsps;
     for (auto dev : devices)
     {
       bool is_dsp = dev.getInfo<CL_DEVICE_TYPE>() != CL_DEVICE_TYPE_CUSTOM;
       is_dsps.push_back(is_dsp);
       if (is_dsp)
         dsp_devices.push_back(dev);
       else
         eve_devices.push_back(dev);
     }
     if (eve_devices.size() == 0)
     {
       cout << "No custom devices present.  Skip this test." << endl;
       FREE(A);
       FREE(B);
       FREE(C);
       RETURN(0);
     }

     Buffer  bufA(context, CL_MEM_READ_WRITE| CL_MEM_USE_HOST_PTR, size, A);
     Buffer  bufB(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, size, B);
     Buffer  bufC(context, CL_MEM_WRITE_ONLY| CL_MEM_USE_HOST_PTR, size, C);

     Program eve_program(context, eve_devices,
                     "tiocl_bik_memcpy_test;tiocl_bik_vecadd");
     //eve_program.build(eve_devices);
     Kernel  K_eve(eve_program, "tiocl_bik_memcpy_test");

     Program::Binaries bin(1, make_pair(kernel_dsp_bin,sizeof(kernel_dsp_bin)));
     Program dsp_program(context, dsp_devices, bin);
     dsp_program.build(dsp_devices);
     Kernel K_dsp(dsp_program, "k_memcpy");

     std::vector<Buffer> subAs, subBs;
     std::vector<CommandQueue*> Qs;
     std::vector<Event*> evs;

     // EVEs: bufB -> bufA
     int num_devices = eve_devices.size();
     for (int d = 0; d < num_devices; d++)
     {
       cl_buffer_region region;
       region.origin = d * (size / num_devices);
       region.size   = size / num_devices;
       subAs.push_back(bufA.createSubBuffer(CL_MEM_WRITE_ONLY,
                                       CL_BUFFER_CREATE_TYPE_REGION, &region));
       subBs.push_back(bufB.createSubBuffer(CL_MEM_READ_ONLY,
                                       CL_BUFFER_CREATE_TYPE_REGION, &region));
       Qs.push_back(new CommandQueue(context, eve_devices[d],
                                     CL_QUEUE_PROFILING_ENABLE));
       evs.push_back(new Event());
     }

     K_eve.setArg(2, size / num_devices);
     for (int d = 0; d < num_devices; d++)
     {
       K_eve.setArg(0, subAs[d]);
       K_eve.setArg(1, subBs[d]);
       Qs[d]->enqueueTask(K_eve, NULL, evs[d]);
     }

     for (int d = 0; d < num_devices; d++)
       evs[d]->wait();

     for (int d = 0; d < num_devices; d++)
     {
       cout << "Custom device " << d << ":" << endl;
       ocl_event_times(*evs[d], "Kernel Exec");
     }

     for (int i = 0; i < (size / num_devices) * num_devices; i++)
         if (A[i] != B[i])  num_errors += 1;

     for (int d = 0; d < num_devices; d++)
     {
       delete Qs[d];
       delete evs[d];
     }

     // DSP: bufA -> bufC
     CommandQueue Qdsp(context, dsp_devices[0],  CL_QUEUE_PROFILING_ENABLE);
     Event ev_dsp;
     Kernel K_null(dsp_program, "null");
     Qdsp.enqueueTask(K_null);
     Qdsp.finish();

     K_dsp.setArg(0, bufC);
     K_dsp.setArg(1, bufA);
     // K_dsp.setArg(2, size);
     Qdsp.enqueueNDRangeKernel(K_dsp, NullRange, NDRange(size),
                               NDRange(size/8), NULL, &ev_dsp);
     ev_dsp.wait();
     cout << "DSP device:" << endl;
     ocl_event_times(ev_dsp, "Kernel Exec");

     for (int i = 0; i < (size / num_devices) * num_devices; i++)
         if (C[i] != B[i])  num_errors += 1;
   }
   catch (Error err)
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
     exit(-1);
   }

   FREE(A);
   FREE(B);
   FREE(C);

   if (num_errors == 0)  std::cout << "Success!" << std::endl;
   else                  std::cout << "Failed with " << num_errors
                                   << " errors!" << std::endl;

   RETURN(num_errors == 0 ? 0 : -1);
}
