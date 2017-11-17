/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
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
#include "eve_builtins.h"

// EVE1, EVE2, EVE3, EVE4 => 0, 1, 2, 3
#define EVE1 0
#define EVE2 1
#define EVE3 2
#define EVE4 3
int  __eve_num()
{
  return EVECORE;
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
           "EVE %d: buf=0x%x len=%d a=%d %d %d %d %d %d %d "
           "h=%d %2.0f 0x%p %d %d %d %d o=%d %d %d %d %d %d "
           "u=%d %d %d %d %d %d aa=%d %d %d %d %d %d %d ah=%d %d\n",
           __eve_num(), buf, len,
           a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w,
           x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai);
}

