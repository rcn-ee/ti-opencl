inline __kernel void simpleAdd(int veclen, __global TYPE1* c, int offset)
{
  // get the index of the test we are performing
  int index = get_global_id(0);
 
  c[index + offset * veclen] += c[index + offset * veclen];
}
