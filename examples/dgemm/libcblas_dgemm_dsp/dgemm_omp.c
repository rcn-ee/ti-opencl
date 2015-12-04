#include "dsp_c.h"
#include "cblas.h"

extern void dgemm(int transa, int transb,
                  double alpha, double beta,
                  int m, int n, int k,
		  double* restrict a, int lda,
                  double* restrict b, int ldb,
                  double* restrict c, int ldc,
                  int NUMAPANELS, int NUMBPANELS,
                  double* restrict pL1, double* restrict pL2, 
                  double* restrict MSMC_buf, int tid);

extern int omp_get_num_threads (void);
extern int omp_get_thread_num  (void);

void cblas_dgemm_omp(int Order, int TransA, int TransB, 
                     int M, int N, int K, double alpha, 
                     double *A, int lda, 
                     double *B, int ldb,
                     double beta, double *C, int ldc,
                     int NUMAPANELS, int NUMBPANELS,
                     double *MSMC_buf, double *L2_buf, 
                     int msmc_size)
{
#pragma omp parallel
    {
        int chunks    = omp_get_num_threads();
        int id        = omp_get_thread_num();
        int mLocal    = M / chunks;
        int mLeft     = M-mLocal*chunks;
        int offset    = mLocal * id;

        double* L1_buf = (double*) 0x00F00000;
        if (msmc_size == 0)  MSMC_buf = NULL;

        if (id < mLeft) 
        {
            mLocal++;
            offset += id;
        }
        else offset += mLeft;

        __cache_l1d_16k();
        dgemm(TransA, TransB, alpha, beta,
                  mLocal, N, K,
                  A + (TransA == CblasNoTrans ? offset : offset * lda), lda,
                  B, ldb,
                  C + offset, ldc, 
                  NUMAPANELS, NUMBPANELS,
                  L1_buf, L2_buf, MSMC_buf, id);
        __cache_l1d_all();
    }
}
