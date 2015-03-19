#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const char * kernelStr =
    "float sinf(float); "
    "kernel void VectorSin(global const float* a, global float* b) {"
    "    int id = get_global_id(0);"
    "    b[id] = sinf(a[id]);"
    "}";

const int NumElements     = 64;

float src   [NumElements];
float dst   [NumElements];
float Golden[NumElements];

int main(int argc, char *argv[])
{
#include "argument_handler.c"
const int WorkGroupSize   = NumElements / NumWorkGroups;

   int    bufsize = sizeof(src);

   for (int i=0; i < NumElements; ++i) 
   { src[i] = i; dst[i] = 2.0; Golden[i] = sin(i); }

   try 
   {
     Context             context(device_category); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufSrc (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices);

     Kernel kernel(program, "VectorSin");
     kernel.setArg(0, bufSrc);
     kernel.setArg(1, bufDst);

     CommandQueue Q(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

     Q.enqueueWriteBuffer(bufSrc, CL_TRUE, 0, bufsize, src);
     Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumElements), 
                                               NDRange(WorkGroupSize));
     Q.enqueueReadBuffer (bufDst, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   int ulp_diff(float v1, float v2);
   int delta;
   for (int i=0; i < NumElements; ++i)
      if ((delta = ulp_diff(Golden[i], dst[i])) > 1)
         cout << "Failed at Element " << i  << ": ulps: " <<  delta << endl; 

   cout << "Success!" << endl; 
}

int ulp_diff(float v1, float v2)
{
    int hex1 = *(int*)&v1;
    int hex2 = *(int*)&v2;
    return abs(hex1 - hex2);
}
