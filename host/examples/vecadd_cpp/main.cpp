#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;
using std::make_pair;

const char * kernelStr = 
    "__kernel void VectorAdd(__global const float* a, "
    "                        __global const float* b, "
    "                        __global float* c, "
    "                        int NumElements)"
    "{"
    "    int id = get_global_id(0);"
    "    if (id >= NumElements) return; "
    "    c[id] = a[id] + b[id];"
    "}";

int NumElements   = 2 * 1024 * 1024; 
int WorkGroupSize = 1024;         

int main(void)
{
   cl_int err = CL_SUCCESS;
   cl_float *srcA   = (cl_float *)malloc(sizeof(cl_float) * NumElements);
   cl_float *srcB   = (cl_float *)malloc(sizeof(cl_float) * NumElements);
   cl_float *dst    = (cl_float *)malloc(sizeof(cl_float) * NumElements);
   cl_float *Golden = (cl_float *)malloc(sizeof(cl_float) * NumElements);

   for (int i=0; i < NumElements; ++i) srcA[i] = i;
   for (int i=0; i < NumElements; ++i) srcB[i] = i;
   for (int i=0; i < NumElements; ++i) Golden[i] = srcA[i] + srcB[i];

   try 
   {
     Context             context(CL_DEVICE_TYPE_CPU); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     Program::Sources    source(1, make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     CommandQueue        Q(context, devices[0]);

     program.build(devices);

     Buffer bufA   (context, CL_MEM_READ_ONLY,  NumElements * sizeof(*srcA));
     Buffer bufB   (context, CL_MEM_READ_ONLY,  NumElements * sizeof(*srcB));
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, NumElements * sizeof(*dst));

     Kernel kernel(program, "VectorAdd", &err);
     kernel.setArg(0, bufA);
     kernel.setArg(1, bufB);
     kernel.setArg(2, bufDst);
     kernel.setArg(3, NumElements);

     Event event;
     Q.enqueueWriteBuffer  (bufA, CL_TRUE, 0, NumElements * sizeof(*srcA), srcA);
     Q.enqueueWriteBuffer  (bufB, CL_TRUE, 0, NumElements * sizeof(*srcB), srcB);
     Q.enqueueNDRangeKernel(kernel, NullRange, 
                     NDRange(NumElements), NDRange(WorkGroupSize), 
                     NULL, &event); 
     event.wait();
     Q.enqueueReadBuffer   (bufDst, CL_TRUE, 0, NumElements * sizeof(*dst), dst);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (memcmp(Golden, dst, NumElements * sizeof(*dst)))
        cout << "Result Failure" << endl;
   else cout << "Result Passed!" << endl;

   return EXIT_SUCCESS;
}
