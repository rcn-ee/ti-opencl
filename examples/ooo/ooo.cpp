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
#include <CL/TI/cl.hpp>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <signal.h>
#include <memory>
#include "ocl_util.h"

using namespace std;
using namespace cl;

#define PROFILE 0 //CL_QUEUE_PROFILING_ENABLE
#define OOOEXEC CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE

typedef std::vector<Event>                              vecEv;
typedef std::vector<vecEv>                              vecVecEv;
typedef std::vector<vecVecEv>                           vecVecVecEv;
typedef std::pair<void*, ::size_t>                      native_arg_t;
typedef std::unique_ptr<Buffer>                         BufUP;
typedef enum   { PRD, WRT, CMP, RD, CNS, STAGES }       stage;

typedef struct { int *ptr; unsigned elms; int val; int iter; }  arguments_t;

const char *stage_names[] = {"PRODUCE","WRITE  ","COMPUTE","READ   ","CONSUME"};

const char * kernStr = 
"kernel void compute(global int* buf, int size) \n"
"{\n"
"  for (int i = 0; i< size; ++i) \n"
"    buf[i] += 1; \n"
"}\n";

const int tasks    = 2048;
const int inflight = 32;
const int elements = 1 << 13; // 8K
const int size     = sizeof(int) * elements;
int       arys [inflight][elements];
int       incorrect_results = false;

static double clock_diff (struct timespec *t1, struct timespec *t2)
       { return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec) / 1e9; }

/******************************************************************************
* cpu_produce
******************************************************************************/
void cpu_produce(void *args)
{
    arguments_t *p = (arguments_t *)args;
    for (int i = 0; i < p->elms; ++i) 
        p->ptr[i] = p->val;
}

/******************************************************************************
* cpu_consume
******************************************************************************/
void cpu_consume(void *args)
{
    arguments_t *p = (arguments_t *)args;
    int i;

    for (i = 0; i < p->elms; ++i) 
        if (p->ptr[i] != p->val) 
        {
            std::cout << "Iteration " << p->iter << ": "
                      << p->ptr[i] << " != " << p->val 
                      << std::endl << std::endl;
            incorrect_results = true;
            break;
        }
}

