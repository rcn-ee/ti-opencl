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
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "ocl_util.h"
#include <time.h>
#include <sys/time.h>

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

#define PROF_SIZE 10
int32_t max_prof_size=PROF_SIZE;
int32_t time_buffer_alloc[PROF_SIZE];
int32_t index_buffer_alloc;

int32_t index_write_core;
int32_t time_write_core_memcpy[PROF_SIZE];
int32_t time_write_core_wb[PROF_SIZE];

int32_t index_read_core;
int32_t time_read_core_memcpy[PROF_SIZE];
int32_t time_read_core_inv[PROF_SIZE];

int32_t index_msg_send2rcv;
int32_t time_msg_send2rcv[PROF_SIZE];

int32_t index_map_wb;
int32_t time_map_wb[PROF_SIZE];

int32_t index_map_inv;
int32_t time_map_inv[PROF_SIZE];

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int num_devices = atoi(argv[1]);
   int numK_start = atoi(argv[2]);
   int numK_end = atoi(argv[3]);
   struct timeval start, end;
   int32_t  profMatrix[20][50];
   int32_t  k =0;
   int prof_index;

  for (int numK=numK_start; numK<=numK_end; numK=numK*2)
  {

   cout << "numK in the loop is " << numK << endl;
   int NumElements = numK*1024;
   //int NumWorkGroups   = 256;
   int NumWorkGroups   = 1;
   int VectorElements  = 4;
   int NumVecElements  = NumElements / VectorElements;
   int WorkGroupSize   = NumVecElements / NumWorkGroups;
   int bufsize=NumElements * sizeof(cl_short);

   cl_short *srcA = (cl_short*) malloc(bufsize);
   cl_short *srcB = (cl_short*) malloc(bufsize);
   cl_short *dst = (cl_short*) malloc(bufsize);
   cl_short *dst_arm = (cl_short*) malloc(bufsize);
   cl_short *Golden = (cl_short*) malloc(bufsize);

   index_buffer_alloc=0;
   index_write_core=0;
   index_read_core=0;
   index_msg_send2rcv=0;

   for (int i=0; i < NumElements; ++i) 
   { 
       srcA[i]   = srcB[i] = i<<2; 
       Golden[i] = srcB[i] + srcA[i]; 
       dst[i]    = 0;
   }

   try 
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
      
     int d = 0;
     std::string str;
     devices[d].getInfo(CL_DEVICE_NAME, &str);
     cout << "DEVICE: " << str << endl << endl;

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices); 

     gettimeofday(&start, NULL);

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     Kernel kernel(program, "VectorAdd");
     kernel.setArg(0, bufA);
     kernel.setArg(1, bufB);
     kernel.setArg(2, bufDst);

     Event ev1,ev2,ev3,ev4;

     CommandQueue Q(context, devices[d], CL_QUEUE_PROFILING_ENABLE);

     Q.enqueueWriteBuffer(bufA, CL_FALSE, 0, bufsize, srcA, NULL, &ev1);
     Q.enqueueWriteBuffer(bufB, CL_FALSE, 0, bufsize, srcB, NULL, &ev2);
     Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumVecElements), 
                            NDRange(WorkGroupSize), NULL, &ev3);
     Q.enqueueReadBuffer (bufDst, CL_TRUE, 0, bufsize, dst, NULL, &ev4);

     gettimeofday(&end, NULL);

     ocl_event_times(ev1, "Write BufA ");
     ocl_event_times(ev2, "Write BufB ");
     ocl_event_times(ev3, "Kernel Exec");
     ocl_event_times(ev4, "Read BufDst");
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   for (int i=4; i < NumElements; ++i)
       if (Golden[i] != dst[i]) 
       { 
           cout << "Failed at Element " << i << ": " 
                << Golden[i] << " != " << dst[i] << endl; 
           return 1;
       }

   cout << "Success!" << endl; 

   prof_index=0;
   int time_addup=0;
   int time_vecadd=0;
   int i;

   profMatrix[k][prof_index++] = bufsize;

   for (i=0; i<index_buffer_alloc; i++) {
     profMatrix[k][prof_index++] = time_buffer_alloc[i];
     time_addup += time_buffer_alloc[i];
   }

    
   for (i=0; i<index_write_core; i++) {
     profMatrix[k][prof_index++] = time_write_core_memcpy[i];
     time_addup += time_write_core_memcpy[i];
   }

   for (i=0; i<index_write_core; i++) {
     profMatrix[k][prof_index++] =  time_write_core_wb[i];
     time_addup += time_write_core_wb[i];
     time_vecadd+=time_write_core_wb[i];
   }

   int time_kernel = 0;
   for (int i=0; i<num_devices; i++ )
   {
     int *dspKernelProcessTsclPtr = (int *)&dst[0+i*(NumElements/2)];
     int *dspCacheInvTsclPtr = (int *)&dst[2+i*(NumElements/2)];
     int dsp_freq = 600;

     profMatrix[k][prof_index++] =  time_msg_send2rcv[i];
     time_kernel+= time_msg_send2rcv[i];
     profMatrix[k][prof_index++] =  *dspKernelProcessTsclPtr/dsp_freq;
     profMatrix[k][prof_index++] =  *dspCacheInvTsclPtr/dsp_freq;

     int latency=time_msg_send2rcv[i] - *dspKernelProcessTsclPtr/dsp_freq - *dspCacheInvTsclPtr/dsp_freq;

     profMatrix[k][prof_index++] =  latency;
   }
   
   time_addup += time_kernel;   
   time_vecadd += time_kernel;

   for (i=0; i<index_read_core; i++) {
     profMatrix[k][prof_index++] =  time_read_core_inv[i];
     time_addup += time_read_core_inv[i];
     time_vecadd+= time_read_core_inv[i];
   }

   for (i=0; i<index_read_core; i++) {
     profMatrix[k][prof_index++] = time_read_core_memcpy[i];
     time_addup += time_read_core_memcpy[i];
   }

   profMatrix[k][prof_index++] =  time_addup;

   profMatrix[k][prof_index++] =   ((end.tv_sec * 1000000 + end.tv_usec)
                - (start.tv_sec * 1000000 + start.tv_usec));

   profMatrix[k][prof_index++] = time_vecadd;


   gettimeofday(&start, NULL);   
   for (int i=0; i < NumElements; ++i)
   {
       dst_arm[i] = srcA[i]+srcB[i];
   }
   gettimeofday(&end, NULL);

   profMatrix[k][prof_index++] =  ((end.tv_sec * 1000000 + end.tv_usec)
               - (start.tv_sec * 1000000 + start.tv_usec));

   k++;
 }
   char fileName[30];
   sprintf(fileName, "Prof_AM572x_ReadWrite.csv");
   ofstream file(fileName, std::ios_base::app);
   file << "vecadd Profiling (Read/Write DSP buffers) on AM572X (DSP runs @600MHz & ARM runs @1.0GHz)" << endl;
   file << "units of time: micro seconds; data type is cl_short " << endl;
   for (int j=0; j<prof_index; j++)
   {     
     switch (j) { 
       case 0:
       file << "BufSize (bytes),";
       break;

       case 1:
       file << "CMEM Alloc A, ";
       break;

       case 2:
       file << "CMEM Alloc B, ";
       break;

       case 3:
       file << "CMEM Alloc D, ";
       break;

       case 4:
       file << "Write Src, ";
       break;

       case 5:
       file << "Write Src, ";
       break;

       case 6:
       file << "Write Src, ";
       break;

       case 7:
       file << "Cache WB Src, ";
       break;

       case 8:
       file << "Cache WB Src, ";
       break;

       case 9:
       file << "Cache WB Src, ";
       break;

       case 10:
       file << "Msg Send to Rcv,";
       break;

       case 11:
       file << "     DSP Kernel Process, ";
       break;

       case 12:
       file << "     DSP Cache Inv, ";
       break;

       case 13:
       file << "     Host-DSP Msg Latency, ";
       break;

       case 14:
       file << "Cache Inv Dest D, ";
       break;

       case 15:
       file << "Read Dest D, ";
       break;

       case 16:
       file << "OpenCL time addup (row 4~13 & row 17;18), ";
       break;

       case 17:
       file << "OpenCL total time measured in unit test, ";
       break;

       case 18:
       file << "OpenCL vecadd time (Cache+Kernel), ";
       break;

       case 19:
       file << "ARM vecadd time, ";
       break;
     }
     for (int i=0; i<k; i++)
     {
       file << profMatrix[i][j] << ",";
     }
     file << endl;
   }
   file.close();
}
