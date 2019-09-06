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
#include <cstdio>
#include <cstdlib>
#include "ocl_util.h"

using namespace cl;
using namespace std;

#define EPISILON 0.00001

const cl_uint NumWorkGroups   = 256;
const cl_uint VectorElements  = 1;
// MPAX can handle 3 256MB buffers: a, b, c
const cl_uint MaxCHUNKSIZE    = 256*1024*1024;

// From the available device memory blocks, determine the maximum size of
// the 3 equally sized buffers that we can allocate: dst[] = srcA[] + srcB[],
// round the size to multiple of 512MB or nearest power of 2
cl_uint get_bufsize(cl_ulong gmem_size,cl_ulong gmem_size_ext1,
                    cl_ulong gmem_size_ext2)
{
     // Find the two bigger buffer sizes
     cl_ulong big = gmem_size;
     cl_ulong middle = gmem_size_ext1;
     if (big <= middle)
     {
        big = gmem_size_ext1;
        middle = gmem_size;
     }
     if (big <= gmem_size_ext2)
     {
        middle = big;
        big = gmem_size_ext2;
     }
     else if (middle <= gmem_size_ext2)
     {
        middle = gmem_size_ext2;
     }

     // Determine maximum size of 3 buffers than 2 bigger buffers can hold
     cl_ulong each_buf_size = 0;
     if (big > 3 * middle || middle == 0)
        each_buf_size = big / 3;
     else if (big > 2 * middle)
        each_buf_size = middle;
     else
        each_buf_size = (big / 2);
     if (each_buf_size == 0)
     {
         printf("Unable to allocate memory, exiting...\n");
         return -1;
     }
     // Cap size of each buffer in 32-bit address space
     if (each_buf_size > 0xFFFFFFFFU)  each_buf_size = 0xFFFFFFFFU;

     // Round down bufsize to be multiple of MaxCHUNKSIZE
     //                          or power of 2s (if < MaxCHUNKSIZE)
     cl_uint bufsize = (each_buf_size / MaxCHUNKSIZE) * MaxCHUNKSIZE;
     if (bufsize == 0)
     {
         int power2bit = 1;
         while ((each_buf_size >> power2bit) != 0)  power2bit++;
         bufsize = 1ULL << (power2bit - 1);
     }

     return bufsize;
}


int main(int argc, char *argv[])
{
   int num_errors = 0;

   try
   {
     Context context (CL_DEVICE_TYPE_ACCELERATOR);

     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     int d = 0;
     std::string str;
     devices[d].getInfo(CL_DEVICE_NAME, &str);
     cout << "DEVICE: " << str << endl << endl;

     // Query OpenCL runtime how much memory and extended memory there is
     cl_ulong gmem_size = 0;
     cl_ulong gmem_size_ext1 = 0;
     cl_ulong gmem_size_ext2 = 0;
     devices[d].getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &gmem_size);
     try
     {
        devices[d].getInfo(CL_DEVICE_GLOBAL_EXT1_MEM_SIZE_TI, &gmem_size_ext1);
        devices[d].getInfo(CL_DEVICE_GLOBAL_EXT2_MEM_SIZE_TI, &gmem_size_ext2);
     } catch (Error& memsz_err)
     { /* Ext Mem Size not available for this device */ }

     cl_uint bufsize   = get_bufsize(gmem_size, gmem_size_ext1, gmem_size_ext2);
     if (bufsize <= 0)  return -1;
     cl_uint CHUNKSIZE = MaxCHUNKSIZE;
     if (CHUNKSIZE > bufsize) CHUNKSIZE = bufsize;
     printf("Bufsize: 0x%x bytes, CHUNKSIZE: 0x%x bytes\n", bufsize, CHUNKSIZE);

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

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

     Kernel kernel(program, "vadd_wrapper");

     Event ev1,ev2,ev3,ev4, ev6,ev7,ev8;

     CommandQueue Q(context, devices[d], CL_QUEUE_PROFILING_ENABLE);

    for (int chunk = 0; chunk * CHUNKSIZE < bufsize; chunk++)
    {
        cl_uint remain_size = bufsize - chunk * CHUNKSIZE;
        if (remain_size > CHUNKSIZE) remain_size = CHUNKSIZE;
        cl_uint remain_elements = remain_size / sizeof(float);
        //cl_uint remain_vec_elements = remain_elements / VectorElements;
        cl_uint chunk_element_start =  chunk * CHUNKSIZE / sizeof(float);

        printf("Chunk %d: float elements 0x%x to 0x%x\n", chunk,
               chunk_element_start, chunk_element_start + remain_elements);

        float * srcA = (float*) Q.enqueueMapBuffer(bufA, CL_TRUE, CL_MAP_WRITE,
                                   chunk * CHUNKSIZE, remain_size, NULL, &ev1);
        float * srcB = (float*) Q.enqueueMapBuffer(bufB, CL_TRUE, CL_MAP_WRITE,
                                   chunk * CHUNKSIZE, remain_size, NULL, &ev2);

        for (cl_uint i=0; i < remain_elements; ++i)
        {
            srcA[i] = srcB[i] = (((chunk * CHUNKSIZE/2) +(i+1)) % 12345)*0.3;
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
        kernel.setArg(3, remain_elements);

        std::vector<Event> vec_ev5(1);
        //Q.enqueueNDRangeKernel(kernel,NullRange,NDRange(remain_vec_elements),
        //                       NDRange(WorkGroupSize), NULL, &vec_ev5[0]);
        Q.enqueueTask(kernel, NULL, &vec_ev5[0]);

        ev3.wait();  // otherwise, we may run short of host address space
        float * dst = (float*)Q.enqueueMapBuffer(bufDst, CL_TRUE, CL_MAP_READ,
                               chunk * CHUNKSIZE, remain_size, &vec_ev5, &ev6);

        for (cl_uint i=0; i < remain_elements; ++i)
        {
          float da = (((chunk * CHUNKSIZE/2)+i+1) % 12345)*0.3;
          if (da+da - dst[i] < -EPISILON || da+da - dst[i] > EPISILON)
          {
    	  cout << "Element " << i << ": "
    	       << da+da << " <==> " << dst[i] << endl;
              num_errors += 1;
              break;
          }
        }

        Q.enqueueUnmapMemObject(bufDst, dst, NULL, &ev7);
        ev7.wait();  // wait to get profiling info

        ocl_event_times(ev1, "Map   BufA  ");
        ocl_event_times(ev2, "Map   BufB  ");
        ocl_event_times(ev3, "UnMap BufA  ");
        ocl_event_times(ev4, "UnMap BufB  ");
        ocl_event_times(vec_ev5[0], "Kernel      ");
        ocl_event_times(ev6, "Map   BufDst");
        ocl_event_times(ev7, "UnMap BufDst");
    }

   }
   catch (Error& err)
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (num_errors == 0) cout << "Success!" << endl;
   else { cout << "Results failed to verify!" << endl; return -1; }
}

