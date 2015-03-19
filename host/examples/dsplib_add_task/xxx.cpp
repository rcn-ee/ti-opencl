const char * kernelStr =
    "int DSP_add16 (global short* x, global short* y, global short*r, int nx); "

    "kernel void DSP_add16_kernel(global const short* x, global const short* y,"
    "                             global short* r, int nx) "
    "{ DSP_add16(x, y, r, nx); }" ;


int main(int argc, char *argv[])
{
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

     Kernel tkernel(program, "DSP_add16_kernel");
     tkernel.setArg(0, bufX);
     tkernel.setArg(1, bufY);
     tkernel.setArg(2, bufR);
     tkernel.setArg(3, N);

     CommandQueue Q(context, devices[0]);

     Q.enqueueWriteBuffer(bufX, CL_TRUE, 0, bufsize, srcX);
     Q.enqueueWriteBuffer(bufY, CL_TRUE, 0, bufsize, srcY);
     Q.enqueueTask(tkernel);
     Q.enqueueReadBuffer (bufR, CL_TRUE, 0, bufsize, dstR);
   }
}
