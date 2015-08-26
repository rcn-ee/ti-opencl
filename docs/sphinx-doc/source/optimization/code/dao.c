
#define ARRAY_SIZE n
#define NUM_BATCHES n_b
#define BATCH_SIZE (ARRAY_SIZE / NUM_BATCHES)	// This is the size of a single buffer
#define LOCAL_SIZE (BATCH_SIZE * 2)	// This is the size of the double buffer

kernel __attribute__ ((reqd_work_group_size (1, 1, 1)))
void gaussian_filter (global const uchar4 * restrict imgin_ptr,
	              global uchar4 * restrict imgout_ptr,
	              short width,
	              short pitch, 
                      global const uchar * kernel_coefficient, 
                      short shift)
{
    //Initialize the required variables

    //Copy content in the double buffer

    //Compute for the buffer in batch 1

    for (batch = 0; batch < NUM_BATCHES - 2; batch++)
    {
      if (batch % 2 == 0)
      {
	  Copy content in buffer batch 1 || Compute for buffer in batch 2
      }
      else
      {
	Copy content in buffer batch 2 || Compute for buffer in batch 1
      }
    }

    if (batch % 2 == 0)
    {
        Copy content in buffer batch 1 || Compute for buffer in batch 2
    }
    else
    {
        Copy content in buffer batch 2 || Compute for buffer in batch 1
    }
}
