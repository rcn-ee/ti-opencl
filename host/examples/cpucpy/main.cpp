#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const char * kernelStr =
    "kernel void cpucpy(global const int* src, "
    "                      global int* dst) "
    "{"
    "    int id = get_global_id(0);"
    "    dst[id] = src[id]; "
    "}";

const int NumElements     = 2048;
const int NumWorkGroups   = 8; 
const int WorkGroupSize   = NumElements / NumWorkGroups;

cl_short src   [NumElements];
cl_short dst   [NumElements];

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int    bufsize = sizeof(src);

   for (int i=0; i < NumElements; ++i) { src[i] = i; dst[i] = 0; }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices); 

     Kernel kernel(program, "cpucpy");
     kernel.setArg(0, bufA);
     kernel.setArg(1, bufDst);

     CommandQueue Q(context, devices[0]);

     Q.enqueueWriteBuffer(bufA, CL_TRUE, 0, bufsize, src);
     Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumElements), 
                                               NDRange(WorkGroupSize));
     Q.flush();
     Q.enqueueReadBuffer (bufDst, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (memcmp(dst, src, bufsize) != 0)
        cout << "Failed!"  << endl; 
   else cout << "Success!" << endl; 
}
