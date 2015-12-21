#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include "ocl_util.h"
#include "ocl.h"
#include "kernel.dsp_h"

using namespace cl;
using namespace std;

ocl_t ocl = {0,0,0,0,0};
dgemm_params_t dparams = {0, 0, 0, 0, 0};

static bool SetDgemmParams(Device& device, bool calc_check);

extern "C" DLL_PUBLIC
void ocl_init(bool calc_check, int *NUMCOMPUNITS)
{
   try 
   {
     ocl.context = new Context(CL_DEVICE_TYPE_ACCELERATOR);

     std::vector<Device> devices = ocl.context->getInfo<CL_CONTEXT_DEVICES>();
     SetDgemmParams(devices[0], calc_check);
     if (NUMCOMPUNITS != NULL)  *NUMCOMPUNITS = dparams.NUMCOMPUNITS;

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

static ulong roundDownPower2(ulong value)
{ return (value == 0) ? 0 :  1 << ilogb(value); }

/*-----------------------------------------------------------------------------
* Check platform name, set dgemm blocking/tiling parameters accordingly
*----------------------------------------------------------------------------*/
bool SetDgemmParams(Device& device, bool calc_check)
{
   int APanelSz        = 16 << 10;
   int BPanelSz        = 16 << 10;
   cl_ulong global_mem = 0;
   cl_ulong l2_mem     = 0;
   cl_ulong msmc_mem   = 0;

   device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &dparams.NUMCOMPUNITS);
   device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE,   &global_mem);
   device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE,    &l2_mem);

#ifdef CL_DEVICE_MSMC_MEM_SIZE_TI
   device.getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI,  &msmc_mem);
#endif

   global_mem            = roundDownPower2(global_mem);
   dparams.L2_BUF_SIZE   = roundDownPower2(l2_mem);
   dparams.MSMC_BUF_SIZE = roundDownPower2(msmc_mem);

   dparams.NUMAPANELS    = dparams.L2_BUF_SIZE / 2 / APanelSz;
   dparams.NUMBPANELS    = dparams.L2_BUF_SIZE / 4 / BPanelSz;

   if ((dparams.NUMCOMPUNITS * APanelSz * dparams.NUMAPANELS) >
                                                         dparams.MSMC_BUF_SIZE)
        dparams.MSMC_BUF_SIZE = 0;
   else dparams.MSMC_BUF_SIZE = dparams.NUMCOMPUNITS * APanelSz
                                                     * dparams.NUMAPANELS;

   if (calc_check)
   {
       cout << "MSMC_BUF_SIZE = " << dparams.MSMC_BUF_SIZE << endl;
       cout << "L2_BUF_SIZE   = " << dparams.L2_BUF_SIZE << endl;
       cout << "NUMAPANELS    = " << dparams.NUMAPANELS << endl;
       cout << "NUMBPANELS    = " << dparams.NUMBPANELS << endl;
   }
}
