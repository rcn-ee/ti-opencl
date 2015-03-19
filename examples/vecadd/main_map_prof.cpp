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

/******************************************************************************
 * This vecadd_md example computes on all available ACCELERATOR devices,
 * while vecadd example only computes on the first ACCELERATOR device.
 *****************************************************************************/
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "ocl_util.h"

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

int gettime_ms()
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
        clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_nsec / 1000 + tp.tv_sec * 1000000;
}

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int num_errors = 0;
   int          d = 0;
   int start_time, end_time;
   int num_devices = atoi(argv[1]);
   int numK_start = atoi(argv[2]);
   int numK_end = atoi(argv[3]);

   int32_t  profMatrix[20][50];
   int32_t  k =0;
   int prof_index;

  for (int numK=numK_start; numK<=numK_end; numK=numK*2)
  {

   cout << "numK in the loop is " << numK << endl;
   int NumElements = numK*1024;
   int NumWorkGroups   = 1;
   int VectorElements  = 4;
   int NumVecElements  = NumElements / VectorElements;
   int WorkGroupSize   = NumVecElements / NumWorkGroups;
   int bufsize=NumElements * sizeof(cl_short);

   index_buffer_alloc=0;
   index_write_core=0;
   index_read_core=0;
   index_msg_send2rcv=0;
   index_map_wb=0;
   index_map_inv=0;

   int *dspKernelTime;
   int *dspCacheTime;
   try 
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
//     int num_devices = devices.size();
     if (NumVecElements % num_devices != 0 || 
         (NumVecElements / num_devices) % WorkGroupSize != 0)
     {
        cerr << "ERROR: Cannot evenly distribute data across devices!" << endl;
        exit(-1);
     }
     int d_bufsize = bufsize / num_devices;
     int d_Elements = NumElements / num_devices;
     int d_VecElements = NumVecElements / num_devices;

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices); 
     Kernel kernel(program, "VectorAdd");

     for (d = 0; d < num_devices; ++d)
     {
        std::string str;
        devices[d].getInfo(CL_DEVICE_NAME, &str);
        cout << "DEVICE " << d << ": " << str << endl << endl;
     }

     cout << "\n\n=== Using MapBuffer/UnmapBuffer APIs ===" << endl; 
     cl_short *h_bufA, *h_bufB;
     cl_short *mGolden = (cl_short *) malloc(bufsize);
     cl_short **h_dst = (cl_short **) malloc(num_devices * sizeof(cl_short *));
     if (!mGolden || !h_dst)
     {
        cout << "Unable to allocate memory for data! (MapBuffer API)" << endl;
        exit(-1);
     }
     
     start_time = gettime_ms();
     std::vector<Buffer*> mbufAs, mbufBs, mbufDs;
     std::vector<CommandQueue*> mQs;
     std::vector<Event*> mev1s, mev2s, mev3s, mev4s, mev5s, mev6s, mev7s;
     for (d = 0; d < num_devices; ++d)
     {
        mbufAs.push_back(new Buffer(context, CL_MEM_READ_ONLY, d_bufsize));
        mbufBs.push_back(new Buffer(context, CL_MEM_READ_ONLY, d_bufsize));
        mbufDs.push_back(new Buffer(context, CL_MEM_WRITE_ONLY, d_bufsize));
        mQs.push_back(new CommandQueue(context, devices[d],
                                      CL_QUEUE_PROFILING_ENABLE));
        mev1s.push_back(new Event());
        mev2s.push_back(new Event());
        mev3s.push_back(new Event());
        mev4s.push_back(new Event());
        mev5s.push_back(new Event());
        mev6s.push_back(new Event());
        mev7s.push_back(new Event());
     }
     for (d = 0; d < num_devices; ++d)
     {
        cl_short *h_bufA = (cl_short *) mQs[d]->enqueueMapBuffer(*mbufAs[d], 
                         CL_FALSE, CL_MAP_WRITE, 0, d_bufsize, NULL, mev1s[d]);
        cl_short *h_bufB = (cl_short *) mQs[d]->enqueueMapBuffer(*mbufBs[d], 
                         CL_FALSE, CL_MAP_WRITE, 0, d_bufsize, NULL, mev2s[d]);
        mev1s[d]->wait();
        for (int i = 0; i < d_Elements; i++)
          h_bufA[i] = (d * d_Elements + i)<<2;
        mQs[d]->enqueueUnmapMemObject(*mbufAs[d], h_bufA, NULL, mev3s[d]);
        mev2s[d]->wait();
        for (int i = 0; i < d_Elements; i++)
        {
          h_bufB[i] = (d * d_Elements + i)<<2;
          mGolden[d*d_Elements + i] = h_bufA[i] + h_bufB[i];
        }
        mQs[d]->enqueueUnmapMemObject(*mbufBs[d], h_bufB, NULL, mev4s[d]);
     }
     for (d = 0; d < num_devices; ++d)
     {
        kernel.setArg(0, *mbufAs[d]);
        kernel.setArg(1, *mbufBs[d]);
        kernel.setArg(2, *mbufDs[d]);
        mev3s[d]->wait();
        mQs[d]->enqueueNDRangeKernel(kernel, NullRange, NDRange(d_VecElements),
                                    NDRange(WorkGroupSize), NULL, mev5s[d]);
        h_dst[d] = (cl_short *) mQs[d]->enqueueMapBuffer(*mbufDs[d], CL_FALSE,
                                    CL_MAP_READ, 0, d_bufsize, NULL, mev6s[d]);
     }
     num_errors = 0;
     for (d = 0; d < num_devices; ++d)
     {
        mev6s[d]->wait();
        dspKernelTime = (int*)&h_dst[d][0];
        dspCacheTime = (int*)&h_dst[d][2];
        for (int i = 4; i < d_Elements; ++i)
          if (mGolden[d*d_Elements + i] != h_dst[d][i]) 
          { 
            num_errors += 1;
            if (num_errors < 10)
                cout << "Failed at Element " << i << ": " 
                 << mGolden[d*d_Elements + i] << " != " << h_dst[d][i] << endl;
          }
        mQs[d]->enqueueUnmapMemObject(*mbufDs[d], h_dst[d], NULL, mev7s[d]);
     }

     for (d = 0; d < num_devices; ++d)
        mev7s[d]->wait();
     end_time = gettime_ms();

     cout << end_time - start_time << " micro seconds" << endl;

     for (d = 0; d < num_devices; ++d)
     {
        cout << "DEVICE " << d << ":" << endl;
        ocl_event_times(*mev1s[d], "Map   BufA ");
        ocl_event_times(*mev2s[d], "Map   BufB ");
        ocl_event_times(*mev3s[d], "Unmap BufA" );
        ocl_event_times(*mev4s[d], "Unmap BufB" );
        ocl_event_times(*mev5s[d], "Kernel Exec");
        ocl_event_times(*mev6s[d], "Map   BufDst");
        ocl_event_times(*mev7s[d], "Unmap BufDst");
     }
     if (num_errors)  cout << "Fail with " << num_errors << " errors!" << endl;
     else             cout << "Success!" << endl; 

     free(mGolden);
     free(h_dst);
     for (d = 0; d < num_devices; ++d)
     {
         delete mbufAs[d];
         delete mbufBs[d];
         delete mbufDs[d];
         delete mQs[d];
         delete mev1s[d];
         delete mev2s[d];
         delete mev3s[d];
         delete mev4s[d];
         delete mev5s[d];
         delete mev6s[d];
         delete mev7s[d];
     }
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   prof_index=0;
   int time_addup=0;
   int time_vecadd=0;
   int i;

   profMatrix[k][prof_index++] = bufsize;

   for (i=0; i<index_buffer_alloc; i++) {
     profMatrix[k][prof_index++] = time_buffer_alloc[i];
     time_addup += time_buffer_alloc[i];
   }

   for(i=0;i<index_map_wb-1;i++)
   {
     profMatrix[k][prof_index++] = time_map_wb[i];
     time_addup += time_map_wb[i];
     time_vecadd += time_map_wb[i];
   }

   int time_kernel = 0;   
   for (int i=0; i<num_devices; i++ )
   {    
     int dsp_freq = 600;
     profMatrix[k][prof_index++] =  time_msg_send2rcv[i];

     time_kernel+= time_msg_send2rcv[i];

     profMatrix[k][prof_index++] = *dspKernelTime/dsp_freq;
     profMatrix[k][prof_index++] = *dspCacheTime/dsp_freq;

     int latency=time_msg_send2rcv[i] - *dspKernelTime/dsp_freq - *dspCacheTime/dsp_freq;
     profMatrix[k][prof_index++] =  latency;
   }
   
   time_addup += time_kernel;   
   time_vecadd += time_kernel;

   for(i=0;i<index_map_inv;i++)
   {
     profMatrix[k][prof_index++] = time_map_inv[i];
     time_addup += time_map_inv[i];
     time_vecadd += time_map_inv[i];
   }

   profMatrix[k][prof_index++] = time_vecadd;

   k++;
 }
   char fileName[20];
   sprintf(fileName, "Prof_AM572x_MAP.csv");
   ofstream file(fileName, std::ios_base::app);

   file << "vecadd Profiling (MAP DSP buffers) on AM572x (DSP runs @600MHz & ARM runs @1.0GHz)" << endl;
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
       file << "Cache WB Src, ";
       break;

       case 5:
       file << "Cache WB Src, ";
       break;

       case 6:
       file << "Msg Send to Rcv,";
       break;

       case 7:
       file << "     DSP Kernel Process, ";
       break;

       case 8:
       file << "     DSP Cache Inv, ";
       break;

       case 9:
       file << "     Host-DSP Msg Latency, ";
       break;

       case 10:
       file << "Cache Inv Dest D, ";
       break;

       case 11:
       file << "OpenCL vecadd time (Cache+Kernel), ";
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
