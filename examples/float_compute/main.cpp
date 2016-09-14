/******************************************************************************
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include "ocl_util.h"
#include "assert.h"

#ifdef _TI_RTOS
#include <ti/sysbios/posix/_time.h>
#include "dsp_compute.dsp_h"
#include "../rtos_main.c"
#endif

using namespace cl;
using namespace std;

const int NumElements     = 2 * 1024 * 1024; // 2M elements
const int VecLen          = sizeof(cl_float2)/sizeof(float);
const int NumVecElements  = NumElements / VecLen;
const int WorkGroupSize   = 4 * 1024/VecLen; // 16KB of local memory per array

const int NUM_TRIES = 5;


const float C       = 42.0;
const float EPSILON = 0.000001;

#ifdef _TI_RTOS
float *y_Golden;
#else
float y_Golden[NumElements];
#endif

static void compute_on_arm(float * __restrict__ in1,
                           float * __restrict__ in2,
                           float * __restrict__ out,
                           int                  count,
                           float                C);

static double clock_diff (struct timespec *t1, struct timespec *t2);
static double average(double *array, int count);
static void   print_results(double *arm_time, double *dsp_time, int count);
static void   print_header();
static void   print_footer();

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
    print_header();

    // OpenCL APIs in a try-catch block to detect errors
    try {
    
    // Create an OpenCL context with the accelerator device
    Context context(CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
     
#ifndef _TI_RTOS
    // Create the OpenCL program using the DSP binary
    char *bin;
    int bin_length = ocl_read_binary("dsp_compute.out", bin);

    Program::Binaries   binary(1, std::make_pair(bin, bin_length));
    Program             program = Program(context, devices, binary);
    program.build(devices);

    delete [] bin;
#else
    Program::Binaries   binary(1, make_pair(dsp_compute_dsp_bin,
                                            sizeof(dsp_compute_dsp_bin)));
    Program             program = Program(context, devices, binary);
    program.build(devices);
#endif
         
    // Create an OpenCL command queue
    CommandQueue Q(context, devices[0]);


    // Allocate arrays in contiguous shared memory to avoid copies when 
    // dispatching from ARM to DSP
    cl_float *M = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);
    cl_float *x = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);
    cl_float *y = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);
#ifdef _TI_RTOS
    cl_float *y_Golden = (cl_float *)__malloc_ddr(sizeof(float) * NumElements);
#endif

    assert (M != NULL);
    assert (x != NULL);
    assert (y != NULL);


    double arm_time[NUM_TRIES];
    double dsp_time[NUM_TRIES];

    for (int run = 0; run < NUM_TRIES; run++)
    {
        // Initialize inputs
        for (int i=0; i < NumElements; ++i) 
        { 
           M[i] = rand(); 
           x[i] = rand();

           y[i]        = 0;
           y_Golden[i] = 0;
        }

        // Perform compute on ARM
        struct timespec tp_start, tp_end;

        clock_gettime(CLOCK_MONOTONIC, &tp_start);

        compute_on_arm(M, x, y_Golden, NumElements, C);

        clock_gettime(CLOCK_MONOTONIC, &tp_end);

        arm_time[run] = clock_diff (&tp_start, &tp_end);

        // Perform compute on OpenCL device
        const int BufSize = sizeof(float) * NumElements;
        Buffer bufM(context,CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,  BufSize, M);
        Buffer bufx(context,CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,  BufSize, x);
        Buffer bufy(context,CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, BufSize, y);

        Kernel kernel(program, "dsp_compute");
        kernel.setArg(0, bufM);
        kernel.setArg(1, bufx);
        kernel.setArg(2, C);
        kernel.setArg(3, bufy);
        kernel.setArg(4, __local(sizeof(cl_float2)*WorkGroupSize));
        kernel.setArg(5, __local(sizeof(cl_float2)*WorkGroupSize));
        kernel.setArg(6, __local(sizeof(cl_float2)*WorkGroupSize));

        Event ev1;
        clock_gettime(CLOCK_MONOTONIC, &tp_start);

        // Dispatch the kernel
        Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumVecElements), 
                               NDRange(WorkGroupSize), NULL, &ev1);

        // Wait fo the kernel to complete execution
        ev1.wait();

        clock_gettime(CLOCK_MONOTONIC, &tp_end);

        dsp_time[run] = clock_diff (&tp_start, &tp_end);

        cout << "." << flush;

       // Check results
       for (int i=0; i < NumElements; ++i)
           if (fabs(y_Golden[i] - y[i]) > EPSILON) 
               { cout << "Failed at Element " << i << endl; RETURN(-1); }
    }

    print_results (arm_time, dsp_time, NUM_TRIES);

    print_footer();

    __free_ddr(M);
    __free_ddr(x);
    __free_ddr(y);
#ifdef _TI_RTOS
    __free_ddr(y_Golden);
#endif

    } // end try
    catch (Error err) 
    { 
        cerr << "ERROR: " << err.what() << "(" << err.err() << ", "
             << ocl_decode_error(err.err()) << ")" << endl;
        exit(1);
    }

    RETURN(0);
}

static void compute_on_arm(float * __restrict__ in1,
                           float * __restrict__ in2,
                           float * __restrict__ out,
                           int                  count,
                           float                C)
{
    #pragma omp parallel for num_threads(2)
    for (int i=0; i < count; ++i) 
        out[i] = in1[i] * in2[i] + C; 
}

/*
 * Helper functions
 */

static double clock_diff (struct timespec *t1, struct timespec *t2)
       { return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec) / 1e9; }


static double average(double *array, int count)
{
    double sum = 0;
    for (int i =0; i < count; i++)
        sum += array[i];

    return sum/count;
}

static void print_results(double *arm_time, double *dsp_time, int count)
{
    double arm_ave = average(arm_time, count); 
    double dsp_ave = average(dsp_time, count);

    cout << endl;
    cout << endl << "Average across " << NUM_TRIES << " runs: " << endl;
#ifndef _TI_RTOS
    cout << "ARM (2 OpenMP threads)         : " << 
#else
    cout << "ARM                            : " <<
#endif
                            fixed << arm_ave << " secs" << endl;

    cout << "DSP (OpenCL NDRange kernel)    : " << 
                            fixed << dsp_ave << " secs" << endl;
    cout << "OpenCL-DSP speedup             : " << 
                            fixed << arm_ave/dsp_ave << endl;
}

static void print_header()
{
    cout << endl;
    cout << "This example computes y[i] = M[i] * x[i] + C on "
         << "single precision floating point arrays of size " << NumElements
         << endl
         << "- Computation on the ARM is parallelized across the A15s"
         << " using OpenMP."
         << endl
         << "- Computation on the DSP is performed by dispatching an OpenCL"
            " NDRange kernel across the compute units (C66x cores) in the"
            " compute device." << endl;

    cout << endl << "Running" << flush;
}

static void print_footer()
{
    cout << endl;
    cout << "For more information on:" << endl;
    cout << "  * TI's OpenCL product, http://software-dl.ti.com/mctools/esd/docs/opencl/index.html" << endl;
    cout << endl;
}



