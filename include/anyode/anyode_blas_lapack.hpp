#pragma once
extern "C" void dgemv_(const char* trans, int* m, int* n, const double* alpha, const double* a, int* lda,
                       const double* x, int* incx, const double* beta, double* y, int* incy, int sundials__=0);
extern "C" void sgemv_(const char* trans, int* m, int* n, const float* alpha, const float* a, int* lda,
                       const float* x, int* incx, const float* beta, float* y, int* incy, int sundials__=0);

extern "C" void dgesvd_(const char* jobu, const char* jobvt, int* m, int* n, const double* a,
                        int* lda, double* s, double* u, int* ldu, double* vt, int* ldvt,
                        double* work, int* lwork, int* info );
extern "C" void sgesvd_(const char* jobu, const char* jobvt, int* m, int* n, const float* a,
                        int* lda, float* s, float* u, int* ldu, float* vt, int* ldvt,
                        float* work, int* lwork, int* info );
