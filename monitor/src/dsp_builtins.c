/******************************************************************************
 * Copyright (c) 2019, Texas Instruments Incorporated - http://www.ti.com/
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

#include <stdio.h>
#include <stdint.h>
#include "dsp_builtins.h"
#include "monitor.h"

extern int  __core_num() EXPORT;
extern int  __cache_l1d_16k() EXPORT;
extern int  __cache_l1d_all() EXPORT;
extern int  __cache_l2_64k()  EXPORT;
extern int  __cache_l2_128k() EXPORT;
extern void ocl_dsp_tidl_setup(void *,void *,void *,void *);
extern void ocl_dsp_tidl_initialize(void *,void *,void *,void *,void *);
extern void ocl_dsp_tidl_process(void *,void *,void *,uint32_t);
extern void ocl_dsp_tidl_cleanup();


void tiocl_bik_null()
{
}

void tiocl_bik_memcpy_test(char *dst, char *src, int len)
{
  memcpy(dst, src, len);
}

void tiocl_bik_calling_conv_test(char *buf, short len, char a,
                                 short b, int c, char d, short e, int f,
                                 short g, char h, float i, int *j, int k,
                                 int l, int m, int n, int o, int p, int q,
                                 int r, int s, int t, int u, int v, int w,
                                 int x, int y, int z, int aa, int ab,
                                 int ac, int ad, int ae, int af, int ag,
                                 int ah, int ai)
{
  snprintf(buf, len,
           "DSP %d: buf=0x%x len=%d a=%d %d %d %d %d %d %d "
           "h=%d %2.0f 0x%p %d %d %d %d o=%d %d %d %d %d %d "
           "u=%d %d %d %d %d %d aa=%d %d %d %d %d %d %d ah=%d %d\n",
           __core_num(), buf, len,
           a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w,
           x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai);
}

void tiocl_bik_vecadd(int *a, int *b, int* c, int len)
{
    int i;
    for (i=0; i<len; i++)
    {
        c[i] = a[i] + b[i];
    }
}

void ocl_tidl_setup(void *a, void *b, void *c, void *d)
{
    ocl_dsp_tidl_setup(a, b, c, d);
}

void ocl_tidl_initialize(void *a, void *b, void *c, void *d, void *e)
{
    // Set L1 cache to 16KB. TIDL requires 16KB of L1 scratch
    __cache_l1d_16k();
#if defined(DEVICE_AM572x)
    // Set L2 cache to 64KB. TIDL requires 148KB of L2 scratch
    // Other SoCs have enough L2 scratch, AM57x has to shrink L2 cache
    int ret = __cache_l2_64k();
#endif
    ocl_dsp_tidl_initialize(a, b, c, d, e);
}

void ocl_tidl_process(void *a, void *b, void *c, uint32_t d)
{
    ocl_dsp_tidl_process(a, b, c, d);
}

void ocl_tidl_cleanup()
{
    ocl_dsp_tidl_cleanup();
#if defined(DEVICE_AM572x)
    __cache_l2_128k();
#endif
    __cache_l1d_all();
}
