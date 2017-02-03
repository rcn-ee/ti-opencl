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

__kernel void null() { }


// 1D convolution applied to each row of an image, filter size 5x1
__kernel void k_conv1d_5x1(__global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch
                          )
{
  int col = get_global_id(0);
  int row = get_global_id(1);

  float left_2  = input[row * inPitch + (col-2 < 0 ? 0 : col-2)];
  float left_1  = input[row * inPitch + (col-1 < 0 ? 0 : col-1)];
  float self    = input[row * inPitch + col];
  float right_1 = input[row * inPitch + (col+1 >= COLS ? COLS-1 : col+1)];
  float right_2 = input[row * inPitch + (col+2 >= COLS ? COLS-1 : col+2)];
  
  // if symmetric filter, 2 multiplications can be optimized away
  output[row * outPitch + col] = left_2 * filter[0] + left_1 * filter[1]
                               + self * filter[2]
                               + right_1 * filter[3] + right_2 * filter[4];
}


__kernel void k_loop(__global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch
                          )
{
  int row = get_global_id(0);
  int col;

  // no boundary checks: col: [2, COLS-3]
  for (col = 2; col < COLS-2; col++)
  {
    float left_2  = input[row * inPitch + col-2];
    float left_1  = input[row * inPitch + col-1];
    float self    = input[row * inPitch + col];
    float right_1 = input[row * inPitch + col+1];
    float right_2 = input[row * inPitch + col+2];
    output[row * outPitch + col] = left_2 * filter[0] + left_1 * filter[1]
                                 + self * filter[2]
                                 + right_1 * filter[3] + right_2 * filter[4];
  }

  // boundary conditions
  // Alternatively, user can choose to pad the input data
  int boundaries[4] = { 0, 1, COLS-2, COLS-1 };
  for (int i = 0; i < 4; i++)
  {
    col = boundaries[i];
    float left_2  = input[row * inPitch + (col-2 < 0 ? 0 : col-2)];
    float left_1  = input[row * inPitch + (col-1 < 0 ? 0 : col-1)];
    float self    = input[row * inPitch + col];
    float right_1 = input[row * inPitch + (col+1 >= COLS ? COLS-1 : col+1)];
    float right_2 = input[row * inPitch + (col+2 >= COLS ? COLS-1 : col+2)];
    
    output[row * outPitch + col] = left_2 * filter[0] + left_1 * filter[1]
                                 + self * filter[2]
                                 + right_1 * filter[3] + right_2 * filter[4];
  }
}


__kernel void k_loop_simd(__global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch
                          )
{
  int row = get_global_id(0);
  int col;

  // _nassert(input % 8 == 0);
  // _nassert(output % 8 == 0);
  // _nassert(inPitch % 2 == 0);
  // _nassert(outPitch % 2 == 0);

#if 0
  // first version: SIMDize the computation
  for (col = 2; col < COLS-2-1; col+=2)
  {
    float2 v2_left_2  = *((float2 *) &input[row * inPitch + col-2]);
    float2 v2_self    = *((float2 *) &input[row * inPitch + col]);
    float2 v2_right_2 = *((float2 *) &input[row * inPitch + col+2]);

    float2 v2_left_1  = (float2) (v2_left_2.s1, v2_self.s0);
    float2 v2_right_1 = (float2) (v2_self.s1, v2_right_2.s0);

    * ((float2 *) &output[row * outPitch + col]) =
                                (v2_left_2 + v2_right_2) * filter[0]
                              + (v2_left_1 + v2_right_1) * filter[1]
                              + v2_self * filter[2];
  }
#else
  // second version: pipeline the memory loads
  float2 v2_left_2  = *((float2 *) &input[row * inPitch + 2-2]);
  float2 v2_self    = *((float2 *) &input[row * inPitch + 2]);
  float2 v2_left_1  = (float2) (v2_left_2.s1, v2_self.s0);
  for (col = 2; col < COLS-2-1; col+=2)
  {
    float2 v2_right_2 = *((float2 *) &input[row * inPitch + col+2]);
    float2 v2_right_1 = (float2) (v2_self.s1, v2_right_2.s0);

    * ((float2 *) &output[row * outPitch + col]) =
                                v2_left_2 * filter[0] + v2_left_1 * filter[1]
                              + v2_self * filter[2]
                              + v2_right_1 * filter[3] + v2_right_2 * filter[4];

    v2_left_2 = v2_self;
    v2_left_1 = v2_right_1;
    v2_self   = v2_right_2;
  }
#endif

  // boundary conditions
  // Alternatively, user can choose to pad the input data
  int extra_col = (col == COLS-3 ? 1 : 0);
  int boundaries[5] = { 0, 1, COLS-2, COLS-1, COLS-3 };
  for (int i = 0; i < 4 + extra_col; i++)
  {
    col = boundaries[i];
    float left_2  = input[row * inPitch + (col-2 < 0 ? 0 : col-2)];
    float left_1  = input[row * inPitch + (col-1 < 0 ? 0 : col-1)];
    float self    = input[row * inPitch + col];
    float right_1 = input[row * inPitch + (col+1 >= COLS ? COLS-1 : col+1)];
    float right_2 = input[row * inPitch + (col+2 >= COLS ? COLS-1 : col+2)];
    
    output[row * outPitch + col] = left_2 * filter[0] + left_1 * filter[1]
                                 + self * filter[2]
                                 + right_1 * filter[3] + right_2 * filter[4];
  }
}


