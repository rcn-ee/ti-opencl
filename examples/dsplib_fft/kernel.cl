void DSPF_sp_fftSPxSP(int N,
    float *x, global float *w, float *y,
    unsigned char *brev, int n_min, int offset, int n_max);

kernel void null(void)
{
}


kernel __attribute__((reqd_work_group_size(1,1,1)))
void ocl_DSPF_sp_fftSPxSP(int N,
    global float *x, global float *w, global float *y,
    int n_min, int offset, int n_max, int num_channels)
{
  // get the channels that this workgroup needs to process
  int gid = get_global_id(0);
  int gsz = get_global_size(0);
  int ch_begin = gid * (num_channels / gsz);
  int ch_end   = (gid + 1) * (num_channels / gsz);   // exclusive
  if (gid == gsz - 1)  ch_end = num_channels;        // exclusive

  for (int i = ch_begin; i < ch_end; i++)
  {
    DSPF_sp_fftSPxSP(N, (float *) x+i*2*N, w, (float *) y+i*2*N,
                     0, n_min, offset, n_max);
  }
}


kernel __attribute__((reqd_work_group_size(1,1,1)))
void ocl_DSPF_sp_fftSPxSP_db(int N,
    global float *restrict x,
    global float *restrict w,
    global float *restrict y,
    int n_min,
    int offset,
    int n_max,
    int num_channels,
    int BLOCK_HEIGHT,              // number of channels for each block
    local float *restrict lInput,  // double buffer
    local float *restrict lOutput  // double buffer
    )
{
  // get the channels that this workgroup needs to process
  int gid = get_global_id(0);
  int gsz = get_global_size(0);
  int ch_begin = gid * (num_channels / gsz);
  int ch_end   = (gid + 1) * (num_channels / gsz);   // exclusive
  if (gid == gsz - 1)  ch_end = num_channels;        // exclusive

  // partition channels into chunks, prefect next chunk, compute this chunk
  bool  first_block, last_block;
  int   b_ch, block_height, next_b_ch, next_block_height;
  int   inPitch  = 2 * N;
  int   outPitch = 2 * N;
  local float *in_buf0  = lInput;
  local float *in_buf1  = lInput + BLOCK_HEIGHT * inPitch;
  local float *out_buf0 = lOutput;
  local float *out_buf1 = lOutput + BLOCK_HEIGHT * outPitch;
  event_t ev_in0, ev_in1, ev_out0, ev_out1;

  // fetch first block
  block_height = (ch_begin + BLOCK_HEIGHT >= ch_end) ? ch_end - ch_begin
                                                       : BLOCK_HEIGHT;
  ev_in0 = async_work_group_copy(in_buf0, x + ch_begin * inPitch,
                                 block_height * inPitch, 0);

  // for each block
  for (b_ch = ch_begin; b_ch < ch_end; b_ch += BLOCK_HEIGHT)
  {
    first_block  = (b_ch == ch_begin);
    last_block   = (b_ch + BLOCK_HEIGHT >= ch_end);
    block_height = (b_ch + BLOCK_HEIGHT >= ch_end) ? ch_end - b_ch
                                                     : BLOCK_HEIGHT;
    next_b_ch    = b_ch + block_height;
    next_block_height = (next_b_ch + BLOCK_HEIGHT > ch_end)
                        ? ch_end - next_b_ch : BLOCK_HEIGHT;

    // prefetch next block
    if (! last_block)
      ev_in1 = async_work_group_copy(in_buf1, x + next_b_ch * inPitch,
                                     next_block_height * inPitch, 0);
    // wait for prefecthed block to finish
    wait_group_events(1, &ev_in0);
    ev_in0 = ev_in1;

    // for each channel in the block: compute
    for (int ch = 0; ch < block_height; ch++)
    {
      DSPF_sp_fftSPxSP(N, (float *) in_buf0+ch*inPitch, w,
                       (float *) out_buf0+ch*outPitch,
                       0, n_min, offset, n_max);
    }

    // store block output back to output image
    ev_out1 = async_work_group_copy(y + b_ch * outPitch, out_buf0,
                                    block_height * outPitch, 0);
    // wait for previous store to finish
    if (! first_block)  wait_group_events(1, &ev_out0);
    ev_out0 = ev_out1;

    // swap buffers for next block
    if (! last_block)
    {
      local float *tmp = in_buf0;   in_buf0 = in_buf1;   in_buf1 = tmp;
                   tmp = out_buf0; out_buf0 = out_buf1; out_buf1 = tmp;
    }
  }

  // wait for last block store to finish
  wait_group_events(1, &ev_out0);
}

