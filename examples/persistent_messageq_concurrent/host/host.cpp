#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include "stdio.h"
#include "../dsp/kernel.dsp_h"
#include "msgq.h"
#include "shared.h"
#include "host_assist.h"

using namespace cl;
using namespace std;

/******************************************************************************
* main
******************************************************************************/
int main(int argc, char *argv[])
{
    /*-------------------------------------------------------------------------
    * Begin OpenCL Setup code in try block to handle any errors
    *------------------------------------------------------------------------*/
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

        Kernel K(program, "wrapper");
        MsgQ   msgq("APP-D2A");

        K.setArg(0, msgq.id());
        K.setArg(1, ccBuf);

        /*---------------------------------------------------------------------
        * Start DSP running 
        *--------------------------------------------------------------------*/
        Q.enqueueTask(K);

        /*---------------------------------------------------------------------
        * Concurrent with the DSP running, receive messages
        *--------------------------------------------------------------------*/
        uint32_t num_received = 0;
        while (1)
        {
            int id = msgq.receive();
            if (id == 0xFFFFu) break;
            num_received++;
        }

        /*---------------------------------------------------------------------
        * Wait for DSP to complete
        *--------------------------------------------------------------------*/
        Q.finish();
        printf("Host received %u messages\n", num_received);

        print_completion_code(completion_code);
    }
    catch (Error err) 
    { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }
}