__kernel __attribute__((reqd_work_group_size(1,1,1)))
void k_loop_db(__global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch,
                           int BLOCK_HEIGHT,
                           __local float *restrict lInput, // double buffer
                           __local float *restrict lOutput // double buffer
                          )
{
  // get the rows that this workgroup needs to process
  int gid = get_global_id(0);
  int gsz = get_global_size(0);
  int row_begin = gid * (ROWS / gsz);
  int row_end   = (gid + 1) * (ROWS / gsz);   // exclusive
  if (gid == gsz - 1)  row_end = ROWS;        // exclusive

  // partition rows into chunks, prefect next chunk, compute this chunk
  bool  first_block, last_block;
  int   b_row, block_height, next_b_row, next_block_height;
  local float *in_buf0  = lInput;
  local float *in_buf1  = lInput + BLOCK_HEIGHT * inPitch;
  local float *out_buf0 = lOutput;
  local float *out_buf1 = lOutput + BLOCK_HEIGHT * outPitch;
  event_t ev_in0, ev_in1, ev_out0, ev_out1;

  // fetch first block
  block_height = (row_begin + BLOCK_HEIGHT >= row_end) ? row_end - row_begin
                                                       : BLOCK_HEIGHT;
  ev_in0 = async_work_group_copy(in_buf0, &input[row_begin * inPitch],
                                 block_height * inPitch, 0);

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

    // prefetch next block
    if (! last_block)
      ev_in1 = async_work_group_copy(in_buf1, input + next_b_row * inPitch,
                                     next_block_height * inPitch, 0);
    // wait for prefecthed block to finish
    wait_group_events(1, &ev_in0);
    ev_in0 = ev_in1;

    // for each row in the block: compute
    for (int row = 0; row < block_height; row++)
    {
      int col;
      // no boundary checks: col: [2, COLS-3]
      for (col = 2; col < COLS-2; col++)
      {
        float left_2  = in_buf0[row * inPitch + col-2];
        float left_1  = in_buf0[row * inPitch + col-1];
        float self    = in_buf0[row * inPitch + col];
        float right_1 = in_buf0[row * inPitch + col+1];
        float right_2 = in_buf0[row * inPitch + col+2];
        out_buf0[row * outPitch + col] = left_2 * filter[0]
                                     + left_1 * filter[1]
                                     + self * filter[2]
                                     + right_1 * filter[3]
                                     + right_2 * filter[4];
      }

      // boundary conditions
      // Alternatively, user can choose to pad the input data
      int boundaries[4] = { 0, 1, COLS-2, COLS-1 };
      for (int i = 0; i < 4; i++)
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

    // store block output back to output image
    ev_out1 = async_work_group_copy(output + b_row * outPitch, out_buf0,
                                    block_height * outPitch, 0);
    // wait for previous store to finish
    if (! first_block)  wait_group_events(1, &ev_out0);
    ev_out0 = ev_out1;

    // swap buffers for next block
    if (! last_block)
    {
      local float *tmp = in_buf0;  in_buf0 = in_buf1;   in_buf1 = tmp;
                  tmp = out_buf0; out_buf0 = out_buf1; out_buf1 = tmp;
    }
  }

  // wait for last block store to finish
  wait_group_events(1, &ev_out0);
}


