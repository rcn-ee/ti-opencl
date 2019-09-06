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
#include <SDL/SDL.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <signal.h>

using namespace std;

/******************************************************************************
* Configuration parameters
******************************************************************************/
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

extern void mandelbrot_cpu(unsigned char *buf, int dim,
          double ctr_x, double ctr_y, double range, int max_iterations);

/******************************************************************************
* main
******************************************************************************/
int main(int argc, char *argv[])
{
    struct timespec tp_start, tp_end;

    /*-------------------------------------------------------------------------
    * Catch ctrl-c so we ensure that we call dtors and the dsp is reset properly
    *------------------------------------------------------------------------*/
    signal(SIGABRT, exit);
    signal(SIGTERM, exit);

    /*-------------------------------------------------------------------------
    * initialize the video display
    *------------------------------------------------------------------------*/
    bool sdl_initialized = false;
    SDL_Surface *data_sf;
    SDL_Surface *screen;

    if ( (SDL_Init(SDL_INIT_VIDEO) >= 0) &&
	(SDL_SetVideoMode(DIM, DIM, 24, SDL_HWSURFACE)))
    {
       sdl_initialized = true;
       data_sf = SDL_CreateRGBSurfaceFrom(rgb, DIM, DIM, 24, DIM * 3,
				         0x000000ff, 0x0000ff00, 0x00ff0000, 0);
       screen = SDL_GetVideoSurface();
       std::string title("Mandelbrot Native");
       SDL_WM_SetCaption(title.c_str(), NULL );
    }

    /*---------------------------------------------------------------------
    * Continuously update image until ctlr-c stops the program
    *--------------------------------------------------------------------*/
    int frame = 1;
    double total_elapsed = 0.0;
    for (double range = RANGE; RANGE/range < 1e15; range /= ZOOM, frame++)
    {
        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        mandelbrot_cpu(rgb, DIM, CTR_X, CTR_Y, range, MAX_IT);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);

        double elapsed = clock_diff (&tp_start, &tp_end);
        total_elapsed += elapsed;
        printf("Frame: %d, \tFPS: %5.2f, \tZoom: %.3g\n" , frame,
                1.0/elapsed, RANGE/range);

       /*------------------------------------------------------------------
       * Display the image if we were able to utilize SDL
       *-----------------------------------------------------------------*/
       if (sdl_initialized)
       {
	        SDL_Event event;
	        SDL_PollEvent(&event);
	        if (event.type == SDL_QUIT) { SDL_Quit(); exit(0); }

	        if (SDL_BlitSurface(data_sf, NULL, screen, NULL) == 0)
	           SDL_UpdateRect(screen, 0, 0, 0, 0);
       }
    }
    printf("Total Time Generating frames: %8.4f secs\n", total_elapsed);

}
