#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <utility>
#include "ocl_util.h"

#define __CL_ENABLE_EXCEPTIONS
#include <CL/TI/cl.hpp>
/******************************************************************************
* C[N][M] = A[N][K] * B[K][M];
******************************************************************************/
using namespace cl;

using std::cout;
using std::cerr;
using std::endl;

const int DIM       = 16;
const int mat_N     = DIM;     
const int mat_K     = DIM;     
const int mat_M     = DIM;     

const std::string kernelSrc = R"(
kernel void ocl_matmpy(const global float *a, 
		       const global float *b, 
		             global float *c, 
		                    int    mat_K,
                                    int    mat_N)
{
    int col    = get_global_id(0);
    int mat_M  = get_global_size(0);

    for (int row = 0; row < mat_N; ++row)
    {
        c[row * mat_M + col] = 0;
        for (int i = 0; i < mat_K; ++i)
            c[row * mat_M + col] += a[row*mat_K+i] * b[i*mat_M+col];
    }
}
)";


void mat_mpy_ocl(float *A, float *B, float *C, int mat_N, 
                 int mat_K, int mat_M, std::size_t mat_size)
{
   try 
   {
     // Initialize context and command queue
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     CommandQueue        Q (context, devices[0]);

     // Build the OpenCL program
     Program::Sources    source(1, std::make_pair(kernelSrc.c_str(), 
                                                  kernelSrc.length()));
     Program             P = Program(context, source);
     P.build(devices); 

     // Create buffers from memory allocated via __malloc_ddr
     Buffer bufA(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,   mat_size, A);
     Buffer bufB(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,   mat_size, B);
     Buffer bufC(context, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR,  mat_size, C);

     // Create kernel and set up arguments
     Kernel        K (P, "ocl_matmpy");
     K.setArg(0, bufA);
     K.setArg(1, bufB);
     K.setArg(2, bufC);
     K.setArg(3, mat_K);
     K.setArg(4, mat_N);

     // Run the kernel and wait for completion
     Event E;
     Q.enqueueNDRangeKernel(K, NullRange, NDRange(mat_M), NDRange(1), NULL, &E);
     E.wait();
   }
   catch (Error err) 
   {
     cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
          << ocl_decode_error(err.err()) << ")" << endl;
     exit(-1);
   }


}

int main(int argc, char *argv[])
{
   std::size_t mat_size = DIM * DIM * sizeof(float);

   // Allocate matrices
   float *A      = (float *) __malloc_ddr(mat_size);
   float *B      = (float *) __malloc_ddr(mat_size);
   float *C      = (float *) __malloc_ddr(mat_size);

   assert(A != nullptr && B != nullptr && C != nullptr && C != nullptr);

   // Initialize matrices
   srand(42);
   for (int i=0; i < mat_N * mat_K; ++i) A[i] = rand() % 5 + 1;
   for (int i=0; i < mat_K * mat_M; ++i) B[i] = rand() % 5 + 1;
   for (int i=0; i < mat_N * mat_M; ++i) C[i] = 0.0;

   // Multiple matrices C = A x B
   mat_mpy_ocl(A, B, C, mat_N, mat_K, mat_M, mat_size);

   // Free the matrices
   __free_ddr(A);
   __free_ddr(B);
   __free_ddr(C);

   return 0;
}
