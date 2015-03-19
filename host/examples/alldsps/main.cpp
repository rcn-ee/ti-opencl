#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
using namespace cl;

const char * kernelStr =
    "kernel void VectorAdd(global const short4* a, "
    "                      global const short4* b, "
    "                      global short4* c) "
    "{"
    "    int id = get_global_id(0);"
    "    c[id] = a[id] + b[id];"
    "}";

const int NumElements     = 1024*1024;     
const int NumWorkGroups   = 8;
const int VectorElements  = 4;
int       NumVecElements  = NumElements / VectorElements;
int       WorkGroupSize   = NumVecElements / NumWorkGroups;

cl_short srcA  [NumElements];
cl_short srcB  [NumElements];
cl_short dst   [NumElements];
cl_short Golden[NumElements];

int main(int argc, char *argv[])
{
   int bufsize = sizeof(srcA);

   for (int i=0; i < NumElements; ++i) 
   { 
       srcA[i]   = srcB[i] = i; 
       Golden[i] = srcB[i] + srcA[i]; 
   }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Program::Sources  source(1, std::make_pair(kernelStr, strlen(kernelStr)));
     Program           program(context, source);
     program.build(devices);

     int NumDevices = devices.size();           // new
     int echunk     = NumElements / NumDevices; // new
     bufsize        /= NumDevices;              // new
     NumVecElements /= NumDevices;              // new
     WorkGroupSize  /= NumDevices;              // new

#pragma omp parallel for
     for (int i = 0; i < NumDevices; ++i)
     {
        Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
        Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
        Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

        Kernel kernel(program, "VectorAdd");
        kernel.setArg(0, bufA);
        kernel.setArg(1, bufB);
        kernel.setArg(2, bufDst);

        CommandQueue Q  (context, devices[i], CL_QUEUE_PROFILING_ENABLE);

        Q.enqueueWriteBuffer(bufA, CL_FALSE, 0, bufsize, &srcA[i*echunk]);
        Q.enqueueWriteBuffer(bufB, CL_FALSE, 0, bufsize, &srcB[i*echunk]);
        Q.enqueueBarrier();
        Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumVecElements), 
                                                  NDRange(WorkGroupSize));
        Q.enqueueBarrier();
        Q.enqueueReadBuffer (bufDst, CL_FALSE, 0, bufsize, &dst[i*echunk]);
        Q.finish();
     }
   }
   catch (Error err)
   {std::cerr<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;}

   if (!memcmp(Golden, dst, sizeof(dst)))
        std::cout << "Success!" << std::endl;
   else std::cout << "Failed!"  << std::endl;
}
