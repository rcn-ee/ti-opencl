extern void abort();
extern void exit(int);

kernel void devset(global char* buf, int abort_gid)
{
  if (get_global_id(0) == abort_gid)  abort();
  buf[get_global_id(0)] = 'x';
}

kernel void devset_t(global char* buf, int size, int exit_gid)
{
  int i;
  for (i = 0; i < size; i++)
  {
    if (i == exit_gid)  exit(-42);
    buf[i] = 'x';
  }
}
