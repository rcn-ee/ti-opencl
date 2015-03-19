#include "data.h"
#include "cblas.h"

void sgemm(int m, int n, int k,
	        global float * a, int lda,
            global float * b, int ldb,
            global float * c, int ldc,
            float* pL1, local float* pL2, global float* MSMC_buf,int tid);

kernel void ocl_matmpy(const global float *a, 
		               const global float *b, 
		                     global float *c, 
			                 int           m,
			                 int           n,
			                 int           k,
                             global float  *MSMC_buf,
			                 local  float  *L2_buf,
			                 int           msmc_size)
{
    int chunks    = get_global_size(0);
    int id        = get_global_id(0);
    int lda       = m;
    int ldb       = k;
    int ldc       = CORE_PROCESS_ROWS*((m+(CORE_PROCESS_ROWS-1))/CORE_PROCESS_ROWS);
    int mLocal    = m < chunks ? 1 : m / chunks;
    int offset    = mLocal * id;
    float* L1_buf = (float*) 0x00F00000;

    __cache_l1d_16k();
    sgemm(mLocal, n, k, a + offset, lda, b, ldb, c + offset, ldc, L1_buf, L2_buf, MSMC_buf, id);
    __cache_l1d_all();
}
