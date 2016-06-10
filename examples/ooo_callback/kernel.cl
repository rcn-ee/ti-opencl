kernel void compute(global int* buf, int size)
{
  for (int i = 0; i< size; ++i)
    buf[i] += 1;
}
