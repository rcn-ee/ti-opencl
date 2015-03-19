void omp(int *p, int N)
{
    int i;

#pragma omp parallel for
    for (i = 0; i < N; ++i)
        p[i] += 1;
}
