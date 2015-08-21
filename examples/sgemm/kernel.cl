#include "data.h"
#include "cblas.h"

void sgemm(
           int m, int n, int k,
           float alpha,
	   global float * a, int lda,
           global float * b, int ldb,
           float beta,
           global float * c, int ldc,
           int NUMAPANELS, int NUMBPANELS,
           float* pL1, local float* pL2, global float* pMsmc, int tid);

kernel __attribute__((reqd_work_group_size(1,1,1))) void 
K_ocl_sgemm_dsp(
                int m, int n, int k,
                float alpha,
                global float *a, int lda,
                global float *b, int ldb,
                float beta,
                global float *c, int ldc,
                int NUMAPANELS, int NUMBPANELS,
                local  float *L2_buf, global float *Msmc_buf)
{
    int chunks    = get_global_size(0);
    int id        = get_global_id(0);

    int mLocal    = m < chunks ? 1 : m / chunks;
    /* if not enough work for all cores, only first (chunks) cores compute */
    if (m < chunks && id >= m)  return;

    int offset    = mLocal * id;
    /* if m > chunks and (chunks) does not divide (m) evenly,
     * first (m % chunks) cores get one extra row to compute */
    if (m > chunks)
    {
        mLocal += (id < (m % chunks) ? 1 : 0);
        offset += (id < (m % chunks) ? id : (m % chunks));
    }

    float* L1_buf = (float*) 0x00F00000;
    if (Msmc_buf >= (global float*) 0x80000000)  Msmc_buf = (global float*) 0;
    __cache_l1d_16k();
    sgemm(mLocal, n, k,
          alpha, a + offset, lda, b, ldb, beta, c + offset, ldc,
          NUMAPANELS, NUMBPANELS,
          L1_buf, L2_buf, Msmc_buf, __core_num());
    __cache_l1d_all();
}

