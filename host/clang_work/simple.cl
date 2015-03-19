#if 0
void VectorAdd(const int* a, const int* b, int* c) 
{
    for (int s0 = 0; s0 < *(((int*)0x00800000) + 4); ++s0)
    {
        int id = *(((int*)0x00800000) + 10) + s0;
        c[id] = a[id] + b[id];
    }
}

void VectorAdd1(const int* a, const int* b, int* c) 
{
    int index_temp       = *(((int*)0x00800000) + 10);
    int loop_upper_bound = *(((int*)0x00800000) + 4);

    for (int s0 = 0; s0 < loop_upper_bound; ++s0)
    {
        int id = index_temp + s0;
        c[id] = a[id] + b[id];
    }
}

void VectorAdd2(const int* restrict a, const int* restrict b, int* restrict c) 
{
    for (int s0 = 0; s0 < *(((int*)0x0080) + 4); ++s0)
    {
        int id = *(((int*)0x0080) + 10) + s0;
        c[id] = a[id] + b[id];
    }
}
#endif

void VectorAdd3(const int* restrict a, const int* restrict b, int* restrict c, int N) 
{
    for (int s0 = 0; s0 < N; ++s0)
        c[s0] = a[s0] + b[s0];
}
