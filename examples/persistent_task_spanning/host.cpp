#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <unistd.h>
#include "kernel.dsp_h"
#include "shared.h"
#include "host_assist.h"

using namespace cl;
using namespace std;

int main(int argc, char *argv[])
{
    try 
    {
        /*---------------------------------------------------------------------
        * Boilerplate OpenCL setup code to create a context, vector of devices,
        * OpenCL C program from the binary contained in kernel.dsp_h and an in
        * order command queue
        *--------------------------------------------------------------------*/
        Context ctx(CL_DEVICE_TYPE_ACCELERATOR);
        std::vector<Device> devices = ctx.getInfo<CL_CONTEXT_DEVICES>();

        Program::Binaries binary(1, 
                              make_pair(kernel_dsp_bin,sizeof(kernel_dsp_bin)));
        Program           program = Program(ctx, devices, binary);
        program.build(devices);

        CommandQueue  Q(ctx, devices[0]);

        /*---------------------------------------------------------------------
        * Abort if not running on an AM57x platform
        *--------------------------------------------------------------------*/
        assert_am57x(devices[0]);

        /*---------------------------------------------------------------------
        * Create a buffer around a completion code variable, so that the DSP
        * can return an application specific status code.
        *--------------------------------------------------------------------*/
        cl_uint completion_code;
        Buffer ccBuf(ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                          sizeof(completion_code), &completion_code);

        Kernel start_dsp(program, "Kstart_dsp");
        Kernel stop_dsp (program, "Kstop_dsp");

        start_dsp.setArg(0, ccBuf);
        stop_dsp .setArg(0, ccBuf);

        /*---------------------------------------------------------------------
        * Enqueue a function to start a persistent task.
        *--------------------------------------------------------------------*/
        Q.enqueueTask(start_dsp);
        Q.finish();

        /*---------------------------------------------------------------------
        * if the start persistent task was successful, then 
        *    Wait for a second to allow time for the persistent task to compute.
        *    Enqueue a function to stop the persistent task.
        *--------------------------------------------------------------------*/
        if (completion_code == APP_OK) 
        {
            sleep(1);  // Give some time for the persistent DSP task to run
            Q.enqueueTask(stop_dsp);
            Q.finish();
        }

        print_completion_code(completion_code);
    }
    catch (Error err) 
    { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }
}
