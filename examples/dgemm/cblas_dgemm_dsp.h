#ifndef CBLAS_DGEMM_DSP_H
#define CBLAS_DGEMM_DSP_H

extern "C"
void ocl_init(bool, int *);

extern "C"
void ocl_free();

extern "C"
void dsp_cblas_dgemm(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                     const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                     const int K, const double alpha, const double *A,
                     const int lda, const double *B, const int ldb,
                     const double beta, double *C, const int ldc);

#endif // CBLAS_DGEMM_DSP_H
