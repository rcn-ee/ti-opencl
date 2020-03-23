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
#include "ocl_util.h"
#include "kernels.dsp_h"

using namespace cl;
using namespace std;

/*-----------------------------------------------------------------------------
* This example demonstrates how a heap may be created and used on the DSP
* for kernels that call legacy code that needs heap capability.  There are dsp
* builtin functions to create and manipulate a user defined heap in both msmc
* and ddr. These heaps are persistent as long as the underlying memory for
* them is allocated.  In this example we create OpenCL buffers that provide
* for the underlying memory store.  The heaps are active and persistent from
* the time they are initialized until the buffers are deallocated.
*
* Additionally, the standard malloc, calloc, free, etc calls are already
* supported on the dsp, but the underlying memory for that heap is limited.
* It currently is approximately 8MB.  If your heap needs are under that size,
* and DDR allocation is ok for you, then the below mechanism is not needed.
*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
   try
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR);
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     devices.resize(1); // Only run on one device for demonstration

     /*------------------------------------------------------------------------
     * OpenCL Build the precompiled kernels
     *-----------------------------------------------------------------------*/
     Program::Binaries binary(1, make_pair(kernels_dsp_bin,sizeof(kernels_dsp_bin)));
     Program           program = Program(context, devices, binary);
     program.build(devices);

     /*------------------------------------------------------------------------
     * Create a command queue and kernelfunctors for all kernels in our program
     *-----------------------------------------------------------------------*/
     CommandQueue Q(context, devices[0]);
     KernelFunctor heap_init_ddr  = Kernel(program, "heap_init_ddr") .bind(Q, NDRange(1), NDRange(1));
     KernelFunctor heap_init_msmc = Kernel(program, "heap_init_msmc").bind(Q, NDRange(1), NDRange(1));
     KernelFunctor alloc_and_free = Kernel(program, "alloc_and_free").bind(Q, NDRange(8), NDRange(1));
     KernelFunctor alloc_only     = Kernel(program, "alloc_only")    .bind(Q, NDRange(8), NDRange(1));

     /*------------------------------------------------------------------------
     * Create the underlying memory store for the heaps with OpenCL Buffers
     * Call kernels to initialize a DDR based and a MSMC based heap, the init
     * step only needs to run once and one 1 core only.  See the functor
     * mapping above that defines the global size to be 1.
     *-----------------------------------------------------------------------*/
     int ddr_heap_size  = 16 << 20;  // 16MB
     int ddr_alloc_size = 1024;
     cout << "[host  ] DDR  heap size " << (ddr_heap_size/1024) << "k" << endl;
     Buffer HeapDDR (context, CL_MEM_READ_WRITE, ddr_heap_size);
     heap_init_ddr (HeapDDR,  ddr_heap_size) .wait();
     cout << "[host  ] DDR  heap init'ed" << endl;

     int msmc_heap_size     = 0;
     int msmc_alloc_size    = 0;
     cl_ulong msmc_mem_size = 0;
     devices[0].getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI, &msmc_mem_size);
     if(msmc_mem_size > 0)
     {
         msmc_heap_size  = msmc_mem_size;
         msmc_alloc_size = 1024;
         cout << "[host  ] MSMC heap size " << (msmc_heap_size/1024) << "k" << endl;
         Buffer HeapMSMC(context, CL_MEM_READ_WRITE|CL_MEM_USE_MSMC_TI, msmc_heap_size);
         heap_init_msmc(HeapMSMC, msmc_heap_size).wait();
         cout << "[host  ] MSMC heap init'ed" << endl;
     }
     else
     {
         cout << "[host  ] MSMC unavailable" << endl;
     }

     cout << endl;

     /*------------------------------------------------------------------------
     * On each core alloc memory from both ddr and msmc and the free it.
     *-----------------------------------------------------------------------*/
     alloc_and_free(ddr_alloc_size, msmc_alloc_size).wait();
     cout << endl;

     /*------------------------------------------------------------------------
     * On each core alloc memory from both ddr and msmc. Should see same memory
     * from above alloc_and_free call.  This time the memory is not freed.
     *-----------------------------------------------------------------------*/
     alloc_only(ddr_alloc_size, msmc_alloc_size).wait();
     cout << endl;

     /*------------------------------------------------------------------------
     * Again, alloc on each core. Since the previous call did not free, these
     * allocations should be in separate memory from the last set.
     *-----------------------------------------------------------------------*/
     alloc_only(ddr_alloc_size, msmc_alloc_size).wait();
     cout << endl;
   }
   catch (Error& err)
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }
}
