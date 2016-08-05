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
#include <iomanip>
#include <signal.h>
#include <cstdlib>
#include "ocl_util.h"

#ifdef _TI_RTOS
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

/******************************************************************************
* devtype
******************************************************************************/
const char *devtype(cl_device_type x)
{
    switch (x)
    {
        case CL_DEVICE_TYPE_CPU:         return "CPU";
        case CL_DEVICE_TYPE_ACCELERATOR: return "ACCELERATOR";
        case CL_DEVICE_TYPE_GPU:         return "GPU";
        default:                         return "UNKNOWN";
    }
}

/******************************************************************************
* main
******************************************************************************/
char ary[1<<20];

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
         std::vector<Platform> platforms;
         Platform::get(&platforms);

         for (int p = 0; p < platforms.size(); p++)
         {
             std::string str;

             platforms[p].getInfo(CL_PLATFORM_NAME, &str);
             cout << "PLATFORM: " << str << endl;

             platforms[p].getInfo(CL_PLATFORM_VERSION, &str);
             cout << "  Version: " <<  str << endl;

             platforms[p].getInfo(CL_PLATFORM_VENDOR, &str);
             cout << "  Vendor : " << str << endl;

             platforms[p].getInfo(CL_PLATFORM_PROFILE, &str);
             cout << "  Profile: " <<  str << endl;

             cl_context_properties properties[] =
              {CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[p])(), 0};

             Context context(CL_DEVICE_TYPE_ALL, properties);

             std::vector<Device> devices= context.getInfo<CL_CONTEXT_DEVICES>();

             for (int d = 0; d < devices.size(); d++)
             {
                 devices[d].getInfo(CL_DEVICE_NAME, &str);
                 cout << "    DEVICE: " << str << endl;

                 bool ti_dsp = (str.find("C66") != std::string::npos);

                 cl_device_type type;
                 devices[d].getInfo(CL_DEVICE_TYPE, &type);
                 cout << "      Type       : " << devtype(type) << endl;

                 int num;
                 devices[d].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &num);
                 cout << "      CompUnits  : " << num  << endl;

                 cl_uint bignum;
                 devices[d].getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &bignum);
                 cout << "      Frequency  : " << (double) bignum / 1e3  
                      << " GHz"<< endl;

                 cl_ulong longnum;
                 devices[d].getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &longnum);
                 cout << "      Glb Mem    : " << setw(7) << longnum / 1024  
                      << " KB" << endl;

                 if (ti_dsp)
                 {
                     devices[d].getInfo(CL_DEVICE_GLOBAL_EXT1_MEM_SIZE_TI, &longnum);
                     cout << "      GlbExt1 Mem: " << setw(7) << longnum / 1024  
                          << " KB" << endl;

                     devices[d].getInfo(CL_DEVICE_GLOBAL_EXT2_MEM_SIZE_TI, &longnum);
                     cout << "      GlbExt2 Mem: " << setw(7) << longnum / 1024  
                          << " KB" << endl;

                     devices[d].getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI, &longnum);
                     cout << "      Msmc Mem   : " << setw(7) << longnum / 1024  
                          << " KB" << endl;
                 }

                 devices[d].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &longnum);
                 cout << "      Loc Mem    : " << setw(7) << longnum / 1024  << " KB" << endl;

                 devices[d].getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &longnum);
                 cout << "      Max Alloc  : " << setw(7) << longnum / 1024  << " KB" << endl;
             }
        }
    }

    /*-------------------------------------------------------------------------
    * Let exception handling deal with any OpenCL error cases
    *------------------------------------------------------------------------*/
    catch (Error err) 
    {
        cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
             << ocl_decode_error(err.err()) << ")" << endl;
    }

   RETURN(0);
}

