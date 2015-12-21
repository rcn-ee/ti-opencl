#define CblasNoTrans 111
#define ONE_WI_PER_WG __attribute__((reqd_work_group_size(1, 1, 1)))

kernel ONE_WI_PER_WG void null() {}

kernel ONE_WI_PER_WG void K_cblas_dgemm_ocl(int Order, int TransA, int TransB, 
                          int M, int N, int K, double alpha, 
                          global double *A, int lda, 
                          global double *B, int ldb,
                          double beta,global double *C, int ldc,
                          int NUMAPANELS, int NUMBPANELS,
                          global double *MSMC_buf, local double *L2_buf, 
                          int msmc_size) 
{
    int chunks    = get_global_size(0);
    int id        = get_global_id(0);
    int mLocal    = M / chunks;
    int mLeft     = M-mLocal*chunks;
    int offset    = mLocal * id;

    double* L1_buf = (double*) 0x00F00000;
    if (msmc_size == 0)  MSMC_buf = (global double*) 0;

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

kernel ONE_WI_PER_WG void K_cblas_dgemm_omp(int Order, int TransA, int TransB, 
                          int M, int N, int K, double alpha, 
                          global double *A, int lda, 
                          global double *B, int ldb,
                          double beta,global double *C, int ldc,
                          int NUMAPANELS, int NUMBPANELS,
                          global double *MSMC_buf, local double *L2_buf, 
                          int msmc_size)
{
    
    cblas_dgemm_omp(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb,
                    beta, C, ldc, NUMAPANELS, NUMBPANELS,
                    MSMC_buf, L2_buf, msmc_size);
}