/******************************************************************************
* main
******************************************************************************/
int main(int argc, char *argv[])
{
   /*-------------------------------------------------------------------------
   * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
   *------------------------------------------------------------------------*/
   signal(SIGABRT, exit);
   signal(SIGTERM, exit);

   struct timespec tp_start, tp_end;

   try 
   {
     /*------------------------------------------------------------------------
     * One time OpenCL Setup
     *-----------------------------------------------------------------------*/
     Context             context(CL_DEVICE_TYPE_ALL); 
     std::vector<Device> devices(context.getInfo<CL_CONTEXT_DEVICES>());

     CommandQueue        *QcpuIO = NULL;
     CommandQueue        *QcpuOO = NULL;
     CommandQueue        *QdspOO = NULL;

     std::vector<Device> dspDevices;
     for (int d = 0; d < devices.size(); d++)
     {
	cl_device_type type;
	devices[d].getInfo(CL_DEVICE_TYPE, &type);

	if (type == CL_DEVICE_TYPE_CPU)
	{
	   QcpuIO = new CommandQueue(context, devices[d], PROFILE);
	   QcpuOO = new CommandQueue(context, devices[d], PROFILE|OOOEXEC);
	}
	else if (type == CL_DEVICE_TYPE_ACCELERATOR)
        {
	   QdspOO  = new CommandQueue(context, devices[d], PROFILE|OOOEXEC);
           dspDevices.push_back(devices[d]);
        }
     }

     if (QcpuIO == NULL)
     {
	std::cout << 
	"CPU devices are not fully supported in the current" << std::endl <<
	"OpenCL implementation (native kernel support only)." << std::endl << 
	"As a result, CPU devices are not enabled by" << std::endl <<
	"default.  This example uses OpenCL CPU native" << std::endl <<
	"kernels and can be run with the CPU device enabled." << std::endl << 
        "To enable a CPU device define the environment variable" << std::endl <<
        "'TI_OCL_CPU_DEVICE_ENABLE' before running the example." << std::endl;
	 exit(-1);
     }

     assert(QdspOO != NULL);

     Program::Sources    source (1, std::make_pair(kernStr, strlen(kernStr)));
     Program             program(Program(context, source));

     program.build(dspDevices);
     Kernel K(program, "compute");

     /*------------------------------------------------------------------------
     * Define a Buffer for each possible in flight task
     *-----------------------------------------------------------------------*/
     std::vector<BufUP> bufs;
     for (int i = 0; i < inflight; ++i) 
         bufs.push_back(BufUP(new Buffer(context, CL_MEM_READ_WRITE, size)));

     /*------------------------------------------------------------------------
     * Define a 3-D vector of OpenCL Events.  1st dim is for the number of 
     * in flight tasks, the second dim is for the processing stages of a single
     * task.  The 3rd dim is an artifact of the c++ binding for event wait 
     * lists.  All enqueue API's take a wait list which is a vector<Event>*, and
     * they take an Event*.  All events in the wait list vector must complete,
     * before this event will execute.  The single event argument is for the 
     * event that will be set as a result of this enqueue. 
     *-----------------------------------------------------------------------*/
     vecVecVecEv evt(inflight, vecVecEv(STAGES, vecEv(1)));

     /*------------------------------------------------------------------------
     * Enqueue a dummy DSP kernel call to force the OpenCL lazy execution
     * to go ahead and compile the kernel and load it.  This will prevent the 
     * compile and load times from skewing the reported numbers.  This is not 
     * needed by the algorithm and is purely a tactic to get consistent numbers
     * from the the running of the bulk of this algorithm
     *-----------------------------------------------------------------------*/
     K.setArg(0, *bufs[0]);
     K.setArg(1, 0);
     QdspOO->enqueueTask(K);

     K.setArg(1, elements);
     clock_gettime(CLOCK_MONOTONIC, &tp_start);

     /*------------------------------------------------------------------------
     * Iterate for as many tasks as there are
     *-----------------------------------------------------------------------*/
     for (int i = 0; i < tasks; ++i)
     {
        /*---------------------------------------------------------------------
        * Variables to ensure that this iteration is using the correct circular
        * resources: i.e. buffers and arrays.
        *--------------------------------------------------------------------*/
        int     circIdx = i % inflight;
        Buffer &buf(*bufs[circIdx]);
        int    *ary(arys [circIdx]);
        Event  nullEv;

        /*---------------------------------------------------------------------
        * Native kernels are only passed a single pointer, so define a structure
        * that contains the actual arguments, populate that and then create 
        * a C++ binding native argument class that has the pointer and a size.
        *--------------------------------------------------------------------*/
        arguments_t proArgs = { ary, elements, i,   i };
        arguments_t conArgs = { ary, elements, i+1, i };
        native_arg_t proNargs(&proArgs, sizeof(proArgs));
        native_arg_t conNargs(&conArgs, sizeof(conArgs));

        K.setArg(0, buf);

        /*---------------------------------------------------------------------
        * Since we are reusing N sets of buffers in this loop, we need to make
        * sure than iteration I does not start until after iteration I-N 
        * completes. Iterations < N can start immediately.
        *--------------------------------------------------------------------*/
        vecEv *start_waits = (i < inflight) ? 0 : &evt[circIdx][CNS];

        evt[circIdx][PRD][0] = nullEv;
        evt[circIdx][WRT][0] = nullEv;
        evt[circIdx][CMP][0] = nullEv;
        evt[circIdx][RD ][0] = nullEv;

        QcpuOO->enqueueNativeKernel(cpu_produce, proNargs, 0, 0,  
                start_waits,        &evt[circIdx][PRD][0]);

        evt[circIdx][CNS][0] = nullEv;

        QdspOO->enqueueWriteBuffer (buf, CL_FALSE, 0, size, ary,  
                &evt[circIdx][PRD], &evt[circIdx][WRT][0]);
        QdspOO->enqueueTask        (K,                            
                &evt[circIdx][WRT], &evt[circIdx][CMP][0]);
        QdspOO->enqueueReadBuffer  (buf, CL_FALSE, 0, size, ary,  
                &evt[circIdx][CMP], &evt[circIdx][RD ][0]);
        QcpuIO->enqueueNativeKernel(cpu_consume, conNargs, 0, 0,  
                &evt[circIdx][RD ], &evt[circIdx][CNS][0]);
     }

     /*------------------------------------------------------------------------
     * Only need to wait for the CPU In Order queue to finish, since all all
     * other enqueue events must finish before the CPU IO queue can finish
     *-----------------------------------------------------------------------*/
     QcpuIO->finish();

     delete QcpuIO;
     delete QcpuOO;
     delete QdspOO;

     clock_gettime(CLOCK_MONOTONIC, &tp_end);
     double elapsed = clock_diff (&tp_start, &tp_end);
     printf("Elapsed : %8.6f secs\n", elapsed);

     /*------------------------------------------------------------------------
     * After the running is complete, report timing for each step
     *-----------------------------------------------------------------------*/
#if PROFILE 
     cl_ulong ref;
     evt[0][0][0].getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &ref);

     for (int i = 0; i < inflight; ++i)
     {
          for (int s = 0; s < STAGES; ++s)
              ocl_relative_times(evt[i][s][0], stage_names[s], ref);
          cout << endl;
     }
#endif
   }

   catch (Error err)
   {
       cerr << "ERROR: " << err.what() << "("
            << ocl_decode_error(err.err()) << ")"
            << endl;
       incorrect_results = true;
   }

   if (incorrect_results) return -1;
}
