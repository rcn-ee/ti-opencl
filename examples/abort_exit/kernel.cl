extern void abort();
extern void exit(int);

kernel void devset(global char* buf, int abort_gid)
{
  if (abort_gid == 70 && get_local_id(0) == 0)
  {
    __cycle_delay(210 * 750000);
    return;
  }

  if (get_global_id(0) == abort_gid)  abort();
  buf[get_global_id(0)] = 'x';
}

kernel void devset_t(global char* buf, int size, int exit_gid)
{
  if (exit_gid == 70 && get_local_id(0) == 0)
  {
    __cycle_delay(210 * 750000);
    return;
  }

  int i;
  for (i = 0; i < size; i++)
  {
    if (i == exit_gid)  exit(-42);
    buf[get_global_id(0)] = 'x';
  }
}
