/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <cassert>
#include <cstdlib>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include "oclwrapper.dsp_h"
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

char    data [1 << 20];
#define BYTES_PER_WG (1 << 12)
#define WGS (sizeof(data)/ BYTES_PER_WG)

/******************************************************************************
* main
******************************************************************************/
#ifdef _TI_RTOS
void ocl_main(UArg arg0, UArg arg1)
{
   int    argc = (int)     arg0;
   char **argv = (char **) arg1;
#else
#define RETURN(x) return x
int main(int argc, char *argv[])
{
#endif
    /*-------------------------------------------------------------------------
    * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
    *------------------------------------------------------------------------*/
    signal(SIGABRT, exit);
    signal(SIGTERM, exit);

    /*-------------------------------------------------------------------------
    * Begin OpenCL Setup code in try block to handle any errors
    *------------------------------------------------------------------------*/
    try 
    {
        Context context(CL_DEVICE_TYPE_ACCELERATOR);
        std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        /*---------------------------------------------------------------------
        * This demo will only setup a Q to dsp0
        *--------------------------------------------------------------------*/
        CommandQueue Q(context, devices[0]);
        Buffer       buffer (context, CL_MEM_READ_WRITE, sizeof(data));

#ifndef _TI_RTOS
        /*---------------------------------------------------------------------
        * Compile the Kernel Source for the devices
        *--------------------------------------------------------------------*/
        ifstream t("oclwrapper.cl");
        std::string kSrc((istreambuf_iterator<char>(t)),
                          istreambuf_iterator<char>());
        Program::Sources source(1, make_pair(kSrc.c_str(), kSrc.length()));
        Program          program = Program(context, source);

        /*---------------------------------------------------------------------
        * Build the opencl c code and tell it to link with the specified 
        * object file.
        *--------------------------------------------------------------------*/
        program.build(devices, "ccode.obj"); 
#else
        Program::Binaries binary(1, make_pair(oclwrapper_dsp_bin,
                                              sizeof(oclwrapper_dsp_bin)));
        Program           program = Program(context, devices, binary);
        program.build(devices);
#endif

        /*---------------------------------------------------------------------
        * Call the first kernel -> c code function.
        *   The result of which should be 0xff written to each element of buffer
        *--------------------------------------------------------------------*/
        Kernel kernel1(program, "oclwrapper1");
        kernel1.setArg(0, buffer);
        kernel1.setArg(1, BYTES_PER_WG);
        Q.enqueueNDRangeKernel(kernel1, NDRange(0),            // offset
                                        NDRange(WGS), // global size
                                        NDRange(1));        // WG size

        /*---------------------------------------------------------------------
        * Call the second kernel -> c code function. 
        *   The result of which should be 0x7f subtracted from each element 
        *   of buffer.
        *
        *   The modifications to the buffer from the last kernel are still 
        *   valid for the invocation of the second kernel.  Ie. the data in the 
        *   buffer is persistent.
        *--------------------------------------------------------------------*/
        Kernel kernel2(program, "oclwrapper2");
        kernel2.setArg(0, buffer);
        kernel2.setArg(1, BYTES_PER_WG);
        Q.enqueueNDRangeKernel(kernel2, NDRange(0),            // offset
                                        NDRange(WGS), // global size
                                        NDRange(1));        // WG size

        /*---------------------------------------------------------------------
        * Read the buffer back into host memory
        *--------------------------------------------------------------------*/
        Q.enqueueReadBuffer(buffer, CL_TRUE, 0, sizeof(data), data);
    }

    /*-------------------------------------------------------------------------
    * Let exception handling deal with any OpenCL error cases
    *------------------------------------------------------------------------*/
    catch (Error err) 
    {
        cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
             << ocl_decode_error(err.err()) << ")" << endl; 
    }

    /*-------------------------------------------------------------------------
    * Check the buffer for all elements == 0x80
    *------------------------------------------------------------------------*/
    for (int i = 0; i < sizeof(data); ++i) assert (data[i] == (char)0x80);
    cout << "Success!" << endl;

    RETURN(0);
}
