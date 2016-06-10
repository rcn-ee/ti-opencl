kernel void devset(global char* buf)
{
  buf[get_global_id(0)] = 'x';
}
