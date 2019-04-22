/******************************************************************************
 * Copyright (c) 2013-2019, Texas Instruments Incorporated - http://www.ti.com/
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
#include <cstdio>
#include <cstdlib>
#include "ocl_util.h"
#include "kernel.dsp_h"
#include "bandwidth.h"

#ifdef _TI_RTOS
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

const int bufsize = 128 << 10; // 128KB

cl_mem_flags CheckMSMCSizeForBuffer(const Device& ocl_device,
                                          int     bufsize);
void PrintResults(const bandwidth_t*  results,
                        int           NUM_CORES,
                        cl_mem_flags  use_msmc);
#ifdef _TI_RTOS
void ocl_main(UArg arg0, UArg arg1)
{
    int    argc = (int)    arg0;
    char** argv = (char**) arg1;
#else
#define RETURN(x) return x
int main(int argc, char* argv[])
{
#endif
    cl_int err     = CL_SUCCESS;
    const cl_device_partition_property device_partition_properties[3] = {
        CL_DEVICE_PARTITION_EQUALLY, /* Divide equally           */
        1,                           /* 1 compute unit/subdevice */
        0                            /* End of the property list */
    };

    try
    {
        std::vector<Platform> platforms;
        Platform::get(&platforms);

        std::vector<Device> devices;
        platforms[0].getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

        /* Get number of compute units available on root device */
        int NUMCOMPUNITS;
        devices[0].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &NUMCOMPUNITS);

        /* Use sub devices to partition into 1 DSP core per sub device
         * so EDMA bandwidth measurements can be performed on a per core basis
         * */
        std::vector<Device> sub_devices(NUMCOMPUNITS);
        devices[0].createSubDevices(device_partition_properties, &sub_devices);

        /* Create context with sub_devices and their own command queues */
        Context context(sub_devices);
        std::vector<CommandQueue> sub_Qs(NUMCOMPUNITS);
        for (int i=0; i<NUMCOMPUNITS; i++)
        {
            sub_Qs[i] = CommandQueue(context, sub_devices[i]);
        }

        /* A result buffer is allocated in shared DDR CMEM. It is used to create
         * an OpenCL buffer which is used by the kernel for EDMA transfers */
        bandwidth_t *result = (bandwidth_t *)__malloc_ddr(sizeof(bandwidth_t));
        /* An all_results buffer is created in host DDR memory to accumulate
         * bandwidth results from each DSP core after kernels finish
         * executing. This is not used to create an OpenCL buffer so it does
         * not need to be allocated in shared DDR CMEM */
        bandwidth_t *all_results = (bandwidth_t *)malloc(
                                            sizeof(bandwidth_t)*NUMCOMPUNITS);

        /* Determine if MSMC is available and large enough to hold buffer */
	    cl_mem_flags use_msmc = CheckMSMCSizeForBuffer(devices[0], bufsize);

        /* Create buffers in DDR, MSMC memory. These buffers will be reused
         * for each measurement on a single DSP sub device i.e. per core */
        Buffer bufDDR(context, CL_MEM_READ_WRITE, bufsize);
        Buffer bufMSMC(context,
                       (cl_mem_flags) CL_MEM_READ_WRITE|use_msmc,
                       bufsize);
        Buffer bufResult(context,
                         (cl_mem_flags) CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR,
                         sizeof(bandwidth_t), result);

        /* Create program */
        Program::Binaries   binary(NUMCOMPUNITS,
                                   std::make_pair(kernel_dsp_bin,
                                                  sizeof(kernel_dsp_bin)));
        Program             program = Program(context, sub_devices, binary);
        program.build(sub_devices);

        /* Create kernel */
        Kernel kernel(program, "MeasureEDMABandwidth");
        kernel.setArg(0, bufDDR);
        kernel.setArg(1, bufMSMC);
        kernel.setArg(2, __local(bufsize));
        kernel.setArg(3, bufsize);
        kernel.setArg(4, bufResult);

        /* Run kernel on each sub device individually */
        Event ev;
        for (int i=0; i<NUMCOMPUNITS; i++)
        {
            memset((void*)result, 0, sizeof(bandwidth_t));
            sub_Qs[i].enqueueTask(kernel, NULL, &ev);
            ev.wait();
            memcpy(&all_results[i], result, sizeof(bandwidth_t));
        }

        /* Display the EDMA bandwidth results across all DSP cores */
        PrintResults(all_results, NUMCOMPUNITS, use_msmc);

        __free_ddr(result);
        free(all_results);
    }
    catch (Error err)
    {
        cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
             << ocl_decode_error(err.err()) << ")" << endl;
    }

    RETURN(0);
}

/* Return CL_MEM_USE_MSMC_TI flag if MSMC is available on the device and is
 * larger than the given buffer size. Otherwise return 0.
 * */
cl_mem_flags CheckMSMCSizeForBuffer(const Device& ocl_device,
                                          int     bufsize)
{
    std::string  dev_exts;
    ocl_device.getInfo(CL_DEVICE_EXTENSIONS, &dev_exts);

    if (dev_exts.find("cl_ti_msmc_buffers") != std::string::npos)
    {
       cl_ulong msmc_size = 0;
       ocl_device.getInfo(CL_DEVICE_MSMC_MEM_SIZE_TI, &msmc_size);
       if (msmc_size < bufsize) return 0;
    }else return 0;

    return CL_MEM_USE_MSMC_TI;
}

void PrintResults(const bandwidth_t*  results,
                        int           NUM_CORES,
                        cl_mem_flags  use_msmc)
{
    printf("Single channel EDMA bandwidth measured in GB/s\n");
    printf("==============\n");
    printf("On DSP Core  :\t");
    for (int i = 0; i < NUM_CORES; i++)     printf("%d\t", i);
    printf("\n");
    printf("==============\n");

    printf("ddr  => ddr  :\t");
    for (int i = 0; i < NUM_CORES; i++)     printf("%.3f\t", results[i].dd);
    printf("\n");

    if (use_msmc == CL_MEM_USE_MSMC_TI)
    {
        printf("ddr  => msmc :\t");
        for (int i = 0; i < NUM_CORES; i++) printf("%.3f\t", results[i].dm);
        printf("\n");
    }

    printf("ddr  => l2   :\t");
    for (int i = 0; i < NUM_CORES; i++)     printf("%.3f\t", results[i].dl);
    printf("\n");

    if (use_msmc == CL_MEM_USE_MSMC_TI)
    {
        printf("msmc => ddr  :\t");
        for (int i = 0; i < NUM_CORES; i++) printf("%.3f\t", results[i].md);
        printf("\n");

        printf("msmc => msmc :\t");
        for (int i = 0; i < NUM_CORES; i++) printf("%.3f\t", results[i].mm);
        printf("\n");

        printf("msmc => l2   :\t");
        for (int i = 0; i < NUM_CORES; i++) printf("%.3f\t", results[i].ml);
        printf("\n");
    }

    printf("l2   => ddr  :\t");
    for (int i = 0; i < NUM_CORES; i++)     printf("%.3f\t", results[i].ld);
    printf("\n");

    if (use_msmc == CL_MEM_USE_MSMC_TI)
    {
        printf("l2   => msmc :\t");
        for (int i = 0; i < NUM_CORES; i++) printf("%.3f\t", results[i].lm);
        printf("\n");
    }

    printf("l2   => l2   :\t");
    for (int i = 0; i < NUM_CORES; i++)     printf("%.3f\t", results[i].ll);
    printf("\n");
}
