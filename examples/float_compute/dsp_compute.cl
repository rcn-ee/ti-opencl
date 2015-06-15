/******************************************************************************
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Texas Instruments Incorporated nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

kernel void dsp_compute(global const float2 *M, 
                        global const float2 *x,
                               const float   C,
                        global       float2 *y,
                        local        float2 *lM,
                        local        float2 *lx,
                        local        float2 *ly)
{
    int grp_id    = get_group_id(0);
    int num_elems = get_local_size(0);

    // Initiate copy of input arrays from global to local memory
    event_t ev1 = async_work_group_copy(lM, M+grp_id*num_elems, num_elems, 0);
    event_t ev2 = async_work_group_copy(lx, x+grp_id*num_elems, num_elems, 0);

    // Wait for copies to complete
    wait_group_events(1, &ev1);
    wait_group_events(1, &ev2);
 
    // Perform compute
    int lid    = get_local_id(0);
    ly[lid] = lx[lid] * lM[lid] + C;

    // Initiate copy of results from local to global memory & wait for 
    // completion
    event_t ev3 = async_work_group_copy(y+grp_id*num_elems, ly, num_elems, 0);
    wait_group_events(1, &ev3);
}
