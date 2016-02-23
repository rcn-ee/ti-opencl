/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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
#include <fstream>
#include <signal.h>
#include <string.h>
#include <cassert>
#include <cstdlib>
#include <stdio.h>
#include "ocl_util.h"
#include "dsp_kernels.dsp_h"
#include "initial.h"

#include "gaussRandom.h"
#include "utilityRoutines.h"
#include "generateRandomGaissian.h"


#define   MAX_ELEMENTS    (32*1024)


using namespace cl;
using namespace std;

#define  ELEMENTS (32 << 10)
#define  ITERATIONS   9         // Must be Odd because of double nature of loop

static void print_event (Event ev);
static void output_data (float *data, int elements, const char *filename);
static void consumeBuffer (float *pp, int N);

static double clock_diff (struct timespec *t1, struct timespec *t2);

static void run_on_arm ();

/******************************************************************************
 * main
 *****************************************************************************/
int main (int argc, char *argv[])
{
    int loopCount;
    initial1_t initArea[8];

   /*-------------------------------------------------------------------------
    * Catch ctrl-c so we ensure dtors are called and the dsp is reset properly
    *------------------------------------------------------------------------*/
    signal (SIGABRT, exit);
    signal (SIGTERM, exit);

    cout << "\nThis code generates Gaussian Random Numbers (0,1) and \n";
    cout << "calculates how many numbers are less than 0 and more than 0\n";
    cout << "\nFor more information, refer the OpenCL documentation \n";
    cout << "located at http://software-dl.ti.com/mctools/esd/docs/opencl/index.html\n";

    run_on_arm();

   /*-------------------------------------------------------------------------
    * Initialize the input matrices to random data
    *------------------------------------------------------------------------*/
    initializeInit1 (initArea);

   /*-------------------------------------------------------------------------
    * Begin OpenCL Setup code in try block to handle any errors
    *------------------------------------------------------------------------*/
    try
    {
        Context ctx (CL_DEVICE_TYPE_ACCELERATOR);
        std::vector < Device > devices =
            ctx.getInfo < CL_CONTEXT_DEVICES > ();
        CommandQueue Q (ctx, devices[0], CL_QUEUE_PROFILING_ENABLE);

      /*---------------------------------------------------------------------
       * Determine how many chunks based on how many DSP cores are available 
       *--------------------------------------------------------------------*/
        int num_chunks;
        devices[0].getInfo (CL_DEVICE_MAX_COMPUTE_UNITS, &num_chunks);

      /*---------------------------------------------------------------------
      * Allow no more than 4 chunks, as they index into the seed structure
      * and only 4 elements of that structure are initialized.
      *--------------------------------------------------------------------*/
        if (num_chunks > 4)
            num_chunks = 4;

        Buffer initBuf (ctx, CL_MEM_READ_WRITE, sizeof (initArea));
        Q.enqueueWriteBuffer (initBuf, CL_TRUE, 0, sizeof (initArea),
                              initArea);

      /*---------------------------------------------------------------------
       * Compile the Kernel Source for the devices
       *--------------------------------------------------------------------*/
        Program::Binaries binary (1,
                                  make_pair (dsp_kernels_dsp_bin,
                                             sizeof (dsp_kernels_dsp_bin)));
        Program program = Program (ctx, devices, binary);
        program.build (devices);

        KernelFunctor dsp_random =
            Kernel (program, "ocl_random").bind (Q, NDRange (num_chunks),
                                                 NDRange (1));

      /*---------------------------------------------------------------------
       * Allocate host arrays (ary*) and DSP buffers (buf*)
       *--------------------------------------------------------------------*/
        int ary_size = ELEMENTS * sizeof (float);
        float *ary1 = (float *) __malloc_ddr (ary_size);
        float *ary2 = (float *) __malloc_ddr (ary_size);

        Buffer buf1 (ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, ary_size,
                     ary1);
        Buffer buf2 (ctx, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, ary_size,
                     ary2);


      /*---------------------------------------------------------------------
       * Call the first kernel -> c code function.
       *--------------------------------------------------------------------*/
        Event ev = dsp_random (buf1, initBuf, ELEMENTS / num_chunks);
        ev.wait ();
        print_event (ev);

      /*---------------------------------------------------------------------
      * Starting the loop
      *     
      *    1. Start a new DSP acceleration execution
      *    2. Read the previous buffer of random numbers
      *    3. Call ARM code that is work in parallel with the DSP acceleration
      *    4. Wait for the DSP acdceleration, calculate the time, print time
      *--------------------------------------------------------------------*/
        for (loopCount = 0; loopCount < ITERATIONS - 1; loopCount += 2)
        {
            Event ev2 = dsp_random (buf2, initBuf, ELEMENTS / num_chunks);
            consumeBuffer (ary1, ELEMENTS);
            ev2.wait ();
            print_event (ev2);

            Event ev = dsp_random (buf1, initBuf, ELEMENTS / num_chunks);
            consumeBuffer (ary2, ELEMENTS);
            ev.wait ();
            print_event (ev);
        }

        consumeBuffer (ary1, ELEMENTS);
        Q.finish ();

        output_data (ary1, ELEMENTS, "dataOut.txt");
    }

    /*----------------------------------------------------------------------
     * Let exception handling deal with any OpenCL error cases
     *--------------------------------------------------------------------*/
    catch (Error err)
    {
        cerr << "ERROR: " << err.what ()
            << "(" << ocl_decode_error (err.err ()) << ")" << endl;
    }

    cout << "\nEND RUN!" << endl;

}

