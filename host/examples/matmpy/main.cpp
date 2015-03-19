#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>
#include <omp.h>

/******************************************************************************
* C[N][M] = A[N][K] * B[K][M];
******************************************************************************/

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

void report_event_timing(const Event &ev, const char* name);

const char * kernelStr =
    "kernel void Simple_MatMpy(global const float* a, "
    "                          global const float* b, "
    "                          global float* c, "
    "                          int N, "
    "                          int TILE_DIM) "
    "{"
    "    int row = get_global_id(0);"
    "    int col = get_global_id(1);"
    "    float sum = 0;"

    "    for (int i=0; i< TILE_DIM; ++i) "
    "    {"
    "        sum += a[row*TILE_DIM+i] * b[i*N+col];"
    "    }"
    "    c[row*N+col] = sum;"
    "}";


const int mat_N     = 128;     
const int mat_K     = 128;     
const int mat_M     = 128;     

float A       [mat_N * mat_K];
float B       [mat_K * mat_M];
float C       [mat_N * mat_M];
float Golden  [mat_N * mat_M];

float dotprod(const float * A, const float * B, int row, int col, int mat_N, int mat_K, int mat_M )
{
    float result = 0;
    for (int k = 0; k < mat_K; ++k)
        result += A[row*mat_K+k] * B[k*mat_M+col];
    return result;
}

void cpu_simple_mat_mpy(const float * A, const float * B, float * C, 
                        int mat_N, int mat_K, int mat_M)
{
    for (int row = 0; row < mat_N; ++row)
        for (int col = 0; col < mat_M; ++col)
            C[row*mat_M+col] = dotprod(A, B, row, col, mat_N, mat_K, mat_M);
}

void mat_print(float *mat, int rows, int cols)
{
   for (int r=0; r < rows; ++r) 
   {
      for (int c=0; c < cols; ++c) 
          cout << mat[r*cols +c] << " ";
      cout << endl ;
   }
   cout << endl << endl;;
}

int main(int argc, char *argv[])
{
   #include "argument_handler.c"

   for (int i=0; i < mat_N * mat_K; ++i) A[i] = 1.0;
   for (int i=0; i < mat_K * mat_M; ++i) B[i] = 1.0;
   for (int i=0; i < mat_N * mat_M; ++i) C[i] = 0.0;

   cpu_simple_mat_mpy(A, B, Golden, mat_N, mat_K, mat_M);

   //mat_print(A, mat_N, mat_K);
   //mat_print(B, mat_K, mat_M);
   //mat_print(Golden, mat_N, mat_M);

   Event ev;

   try 
   {
     Context             context(device_category); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Program::Sources  source(1, std::make_pair(kernelStr, strlen(kernelStr)));
     Program           program(context, source);
     program.build(devices);

     CommandQueue QDSP  (context, devices[0], CL_QUEUE_PROFILING_ENABLE);
     
     Buffer bufA   (context, CL_MEM_READ_ONLY,  sizeof(A));
     Buffer bufB   (context, CL_MEM_READ_ONLY,  sizeof(B));
     Buffer bufC   (context, CL_MEM_WRITE_ONLY, sizeof(C));
     Kernel kernel (program, "Simple_MatMpy");

     kernel.setArg(0, bufA);
     kernel.setArg(1, bufB);
     kernel.setArg(2, bufC);
     kernel.setArg(3, mat_M);
     kernel.setArg(4, mat_K);

     QDSP.enqueueWriteBuffer(bufA, CL_FALSE, 0, sizeof(A), A);
     QDSP.enqueueWriteBuffer(bufB, CL_FALSE, 0, sizeof(B), B);
     QDSP.enqueueNDRangeKernel(kernel, NullRange, NDRange(mat_N, mat_M), 
                                       NDRange(mat_N/NumWorkGroups, mat_M), 
                                       NULL, &ev);
     QDSP.enqueueReadBuffer (bufC, CL_TRUE, 0, sizeof(C), C);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   //mat_print(C, mat_N, mat_M);
   report_event_timing(ev, "MatMpy");

   for (int i = 0; i < mat_N * mat_M; i++)
       if (Golden[i] != C[i])
       {
           std::cout << "Error at " << i << " : " << Golden[i] << " != " 
                     << C[i] << std::endl;
           return 1;
       }

   std::cout << "Success!" << std::endl;
}

/******************************************************************************
* REPORT_EVENT_TIMING - Given an OpenCL Event, report to stdout the profiling
*    info associated with the event
******************************************************************************/
void report_event_timing(const Event &ev, const char* name)
{
     cl_ulong t_que, t_sub, t_strt, t_end;
     
     ev.getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &t_que);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &t_sub);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_START,  &t_strt);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_END,    &t_end);

     if (!name) name = "";

     cout<< name << " : Queue  to Submit: " << t_sub-t_que  << " us" << endl;
     cout<< name << " : Submit to Start : " << t_strt-t_sub << " us" << endl;
     cout<< name << " : Start  to End   : " << t_end-t_strt << " us" << endl;
     cout<< endl;
}
