float image[image_size];
float gaussian_kernel[9];
float weight;
float filtered_image[(image_width - 2) * (image_height - 2)];

for (i = 1; i < img_height - 1; i++)
{
    for (j = 1; j < img_width - 1; j++)
    {
	sum = 0;
	for (p = 0; p < 3; p++)
        {
	    for (q = 0; q < 3; q++)
            {
		sum += image[(i + p) * img_width + j + q] * 
                       gaussian_kernel[p * 3 + q];
            }
	}
	sum /= weight;
	filtered_image[(i - 1) * img_width + j] = sum;
    }
}
