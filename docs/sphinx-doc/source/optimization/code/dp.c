inline int
dot_product (uchar4 mask, uchar4 data)
{
    int sum = 0;
    sum = (int) (mask.s0 * data.s0 +
		 mask.s1 * data.s1 + mask.s2 * data.s2 + mask.s3 * data.s3);
    return sum;
}

kernel __attribute__ ((reqd_work_group_size (1, 1, 1)))
void gaussian_filter (global const uchar4* restrict imgin_ptr,
		      global       uchar4* restrict imgout_ptr,
		      short                         width,
		      short                         pitch,
		      global const uchar*           kernel_coefficient,
		      short                         shift)
{

    int i;
    int sum0, sum1, sum2;
    int sum3;

    uchar4 mask1_0, mask2_0, mask3_0;
    uchar4 mask1_1, mask2_1, mask3_1;

    uchar4 r1_3210;
    uchar4 r2_3210;
    uchar4 r3_3210;
    uchar4 r1_5432;
    uchar4 r2_5432;
    uchar4 r3_5432;

    uchar8 r1_76543210, r2_76543210, r3_76543210;

    mask1_0 =
	(uchar4) (kernel_coefficient[0], kernel_coefficient[1],
		  kernel_coefficient[2], 0);
    mask2_0 =
	(uchar4) (kernel_coefficient[3], kernel_coefficient[4],
		  kernel_coefficient[5], 0);
    mask3_0 =
	(uchar4) (kernel_coefficient[6], kernel_coefficient[7],
		  kernel_coefficient[8], 0);

    mask1_1 =
	(uchar4) (0, kernel_coefficient[0], kernel_coefficient[1],
		  kernel_coefficient[2]);
    mask2_1 =
	(uchar4) (0, kernel_coefficient[3], kernel_coefficient[4],
		  kernel_coefficient[5]);
    mask3_1 =
	(uchar4) (0, kernel_coefficient[6], kernel_coefficient[7],
		  kernel_coefficient[8]);

    for (i = 0; i < width; i += 1)
    {
	  r1_76543210 = vload8 (i, imgin_ptr);
	  r1_76543210 = vload8 (pitch + i, imgin_ptr);
	  r1_76543210 = vload8 (2 * pitch + i, imgin_ptr);

	  r1_3210 = (uchar4) (r1_76543210.s0123);
	  r2_3210 = (uchar4) (r2_76543210.s0123);
	  r3_3210 = (uchar4) (r3_76543210.s0123);

	  sum0 = (dot_product (mask1_0, r1_3210) +
		  dot_product (mask2_0, r2_3210) +
		  dot_product (mask3_0, r3_3210)) >> shift;

	  sum1 = (dot_product (mask1_1, r1_3210) +
		  dot_product (mask2_1, r2_3210) +
		  dot_product (mask3_1, r3_3210)) >> shift;

	  r1_5432 = (uchar4) (r1_76543210.s2345);
	  r2_5432 = (uchar4) (r2_76543210.s2345);
	  r3_5432 = (uchar4) (r3_76543210.s2345);

	  sum2 = (dot_product (mask1_0, r1_5432) +
		  dot_product (mask2_0, r2_5432) +
		  dot_product (mask3_0, r3_5432)) >> shift;

	  sum3 = (dot_product (mask1_1, r1_5432) +
		  dot_product (mask2_1, r2_5432) +
		  dot_product (mask3_1, r3_5432)) >> shift;

	  imgout_ptr[i] = (uchar4) (sum0, sum1, sum2, sum3);
    }
}
