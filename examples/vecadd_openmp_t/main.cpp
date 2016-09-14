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
#include <stdio.h>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include "vadd_wrapper.dsp_h"
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

const int NumChunks       = 8;
const int ChunkSize       = 1024*16;
const int NumElements     = NumChunks*ChunkSize;
const int NumWorkGroups   = 256;
const int VectorElements  = 1;
const int NumVecElements  = NumElements / VectorElements;
const int WorkGroupSize   = NumVecElements / NumWorkGroups;

#define EPISILON  0.00001

float srcA  [NumElements];
float srcB  [NumElements];
float dst   [NumElements+8];
float Golden[NumElements];

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
   int    bufsize = sizeof(Golden);
   int    num_errors = 0;
   const int    print_nerrors = 12;

   for (int i=0; i < NumElements; ++i) 
   {
       srcA[i] = i * 1.0; 
       srcB[i] = ((i+7) % 257 )* 1.0; 
       Golden[i]   =   srcA[i] + srcB[i];
   }

   try 
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);

     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
      
     int d = 0;
     std::string str;
     devices[d].getInfo(CL_DEVICE_NAME, &str);
     cout << "DEVICE: " << str << endl << endl;

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

#ifndef _TI_RTOS
     ifstream t("vadd_wrapper.cl");
     if (!t)
     {
         std::cout << "Could not open Kernel Source file ([file].cl)\n";
         exit(-1);
     }

     std::string kSrc((istreambuf_iterator<char>(t)),
                      istreambuf_iterator<char>());
     Program::Sources    source(1, make_pair(kSrc.c_str(), kSrc.length()));
     Program             program = Program(context, source);
     program.build(devices, "vadd_openmp.obj"); 
#else
     Program::Binaries binary(1, make_pair(vadd_wrapper_dsp_bin,
                                              sizeof(vadd_wrapper_dsp_bin)));
     Program           program = Program(context, devices, binary);
     program.build(devices);
#endif

     Kernel kernel(program, "vadd_wrapper");
     kernel.setArg(0, bufA);
     kernel.setArg(1, bufB);
     kernel.setArg(2, bufDst);
     kernel.setArg(3, NumChunks);
     kernel.setArg(4, ChunkSize);

     Event ev1,ev2,ev3,ev4, ev5,  ev6,ev7,ev8;

     // In Order Command Queue, only one kernel pushed to device at a time
     // OpenMP c code should use: In Order Command Queue + Task
     CommandQueue InO_Q(context, devices[d], CL_QUEUE_PROFILING_ENABLE);

     InO_Q.enqueueWriteBuffer(bufA, CL_FALSE, 0, bufsize, srcA, NULL, &ev1);
     InO_Q.enqueueWriteBuffer(bufB, CL_FALSE, 0, bufsize, srcB, NULL, &ev2);

     std::vector<Event> vec_ev5(1);
     InO_Q.enqueueTask(kernel, NULL, &vec_ev5[0]);

     InO_Q.enqueueReadBuffer(bufDst, CL_TRUE, 0, bufsize, dst, &vec_ev5, &ev6);

     for (int i=0; i < NumElements; ++i)
     {
       if (Golden[i] - dst[i] < -EPISILON || Golden[i] - dst[i] > EPISILON) 
       { 
           if((num_errors += 1) < print_nerrors)
               printf("Error %d: %f <==> %f\n", i, Golden[i], dst[i]);
       }
     }

     ocl_event_times(ev1, "Write   BufA ");
     ocl_event_times(ev2, "Write   BufB ");
     ocl_event_times(vec_ev5[0], "Kernel       ");
     ocl_event_times(ev6, "Read   BufDst");
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (num_errors > 0)
   { 
      cout << "FAIL with " << num_errors << " errors!\n";
      RETURN (-1);
   }
   else cout << "PASS!" << endl; 

   RETURN (0);
}
