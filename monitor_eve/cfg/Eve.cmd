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

--retain="printf"
--retain="puts"

/* Grouping kept the same as those in TIDL linker command file,   */
/*     for relative addressing, internal memory (DMEM) placement, */
/*     better L1P$ locality, etc.                                 */
SECTIONS
{
  /* With updated secondary boot loader (SBL) library, vector table
     and entry point need to be located in the same 1MB page, with
     vector table placed at the beginning of 1MB page */
  .entry_point_page
  {
    * (.vecs)
    * (.text:_c_int00)
  } align = 0x100000 > VECSMEM PAGE 1

  .tidl_lib_txtdata
  {
    *dmautils.lib<*.o*> (.text)
    *tidl_algo.lib<tidl_conv2d*.o*> (.text)
    *tidl_algo.lib<*kernel*.o*> (.text)
    *tidl_algo.lib<*alg*.o*> (.text)
  }   align = 0x8000 > CODEMEM  PAGE 1

  GROUP
  {
      .bss            /* This order facilitates a single segment for */
      .data           /* GDP-relative addressing                     */
      .rodata
  }  > DATAMEM PAGE 1

  .tidl_lib_fardata
  {
    *tidl_algo.lib<*.o*> (.far)
    *tidl_algo.lib<*.o*> (.const)
  }  > DMEM  PAGE 1

  .edma_utils_lib_fardata
  {
    *dmautils.lib<*.o*> (.far)
    *dmautils.lib<*.o*> (.bss)
    *dmautils.lib<*.o*> (.const)
  }  > DMEM  PAGE 1
}

