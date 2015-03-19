#include <c6x.h>
void c_vecadd1(const short* restrict a, const short* restrict  b, 
              short* restrict c, int N) 
{
    int i; 

    __x128_t * restrict na = (__x128_t *) &a;
    __x128_t * restrict nb = (__x128_t *) &b;
    __x128_t * restrict nc = (__x128_t *) &c;

    N >>= 3;

    for (i=0; i<N; ++i) 
    {
        __x128_t aval = na[i];
        __x128_t bval = nb[i];
        nc[i] = _llto128(_dadd2(_hi128(aval), _hi128(bval)),
                         _dadd2(_lo128(aval), _lo128(bval)));
    }
}

void c_vecadd0(const short* restrict a, const short* restrict  b, 
              short* restrict c, int N) 
{
}

void c_vecadd2(const short* restrict a, const short* restrict  b, 
              short* restrict c, int N) 
{
    int i; 

    long long  * restrict na = (long long  *) &a;
    long long  * restrict nb = (long long  *) &b;
    long long  * restrict nc = (long long  *) &c;

    N >>= 2;

    for (i=0; i<N; ++i) 
        nc[i] = _dadd2(na[i], nb[i]);
}

void c_vecadd(const short* restrict a, const short* restrict  b, 
              short* restrict c, int N) 
{
    int i; 

    for (i=0; i<N; ++i) 
        c[i] = a[i] + b[i];
}