static void run_on_arm ()
{
    struct initial_t vector1[1];
    float outputResults[MAX_ELEMENTS];


    initializeInit (vector1);

    struct timespec tp_start, tp_end;
    clock_gettime (CLOCK_MONOTONIC, &tp_start);

    longPseudoRandom (vector1, 0, MAX_ELEMENTS, outputResults);

    consumeBuffer (outputResults, MAX_ELEMENTS);

    clock_gettime (CLOCK_MONOTONIC, &tp_end);

    double elapsed = clock_diff (&tp_start, &tp_end);
    printf ("ARM Only, Total time %.2lf nanoseconds\n", elapsed);
    printf ("          %0.2f nanoseconds per element\n",
            elapsed / MAX_ELEMENTS);
    printf ("          %d elements processed\n", MAX_ELEMENTS);
}


/******************************************************************************
* consumeBuffer    
******************************************************************************/
static void consumeBuffer (float *pp, int N)
{
    long countPos = 0;
    long countNeg = 0;
    long countZero = 0;

    for (int i = 0; i < N; i++)
    {
        if (*pp > 0.0)
            countPos++;
        if (*pp == 0.0)
            countZero++;
        if (*pp < 0.0)
            countNeg++;
        pp++;
    }

    printf (" count positive =  %ld  count Negative = %ld zero  %ld  \n",
            countPos, countNeg, countZero);
}

/******************************************************************************
* void print_event(Event ev)
******************************************************************************/
void print_event (Event ev)
{
    cl_ulong start, end;
    ev.getProfilingInfo (CL_PROFILING_COMMAND_START, &start);
    ev.getProfilingInfo (CL_PROFILING_COMMAND_END, &end);

    float total_time = end - start;

    printf ("ARM + DSP, Total time %.2f nanoseconds\n", total_time);
    printf ("           %0.2f nanoseconds per element\n",
            total_time / ELEMENTS);
    printf ("           %d elements processed\n", ELEMENTS);
}

/******************************************************************************
* void output_data(float *data, int elements, const char * filename)
******************************************************************************/
void output_data (float *data, int elements, const char *filename)
{
    ofstream myfile (filename);

    if (!myfile.is_open ())
    {
        cout << "unable to open file" << endl;
        exit (-1);
    }

    for (int i = 0; i < elements; i++)
        myfile << *data++ << endl;

    myfile.close ();
}


static double clock_diff (struct timespec *t1, struct timespec *t2)
{
    return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec);
}
