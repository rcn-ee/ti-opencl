#include <cassert>
#include <cstdlib>

const int DIM       = 16;
const int mat_N     = DIM;
const int mat_K     = DIM;
const int mat_M     = DIM;

void mat_mpy(const float *A, const float *B, float *C, int mat_N,
             int mat_K, int mat_M)
{
    for (int col = 0; col < mat_M; ++col)
        for (int row = 0; row < mat_N; ++row)
        {
            C[row*mat_M+col] = 0;
            for (int i = 0; i < mat_K; ++i)
                C[row*mat_M+col] += A[row*mat_K+i] * B[i*mat_M+col];
        }
}


int main(int argc, char *argv[])
{
   size_t mat_size = DIM * DIM * sizeof(float);

   // Allocate matrices
   float *A      = (float *) malloc(mat_size);
   float *B      = (float *) malloc(mat_size);
   float *C      = (float *) malloc(mat_size);

   assert(A != nullptr && B != nullptr && C != nullptr && C != nullptr);

   // Initialize matrices
   srand(42);
   for (int i=0; i < mat_N * mat_K; ++i) A[i] = rand() % 5 + 1;
   for (int i=0; i < mat_K * mat_M; ++i) B[i] = rand() % 5 + 1;
   for (int i=0; i < mat_N * mat_M; ++i) C[i] = 0.0;

   // Multiple matrices C = A x B
   mat_mpy(A, B, C, mat_N, mat_K, mat_M);

   free(A);
   free(B);
   free(C);

   return 0;
}
