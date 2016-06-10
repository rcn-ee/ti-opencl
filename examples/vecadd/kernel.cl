kernel void VectorAdd(global const short4* a,
                      global const short4* b,
                      global short4* c)
{
    int id = get_global_id(0);
    c[id] = a[id] + b[id];
}

