#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include "ocl_util.h"
#include "ocl.h"
#include "kernel.dsp_h"

using namespace cl;
using namespace std;

ocl_t ocl ={0,0,0,0,0};

extern "C" DLL_PUBLIC
void ocl_init()
{
   try 
   {
     ocl.context = new Context(CL_DEVICE_TYPE_ACCELERATOR);

     std::vector<Device> devices = ocl.context->getInfo<CL_CONTEXT_DEVICES>();

     Program::Binaries binary(1, make_pair(kernel_dsp_bin,sizeof(kernel_dsp_bin)));
     ocl.program = new Program(*(ocl.context), devices, binary);
     ocl.program->build(devices);

     /*------------------------------------------------------------------------
     * Create two queues for use. Api can determine which to use
     *-----------------------------------------------------------------------*/
     ocl.queueInOrder    = new CommandQueue(*(ocl.context), devices[0]);
     ocl.queueOutOfOrder = new CommandQueue(*(ocl.context), devices[0], 
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

     ocl.K_cblas_dgemm = new Kernel(*(ocl.program), "K_cblas_dgemm_omp");

     /*------------------------------------------------------------------------
     * Dispatch a null kernel to force load of the OpenCL C program
     *-----------------------------------------------------------------------*/
     KernelFunctor null = Kernel(*(ocl.program), "null")
                           .bind(*(ocl.queueInOrder), NDRange(1), NDRange(1));
     null().wait();
   }
   catch (Error err)
   {cerr<<"ERROR: "<<err.what()<<endl;}
}

extern "C" DLL_PUBLIC
void ocl_free()
{
     if (ocl.K_cblas_dgemm)   delete ocl.K_cblas_dgemm;
     if (ocl.program)         delete ocl.program;
     if (ocl.queueInOrder)    delete ocl.queueInOrder;
     if (ocl.queueOutOfOrder) delete ocl.queueOutOfOrder;
     if (ocl.context)         delete ocl.context;

     ocl.K_cblas_dgemm  =0;
     ocl.program        =0;
     ocl.queueInOrder   =0;
     ocl.queueOutOfOrder=0;
     ocl.context        =0;
}
