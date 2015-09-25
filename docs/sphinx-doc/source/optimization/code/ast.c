// Serves as bounds check
bool OutsideImage(int2 pos, int width, int height)
{
    if (pos.x < 1 || pos.y < 1)
        return true;

    if (pos.x >= width - 1 || pos.y >= height - 1)
        return true;

    return false;
}

kernel void gaussian_filter (global uchar* image,
                             global uchar* filtered_image,
                             global char*  gaussian_kernel,
                             global int*   image_dims,
                                    short   weight)
{
   const int image_height = image_dims[0];
   const int image_width = image_dims[1];
    
   const int global_x = get_global_id(0);
   const int global_y = get_global_id(1);
   const int2 pixel_pos = { global_x, global_y };

   if (OutsideImage(pixel_pos, image_width, image_height))
      return;

   short sum = 0;
   int index = 0;
   int2 pos;

   /* 3x3 Convolution */
   for(int y= -1; y<=1; y++)
      for(int x=-1; x<=1; x++)
      {
         pos = pixel_pos + (int2)(x,y);
         sum += gaussian_kernel[index++] * image[pos.y * image_width + pos.x];
      }

   sum /= weight;

   filtered_image[global_y * img_width + global_x] = (uchar) sum; 
}
