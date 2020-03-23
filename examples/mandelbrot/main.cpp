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
#include <CL/TI/cl.hpp>
#include <SDL/SDL.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <signal.h>
#include "ocl_util.h"

using namespace cl;
using namespace std;

/******************************************************************************
* Configuration parameters
******************************************************************************/
#define MAX_DEVICES 33
#define MAX_IT   1000
#define DIM      500
#define CTR_X   -0.743644177934177585953534617147
#define CTR_Y    0.131826205602324997290253350002
#define RANGE    3.0
#define ZOOM     1.25

/******************************************************************************
* Global Objects
******************************************************************************/
static unsigned char rgb[DIM * DIM * 3];

static double clock_diff (struct timespec *t1, struct timespec *t2)
   { return t2->tv_sec - t1->tv_sec + (t2->tv_nsec - t1->tv_nsec) / 1e9; }

/******************************************************************************
* main
******************************************************************************/
int main(int argc, char *argv[])
{
    struct timespec tp_start, tp_end;
    std::string str;

    /*-------------------------------------------------------------------------
    * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
    *------------------------------------------------------------------------*/
    signal(SIGABRT, exit);
    signal(SIGTERM, exit);

    /*-------------------------------------------------------------------------
    * initialize the video display
    *------------------------------------------------------------------------*/
    bool sdl_initialized = true;
    SDL_Surface *data_sf;
    SDL_Surface *screen;
    if (SDL_Init(SDL_INIT_VIDEO) >= 0 &&
        SDL_SetVideoMode(DIM, DIM, 24, SDL_HWSURFACE))
    {
       data_sf = SDL_CreateRGBSurfaceFrom(rgb, DIM, DIM, 24, DIM * 3,
					 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
       screen = SDL_GetVideoSurface();
       std::string title("Mandelbrot");
       SDL_WM_SetCaption(title.c_str(), NULL );
    }
    else
       sdl_initialized = false;

    /*-------------------------------------------------------------------------
    * Begin OpenCL Setup code in try block to handle any errors
    *------------------------------------------------------------------------*/
    try
    {
        Context context(CL_DEVICE_TYPE_ACCELERATOR);

        /*---------------------------------------------------------------------
        * One time OpenCL Setup
        *--------------------------------------------------------------------*/
        std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
        int                 numDevices = devices.size();
        std::vector<Event>  ev(numDevices);

        CommandQueue*       Q[MAX_DEVICES];
        Buffer              buffer (context, CL_MEM_WRITE_ONLY, sizeof(rgb));

        cout << "DEVICE LIST: " << endl;
        for (int d = 0; d < numDevices; d++)
        {
            devices[d].getInfo(CL_DEVICE_NAME, &str);
            cout << "   " << str << endl;

            Q[d]  = new CommandQueue(context, devices[d]);
        }
        cout << endl;

        /*---------------------------------------------------------------------
        * Compile the Kernel Source for the devices
        *--------------------------------------------------------------------*/
        ifstream t("mandelbrot.cl");
        if (!t)
        {
           std::cout << "Could not open Kernel Source file ([file].cl)\n" << std::endl;
           exit(-1);
        }

        std::string kSrc((istreambuf_iterator<char>(t)),
                          istreambuf_iterator<char>());
        Program::Sources source(1, make_pair(kSrc.c_str(), kSrc.length()));
        Program          program = Program(context, source);
        program.build(devices);

        /*---------------------------------------------------------------------
        * Setup the invariant arguments to the kernel
        *--------------------------------------------------------------------*/
        Kernel kernel(program, "mandelbrot");
        kernel.setArg(0, buffer);
        kernel.setArg(1, DIM);
        kernel.setArg(2, CTR_X);
        kernel.setArg(3, CTR_Y);
        kernel.setArg(5, MAX_IT);

        /*---------------------------------------------------------------------
        * Continuously update image until ctlr-c stops the program
        *--------------------------------------------------------------------*/
        int    frame         = 1;
        double total_elapsed = 0.0;
        for (double range = RANGE; RANGE/range < 1e15; range /= ZOOM, frame++)
        {

            clock_gettime(CLOCK_MONOTONIC, &tp_start);

            kernel.setArg(4, range);

            for (int c = 0; c < numDevices; c++)
            {
                int bufChunk = sizeof(rgb) / numDevices;

                Q[c]->enqueueNDRangeKernel(kernel,
                   NDRange(0,   c * DIM / numDevices), // offset
                   NDRange(DIM, DIM / numDevices),     // global size
                   NDRange(DIM, 1));                   // WG size

                Q[c]->enqueueReadBuffer(buffer, CL_FALSE,
                        c * bufChunk, bufChunk,
                        &rgb[c * bufChunk], NULL, &ev[c]);
            }

            Event::waitForEvents(ev);

            /*-----------------------------------------------------------------
            * Print timing stats
            *----------------------------------------------------------------*/
            clock_gettime(CLOCK_MONOTONIC, &tp_end);
            double elapsed = clock_diff (&tp_start, &tp_end);
            total_elapsed += elapsed;
            printf("Frame: %d, \tFPS: %5.2f, \tZoom: %.3g\n" , frame,
                    1.0/elapsed, RANGE/range);

           /*------------------------------------------------------------------
           * Display the image if SDL successfully initialized.
           * NOTE: Requires X server to display
           *-----------------------------------------------------------------*/
	    if(sdl_initialized)
	    {
	       SDL_Event event;
	       SDL_PollEvent(&event);
	       if (event.type == SDL_QUIT) { SDL_Quit(); exit(0); }

	       if (SDL_BlitSurface(data_sf, NULL, screen, NULL) == 0)
		  SDL_UpdateRect(screen, 0, 0, 0, 0);
	    }
        }
        printf("Total Time Generating frames: %8.4f secs\n", total_elapsed);

        /*---------------------------------------------------------------------
        * Cleanup OpenCL objects
        *--------------------------------------------------------------------*/
        for (int d = 0; d < numDevices; d++) delete Q[d];
    }

    /*-------------------------------------------------------------------------
    * Let exception handling deal with any OpenCL error cases
    *------------------------------------------------------------------------*/
    catch (Error& err)
    { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }
}
