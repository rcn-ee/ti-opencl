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

using namespace cl;
using namespace std;

const char * kernelStr =
    "kernel void VectorAdd(global const short4* a, \n"
    "                      global const short4* b, \n"
    "                      global short4* c) \n"
    "{\n"
    "    int id = get_global_id(0);\n"
    "    c[id] = a[id] + b[id];\n"
    "}\n";

// Total available device memory via CMEM DDR blocks,
// equally divide into three cl_short arrays: dst[] = srcA[] + srcB[]
// Choose one of the following settings based on your CMEM configuration

// 1. For available CMEM DDR Blocks: ~1.5GB
const int64_t NumElements     = 128 * 1024*1024; // 256MB each array
#define CHUNKSIZE	128*1024*1024  // 128MB each time, works
// #define CHUNKSIZE	256*1024*1024  // 256MB each time, also works

// 2. For available CMEM DDR Blocks: ~3.5GB
// const int64_t NumElements     = 512 * 1024*1024; // 1024MB each array
// #define CHUNKSIZE	256*1024*1024  // 256MB each time, works
// #define CHUNKSIZE	512*1024*1024  // 512MB each time, also works

// 3. For available CMEM DDR Blocks: ~7.5GB
//const int64_t NumElements     = 1024 * 1024*1024; // 2048 each array
//#define CHUNKSIZE	256*1024*1024  // 256MB each time, works

const int64_t NumWorkGroups   = 256;
const int64_t VectorElements  = 4;
const int64_t NumVecElements  = NumElements / VectorElements;
const int64_t WorkGroupSize   = NumVecElements / NumWorkGroups;

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int64_t    bufsize = sizeof(cl_short) * NumElements;
   int num_errors = 0;

   try 
   {
     Context context (CL_DEVICE_TYPE_ACCELERATOR);

     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
      
     int d = 0;
     std::string str;
     devices[d].getInfo(CL_DEVICE_NAME, &str);
     cout << "DEVICE: " << str << endl << endl;

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices); 

     Kernel kernel(program, "VectorAdd");

     Event ev1,ev2,ev3,ev4, ev6,ev7,ev8;

     CommandQueue Q(context, devices[d], CL_QUEUE_PROFILING_ENABLE);

    for (int chunk = 0; chunk * CHUNKSIZE < bufsize; chunk++)
    {
        cl_uint remain_size = bufsize - chunk * CHUNKSIZE;
        if (remain_size > CHUNKSIZE) remain_size = CHUNKSIZE;
        cl_uint remain_elements = remain_size / sizeof(cl_short);
        cl_uint remain_vec_elements = remain_elements / VectorElements;
        cl_uint chunk_element_start =  chunk * CHUNKSIZE / sizeof(cl_short);
    
        printf("Chunk %d: elements 0x%x to 0x%x\n", chunk,
               chunk_element_start, chunk_element_start + remain_elements);
    
        short * srcA = (short*) Q.enqueueMapBuffer(bufA, CL_TRUE, CL_MAP_WRITE, 
                                   chunk * CHUNKSIZE, remain_size, NULL, &ev1);
        short * srcB = (short*) Q.enqueueMapBuffer(bufB, CL_TRUE, CL_MAP_WRITE,
                                   chunk * CHUNKSIZE, remain_size, NULL, &ev2);
    
        for (int i=0; i < remain_elements; ++i) 
        {
            srcA[i]   = srcB[i] = ((chunk * CHUNKSIZE/2) +(i+1)) % 12345; 
        }
    
        Q.enqueueUnmapMemObject(bufA, srcA, NULL, &ev3);
        Q.enqueueUnmapMemObject(bufB, srcB, NULL, &ev4);
    
        cl_buffer_region region;
        region.origin = chunk*CHUNKSIZE;
        region.size   =  remain_size;
        Buffer subA = bufA.createSubBuffer(CL_MEM_READ_ONLY,
                                        CL_BUFFER_CREATE_TYPE_REGION, &region);
        Buffer subB = bufB.createSubBuffer(CL_MEM_READ_ONLY,
                                        CL_BUFFER_CREATE_TYPE_REGION, &region);
        Buffer subC = bufDst.createSubBuffer(CL_MEM_WRITE_ONLY,
                                        CL_BUFFER_CREATE_TYPE_REGION, &region);
    
        kernel.setArg(0, subA);
        kernel.setArg(1, subB);
        kernel.setArg(2, subC);
    
        std::vector<Event> vec_ev5(1);
        Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(remain_vec_elements), 
                               NDRange(WorkGroupSize), NULL, &vec_ev5[0]);
    
        ev3.wait();  // otherwise, we may run short of host address space
        short * dst = (short*)Q.enqueueMapBuffer(bufDst, CL_TRUE, CL_MAP_READ,
                               chunk * CHUNKSIZE, remain_size, &vec_ev5, &ev6);
    
        for (int i=0; i < remain_elements; ++i)
        {
          cl_short da = ((chunk * CHUNKSIZE/2)+i+1) % 12345;
          if (da+da != dst[i]) 
          { 
    	  cout << "Element " << i << ": " 
    	       << da+da << " <==> " << dst[i] << endl; 
              num_errors += 1;
              break;
          }
        }
    
        Q.enqueueUnmapMemObject(bufDst, dst, NULL, &ev7);
        ev7.wait();
    
        ocl_event_times(ev1, "Map   BufA  ");
        ocl_event_times(ev2, "Map   BufB  ");
        ocl_event_times(ev3, "UnMap BufA  ");
        ocl_event_times(ev4, "UnMap BufB  ");
        ocl_event_times(vec_ev5[0], "Kernel      ");
        ocl_event_times(ev6, "Map   BufDst");
        ocl_event_times(ev7, "UnMap BufDst");
    }

   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (num_errors == 0) cout << "Success!" << endl; 
   else { cout << "Results failed to verify!" << endl; return -1; }
}