__kernel __attribute__((reqd_work_group_size(1,1,1)))
void k_loop_simd_db(__global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch,
                           int BLOCK_HEIGHT,
                           __local float *restrict lInput, // double buffer
                           __local float *restrict lOutput // double buffer
                          )
{
  // get the rows that this workgroup needs to process
  int gid = get_global_id(0);
  int gsz = get_global_size(0);
  int row_begin = gid * (ROWS / gsz);
  int row_end   = (gid + 1) * (ROWS / gsz);   // exclusive
  if (gid == gsz - 1)  row_end = ROWS;        // exclusive

  // partition rows into chunks, prefect next chunk, compute this chunk
  bool  first_block, last_block;
  int   b_row, block_height, next_b_row, next_block_height;
  local float *in_buf0  = lInput;
  local float *in_buf1  = lInput + BLOCK_HEIGHT * inPitch;
  local float *out_buf0 = lOutput;
  local float *out_buf1 = lOutput + BLOCK_HEIGHT * outPitch;
  event_t ev_in0, ev_in1, ev_out0, ev_out1;

  // fetch first block
  block_height = (row_begin + BLOCK_HEIGHT >= row_end) ? row_end - row_begin
                                                       : BLOCK_HEIGHT;
  ev_in0 = async_work_group_copy(in_buf0, &input[row_begin * inPitch],
                                 block_height * inPitch, 0);

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

    // prefetch next block
    if (! last_block)
      ev_in1 = async_work_group_copy(in_buf1, input + next_b_row * inPitch,
                                     next_block_height * inPitch, 0);
    // wait for prefecthed block to finish
    wait_group_events(1, &ev_in0);
    ev_in0 = ev_in1;

    // for each row in the block: compute
    for (int row = 0; row < block_height; row++)
    {
      int col;
      float2 v2_left_2  = *((float2 *) &in_buf0[row * inPitch + 2-2]);
      float2 v2_self    = *((float2 *) &in_buf0[row * inPitch + 2]);
      float2 v2_left_1  = (float2) (v2_left_2.s1, v2_self.s0);
      for (col = 2; col < COLS-2-1; col+=2)
      {
        float2 v2_right_2 = *((float2 *) &in_buf0[row * inPitch + col+2]);
        float2 v2_right_1 = (float2) (v2_self.s1, v2_right_2.s0);

        * ((float2 *) &out_buf0[row * outPitch + col]) =
                                    v2_left_2 * filter[0]
                                  + v2_left_1 * filter[1]
                                  + v2_self * filter[2]
                                  + v2_right_1 * filter[3]
                                  + v2_right_2 * filter[4];

        v2_left_2 = v2_self;
        v2_left_1 = v2_right_1;
        v2_self   = v2_right_2;
      }

      // boundary conditions
      // Alternatively, user can choose to pad the input data
      int extra_col = (col == COLS-3 ? 1 : 0);
      int boundaries[5] = { 0, 1, COLS-2, COLS-1, COLS-3 };
      for (int i = 0; i < 4 + extra_col; i++)
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

    // store block output back to output image
    ev_out1 = async_work_group_copy(output + b_row * outPitch, out_buf0,
                                    block_height * outPitch, 0);
    // wait for previous store to finish
    if (! first_block)  wait_group_events(1, &ev_out0);
    ev_out0 = ev_out1;

    // swap buffers for next block
    if (! last_block)
    {
      local float *tmp = in_buf0;  in_buf0 = in_buf1;   in_buf1 = tmp;
                  tmp = out_buf0; out_buf0 = out_buf1; out_buf1 = tmp;
    }
  }

  // wait for last block store to finish
  wait_group_events(1, &ev_out0);
}


extern void c_loop_simd_db_extc(int row_begin, int row_end,
                           __global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch,
                           int BLOCK_HEIGHT,
                           __local float *restrict lInput, // double buffer
                           __local float *restrict lOutput // double buffer
                          );

__kernel __attribute__((reqd_work_group_size(1,1,1)))
void k_loop_simd_db_extc(__global float *restrict input,
                           __global float *restrict output,
                           __global float *restrict filter,
                           int COLS,
                           int ROWS,
                           int inPitch,
                           int outPitch,
                           int BLOCK_HEIGHT,
                           __local float *restrict lInput, // double buffer
                           __local float *restrict lOutput // double buffer
                          )
{
  // get the rows that this workgroup needs to process
  int gid = get_global_id(0);
  int gsz = get_global_size(0);
  int row_begin = gid * (ROWS / gsz);
  int row_end   = (gid + 1) * (ROWS / gsz);   // exclusive
  if (gid == gsz - 1)  row_end = ROWS;        // exclusive

  c_loop_simd_db_extc(row_begin, row_end,
                          input, output, filter,
                          COLS, ROWS, inPitch, outPitch,
                          BLOCK_HEIGHT, lInput, lOutput);
}

