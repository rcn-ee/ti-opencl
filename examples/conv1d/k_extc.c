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
#include <stdbool.h>
#include <c6x.h>
#include "dsp_c.h"
#include "dsp_edmamgr.h"

void c_loop_simd_db_extc(int row_begin, int row_end,
                           float *restrict input,
                           float *restrict output,
                           float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch,
                           int BLOCK_HEIGHT,
                           float *restrict lInput, // double buffer
                           float *restrict lOutput // double buffer
                          )
{
  // partition rows into chunks, prefect next chunk, compute this chunk
  bool  first_block, last_block;
  int   row, b_row, block_height, next_b_row, next_block_height;
  float *in_buf0  = lInput;
  float *in_buf1  = lInput + BLOCK_HEIGHT * inPitch;
  float *out_buf0 = lOutput;
  float *out_buf1 = lOutput + BLOCK_HEIGHT * outPitch;

  EdmaMgr_Handle chan_in, chan_out;
  chan_in  = __ocl_EdmaMgr_alloc_intrakernel(1);
  chan_out = __ocl_EdmaMgr_alloc_intrakernel(1);
  if (! chan_in || ! chan_out)
  {
    printf("Failed to allocate edma handle.\n");
    return;
  }

  // fetch first block
  block_height = (row_begin + BLOCK_HEIGHT >= row_end) ? row_end - row_begin
                                                       : BLOCK_HEIGHT;
  EdmaMgr_copy2D1D(chan_in, &input[row_begin * inPitch], in_buf0,
                   COLS * sizeof(float), block_height,
                   inPitch *sizeof(float));

  // for each block
  for (b_row = row_begin; b_row < row_end; b_row += BLOCK_HEIGHT)
  {
    first_block  = (b_row == row_begin);
    last_block   = (b_row + BLOCK_HEIGHT >= row_end);
    block_height = (b_row + BLOCK_HEIGHT >= row_end) ? row_end - b_row
                                                     : BLOCK_HEIGHT;
    next_b_row      = b_row + block_height;
    next_block_height = (next_b_row + BLOCK_HEIGHT > row_end)
                        ? row_end - next_b_row : BLOCK_HEIGHT;

    // wait for prefecthed block to finish
    EdmaMgr_wait(chan_in);
    // prefetch next block
    if (! last_block)
      EdmaMgr_copy2D1D(chan_in, input + next_b_row * inPitch, in_buf1,
                   COLS * sizeof(float), next_block_height,
                   inPitch *sizeof(float));

    // for each row in the block: compute
    for (row = 0; row < block_height; row++)
    {
      int col;
      __float2_t v2_left_2  = _mem8_f2((void *) &in_buf0[row * inPitch + 2-2]);
      __float2_t v2_self    = _mem8_f2((void *) &in_buf0[row * inPitch + 2]);
      __float2_t v2_left_1  = _ftof2(_lof2(v2_self), _hif2(v2_left_2));
      __float2_t vf0 = _ftof2(filter[0], filter[0]);
      __float2_t vf1 = _ftof2(filter[1], filter[1]);
      __float2_t vf2 = _ftof2(filter[2], filter[2]);
      __float2_t vf3 = _ftof2(filter[3], filter[3]);
      __float2_t vf4 = _ftof2(filter[4], filter[4]);
      for (col = 2; col < COLS-2-1; col+=2)
      {
        __float2_t v2_right_2 = _mem8_f2((void *) &in_buf0[row*inPitch+col+2]);
        __float2_t v2_right_1 = _ftof2(_lof2(v2_right_2), _hif2(v2_self));

        _amem8_f2((void *) &out_buf0[row * outPitch + col]) =
                               _daddsp(
                                _daddsp(
                                  _daddsp(
                                    _daddsp(_dmpysp(v2_left_2, vf0),
                                            _dmpysp(v2_left_1, vf1)),
                                    _dmpysp(v2_self, vf2)),
                                  _dmpysp(v2_right_1, vf3)),
                                _dmpysp(v2_right_2, vf4));

        v2_left_2 = v2_self;
        v2_left_1 = v2_right_1;
        v2_self   = v2_right_2;
      }

      // boundary conditions
      // Alternatively, user can choose to pad the input data
      int extra_col = (col == COLS-3 ? 1 : 0);
      int boundaries[5] = { 0, 1, COLS-2, COLS-1, COLS-3 };
      int i;
      for (i = 0; i < 4 + extra_col; i++)
      {
        col = boundaries[i];
        float left_2  = in_buf0[row * inPitch + (col-2 < 0 ? 0 : col-2)];
        float left_1  = in_buf0[row * inPitch + (col-1 < 0 ? 0 : col-1)];
        float self    = in_buf0[row * inPitch + col];
        float right_1 = in_buf0[row * inPitch + (col+1 >= COLS ? COLS-1:col+1)];
        float right_2 = in_buf0[row * inPitch + (col+2 >= COLS ? COLS-1:col+2)];

        out_buf0[row * outPitch + col] = left_2 * filter[0]
                                     + left_1 * filter[1]
                                     + self * filter[2]
                                     + right_1 * filter[3]
                                     + right_2 * filter[4];
      }
    }

    // wait for previous store to finish
    if (! first_block)  EdmaMgr_wait(chan_out);
    // store block output back to output image
    EdmaMgr_copy1D2D(chan_out, out_buf0, output + b_row * outPitch,
                     COLS * sizeof(float), block_height,
                     outPitch * sizeof(float));

    // swap buffers for next block
    if (! last_block)
    {
      float *tmp = in_buf0;  in_buf0 = in_buf1;   in_buf1 = tmp;
                tmp = out_buf0; out_buf0 = out_buf1; out_buf1 = tmp;
    }
  }

  // wait for last block store to finish
  EdmaMgr_wait(chan_out);

  EdmaMgr_free(chan_in);
  EdmaMgr_free(chan_out);
}
