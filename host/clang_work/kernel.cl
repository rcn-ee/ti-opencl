//#include "stdlib.h"
//#include "dsp.h"



#if 1
typedef unsigned __INT32_TYPE__ uint;
typedef __typeof__(sizeof(int)) size_t;
typedef short short4 __attribute__((ext_vector_type(4)));

#define global __attribute__((address_space(1)))

size_t get_global_id(uint dimindx);
#endif

kernel void VectorAdd(global const short4* a, 
                      global const short4* b, 
                      global short4* c) 
{
    for (int s0 = 0; s0 < *(((int*)0x00800000) + 4); ++s0)
    {
        int id = *(((int*)0x00800000) + 10) + s0;
        c[id] = a[id] + b[id];
    }
}
