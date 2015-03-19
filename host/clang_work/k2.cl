#include "stdlib.h"
#include "dsp.h"

double sin(double);

kernel void VectorAdd(global double *a)
{
   a[0] = sin(a[0]);
}
