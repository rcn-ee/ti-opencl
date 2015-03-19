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

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define NUM_COLORS 16
__constant unsigned char colors[NUM_COLORS][3] = {
    { 25, 7, 26 },      { 9, 1, 47 },      { 4, 4, 73 },     { 0, 7, 100 },
    { 12, 44, 138 },    { 24, 82, 177 },   { 57, 125, 209 }, { 134, 181, 229 },
    { 211, 236, 248 },  { 241, 233, 191 }, { 248, 201, 95 }, { 255, 170, 0 },
    { 204, 128, 0 },    { 153, 87, 0 },    { 106, 52, 3 },   { 66, 30, 15 },
};

/******************************************************************************
* get_color
******************************************************************************/
void get_color(__global unsigned char *p, double mu, int max_it)
{
    if (mu >= max_it ) { p[0] = 0; p[1] = 0; p[2] = 0; return; }

    mu /= 6;  // arbitrary and makes colors nice

    int    c1_idx = (int)mu % NUM_COLORS;
    int    c2_idx = (c1_idx + 1) % NUM_COLORS;
    double offset = mu - (double)(int)mu;

    p[0] = offset * (colors[c2_idx][0] - colors[c1_idx][0]) + colors[c1_idx][0];
    p[1] = offset * (colors[c2_idx][1] - colors[c1_idx][1]) + colors[c1_idx][1];
    p[2] = offset * (colors[c2_idx][2] - colors[c1_idx][2]) + colors[c1_idx][2];
}

/******************************************************************************
* Mandelbrot OpenCL kernel
******************************************************************************/
__kernel void mandelbrot(__global unsigned char *buf, int dim,
                    double ctr_x, double ctr_y,
                    double range, int max_iterations)
{
    int col  = get_global_id(0);
    int row  = get_global_id(1);
    double x = ((double)col / (double)dim * range) + (ctr_x - range/2);
    double y = ((double)row / (double)dim * range) + (ctr_y - range/2);
    double x0 = x;
    double y0 = y;
    int iteration = 0;

    while (x*x + y*y < (2*2) && iteration < max_iterations)
    {
        double xtemp = x*x - y*y + x0;
        y = 2 * x * y + y0;
        x = xtemp;
        iteration++;
    }

    double mu = iteration; 
    if (iteration < max_iterations)
        mu = iteration + 1 - log(log(sqrt(x*x+y*y))) / M_LN2;

    get_color(&buf[(row * dim + col)*3], mu, max_iterations);
}
