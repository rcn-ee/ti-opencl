#ifndef _OCL_H_ 
#define _OCL_H_ 
#include <CL/cl.hpp>

#define DLL_PUBLIC __attribute__ ((visibility ("default")))

typedef struct
{
    cl::Context       *context;
    cl::CommandQueue  *queueInOrder;
    cl::CommandQueue  *queueOutOfOrder;
    cl::Program       *program;
    cl::Kernel        *K_cblas_dgemm;
} ocl_t;

typedef struct
{
    int L2_BUF_SIZE;
    int MSMC_BUF_SIZE;
    int NUMAPANELS;
    int NUMBPANELS;
    int NUMCOMPUNITS;
} dgemm_params_t;

extern ocl_t ocl;
extern dgemm_params_t dparams;

extern "C" void DLL_PUBLIC ocl_init(bool, int *);
extern "C" void DLL_PUBLIC ocl_free();

#endif // _OCL_H_ 
