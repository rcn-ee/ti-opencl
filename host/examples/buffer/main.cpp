#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const int NumElements = 32;

cl_int src  [NumElements];
cl_int dst  [NumElements];

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int    bufsize = sizeof(src);

   for (int i=0; i < NumElements; ++i) { src[i] = i; dst[i] = 0; }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer buf    (context, CL_MEM_READ_ONLY,  bufsize);
     CommandQueue Q(context, devices[0]);

     Q.enqueueWriteBuffer(buf, CL_TRUE, 0, bufsize, src);
     Q.enqueueReadBuffer (buf, CL_TRUE, 0, bufsize, dst);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (memcmp(dst, src, bufsize) != 0) cout << "Failed!" << endl; 
   else                                cout << "Passed!" << endl; 
}
