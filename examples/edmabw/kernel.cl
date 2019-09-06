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
#include "bandwidth.h"

float bandwidth(char *dst, char *src, int size)
{
    int64_t t0 = __clock64();
    /* Since the async_work_group_copy API is used to transfer between global
     * and local memory regions, the dst and src pointes must be cast
     * appropriately. This example transfers l2 to l2 i.e. local to local as
     * well as ddr to ddr or msmc to msmc i.e. global to global */
    event_t ev = async_work_group_copy((local char*)        dst,
                                       (const global char*) src,
                                       (size_t)             size,
                                                            0);
    wait_group_events(1, &ev);
    int64_t t1 = __clock64();

    /*-------------------------------------------------------------------------
    * return GB/s
    *------------------------------------------------------------------------*/
    return ((float)size/1e9) / ((t1-t0)/(__dsp_frequency()*1e6));
}

kernel void MeasureEDMABandwidth(global const char*        ddr,
                                 global const char*        msmc,
                                 local  const char*        l2,
                                              int          size,
                                 global       bandwidth_t* result)
{
    int trials = 100;
    int i;
    float dd = 0;
    float dm = 0;
    float dl = 0;
    float md = 0;
    float mm = 0;
    float ml = 0;
    float ld = 0;
    float lm = 0;
    float ll = 0;

    for(i=0;i<trials;++i) dd += bandwidth((char*)ddr,  (char*)ddr,  size);
    for(i=0;i<trials;++i) dm += bandwidth((char*)ddr,  (char*)msmc, size);
    for(i=0;i<trials;++i) dl += bandwidth((char*)ddr,  (char*)l2,   size);
    for(i=0;i<trials;++i) md += bandwidth((char*)msmc, (char*)ddr,  size);
    for(i=0;i<trials;++i) mm += bandwidth((char*)msmc, (char*)msmc, size);
    for(i=0;i<trials;++i) ml += bandwidth((char*)msmc, (char*)l2,   size);
    for(i=0;i<trials;++i) ld += bandwidth((char*)l2,   (char*)ddr,  size);
    for(i=0;i<trials;++i) lm += bandwidth((char*)l2,   (char*)msmc, size);
    for(i=0;i<trials;++i) ll += bandwidth((char*)l2,   (char*)l2,   size);

    (*result).dd = dd/trials;
    (*result).dm = dm/trials;
    (*result).dl = dl/trials;
    (*result).md = md/trials;
    (*result).mm = mm/trials;
    (*result).ml = ml/trials;
    (*result).ld = ld/trials;
    (*result).lm = lm/trials;
    (*result).ll = ll/trials;
}
