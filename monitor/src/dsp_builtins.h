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

#ifndef _DSP_BUILTINS_H_
#define _DSP_BUILTINS_H_

typedef void (*tiocl_dsp_builtin_kernel)();
extern tiocl_dsp_builtin_kernel tiocl_dsp_builtin_kernel_table[];

extern int  __dsp_num();
extern void tiocl_bik_null();
extern void tiocl_bik_memcpy_test(char *dst, char *src, int len);
extern void tiocl_bik_calling_conv_test(char *buf, short len, char a,
                                      short b, int c, char d, short e, int f,
                                      short g, char h, float i, int *j, int k,
                                      int l, int m, int n, int o, int p, int q,
                                      int r, int s, int t, int u, int v, int w,
                                      int x, int y, int z, int aa, int ab,
                                      int ac, int ad, int ae, int af, int ag,
                                      int ah, int ai);
extern void tiocl_bik_vecadd(int *a, int *b, int* c, int len);

extern void ocl_tidl_setup(void *,void *,void *,void *);
extern void ocl_tidl_initialize(void *,void *,void *,void *,void *);
extern void ocl_tidl_process(void *,void *,void *,uint32_t);
extern void ocl_tidl_cleanup();

#endif // _DSP_BUILTINS_H_

