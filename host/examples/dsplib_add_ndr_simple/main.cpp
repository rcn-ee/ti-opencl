#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

const char * kernelStr =
    "kernel void DSP_add16_k(global const short* x, "
    "      global const short* y, global short* r) "
    "{ "
    "  int N_per_WG = get_local_size(0); "
    "  int WG       = get_group_id(0); "
    "  DSP_add16(x + WG*N_per_WG, y + WG*N_per_WG, r + WG*N_per_WG, N_per_WG); "
    "} " ;

const int N    = 4 * 1024*1024;
const int WG   = 8; 
const int WGsz = N / WG;

cl_short srcX[N], srcY[N], dstR[N], Golden[N];

int main(int argc, char *argv[])
{
   for (int i=0; i < N; ++i) 
       { srcX[i] = srcY[i] = i; dstR[i] = 0; Golden[i] = srcY[i] + srcX[i]; }

   int bufsize = sizeof(srcX);
   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufX   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufY   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufR   (context, CL_MEM_WRITE_ONLY, bufsize);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices, "/opt/ti/dsplib_c66x_3_1_0_0/lib/dsplib.ae66"); 

     CommandQueue Q(context, devices[0]);
     Q.enqueueWriteBuffer(bufX, CL_TRUE, 0, bufsize, srcX);
     Q.enqueueWriteBuffer(bufY, CL_TRUE, 0, bufsize, srcY);

     Kernel dkernel(program, "DSP_add16_k");
     KernelFunctor DSP_add16(dkernel,Q, NullRange, NDRange(N), NDRange(WGsz));
     DSP_add16(bufX, bufY, bufR);

     Q.enqueueReadBuffer (bufR, CL_TRUE, 0, bufsize, dstR);
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   if (memcpy(Golden, dstR, sizeof(Golden) == 0)) cout << "Success!" << endl; 
   else                                           cout << "Failed!"  << endl; 
}
