#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const char * kernelStr =
    "int omp (global int* p, int N); "
    "kernel void omp_facade(global int* p, int N) { omp(p,N); }" ;

const int N     = 1024*1024;
cl_short src   [N];
cl_short dst   [N];

int main(int argc, char *argv[])
{
   int    bufsize = sizeof(src);

   for (int i=0; i < N; ++i) src[i] = i; 

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer buf   (context, CL_MEM_READ_WRITE,bufsize);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices, "/home/al/clover/examples/omp/omp.out");

     Kernel fkernel(program, "omp_facade");
     fkernel.setArg(0, buf);
     fkernel.setArg(1, N);

     CommandQueue Q(context, devices[0]);

     Q.enqueueWriteBuffer(buf, CL_TRUE, 0, bufsize, src);
     Q.enqueueTask(fkernel);
     Q.enqueueReadBuffer(buf, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   for (int i = 0; i < N; i++)
       if (dst[i] != src[i] + 1) { cout << "Failed!" << endl; break; }
   cout << "success!" << endl;
}
