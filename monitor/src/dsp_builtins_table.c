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

#include <stdint.h>
#include "dsp_builtins.h"

tiocl_dsp_builtin_kernel tiocl_dsp_builtin_kernel_table[] =
{
  // 0-9: reserved for OpenCL DSP runtime testing
  (tiocl_dsp_builtin_kernel) tiocl_bik_null,
  (tiocl_dsp_builtin_kernel) tiocl_bik_memcpy_test,
  (tiocl_dsp_builtin_kernel) tiocl_bik_calling_conv_test,
  (tiocl_dsp_builtin_kernel) tiocl_bik_vecadd,
  (tiocl_dsp_builtin_kernel) 0,     // opencl runtime reserved
  (tiocl_dsp_builtin_kernel) 0,
  (tiocl_dsp_builtin_kernel) 0,
  (tiocl_dsp_builtin_kernel) 0,
  (tiocl_dsp_builtin_kernel) 0,
  (tiocl_dsp_builtin_kernel) 0,

  // Intended for user callable functions
  // 10-13: tidl
  (tiocl_dsp_builtin_kernel) ocl_tidl_setup,
  (tiocl_dsp_builtin_kernel) ocl_tidl_initialize,
  (tiocl_dsp_builtin_kernel) ocl_tidl_process,
  (tiocl_dsp_builtin_kernel) ocl_tidl_cleanup,
};

