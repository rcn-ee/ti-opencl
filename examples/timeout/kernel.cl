kernel void devset(global char* buf, int timeout_flag)
{
  if (timeout_flag == 70 && get_local_id(0) == 0)
  {
    __cycle_delay(210 * 750000);
    return;
  }

  buf[get_global_id(0)] = 'x';
}

kernel void devset_t(global char* buf, int size, int timeout_flag)
{
  if (timeout_flag == 70 && get_local_id(0) == 0)
  {
    __cycle_delay(210 * 750000);
    return;
  }

  int i;
  for (i = 0; i < size; i++)
    buf[i] = 'x';
}
