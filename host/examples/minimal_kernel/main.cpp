#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const char * ksrc = "kernel void VectorAdd(global const short4* a,"
                    " global const short4* b, global short4* c) "
                    " { int id = get_global_id(0); c[id] = a[id] + b[id]; }";

const int N = 1024*1024;
cl_short srcA[N], srcB[N], dst[N], Golden[N];

int main(int argc, char *argv[])
{
   for (int i=0; i < N; ++i) 
       { srcA[i] = srcB[i] = i; dst[i] = 0; Golden[i] = srcB[i] + srcA[i]; }

   int bufsize = sizeof(srcA);
   try 
   {
     Context context(CL_DEVICE_TYPE_ACCELERATOR); 
     Buffer  bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer  bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer  bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     Program::Sources    source(1, std::make_pair(ksrc, strlen(ksrc)));
     Program             program = Program(context, source);
     program.build(devices); 

     Kernel kernel(program, "VectorAdd");
     kernel.setArg(0, bufA); kernel.setArg(1, bufB); kernel.setArg(2, bufDst);

     CommandQueue Q(context, devices[0]);
     Q.enqueueWriteBuffer(bufA, CL_TRUE, 0, bufsize, srcA);
     Q.enqueueWriteBuffer(bufB, CL_TRUE, 0, bufsize, srcB);
     Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(N/4), NDRange(N/32));
     Q.enqueueReadBuffer (bufDst, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (memcpy(Golden, dst, sizeof(Golden) == 0)) cout << "Success!" << endl; 
   else                                          cout << "Failed!"  << endl; 
}
