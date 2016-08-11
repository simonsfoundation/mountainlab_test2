/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/11/2016
*******************************************************/

#include "pca.h"
#include "mlcommon.h"

void iterate_to_get_top_component(Mda& C, double& sigma, Mda& X, int num_iterations);
void iterate_to_get_top_component(Mda32& C, double& sigma, Mda32& X, int num_iterations);
void iterate_XXt_to_get_top_component(Mda& C, double& sigma, Mda& XXt, int num_iterations);
void iterate_XXt_to_get_top_component(Mda32& C, double& sigma, Mda32& XXt, int num_iterations);
Mda mult_AB(Mda& A, Mda& B);
Mda32 mult_AB(Mda32& A, Mda32& B);
Mda mult_AtransB(Mda& A, Mda& B);
Mda32 mult_AtransB(Mda32& A, Mda32& B);
Mda mult_ABtrans(Mda& A, Mda& B);
void subtract_out_rank_1(Mda& X, Mda& C);
void subtract_out_rank_1(Mda32& X, Mda32& C);
void subtract_out_rank_1_from_XXt(Mda& X, Mda& C);
void subtract_out_rank_1_from_XXt(Mda32& X, Mda32& C);
void normalize_vector(Mda& V);

void pca(Mda& C, Mda& F, Mda& sigma, Mda& X, int num_features)
{
    long M = X.N1();
    //long N = X.N2();
    long K = num_features;
    long num_iterations_per_component = 10; //hard-coded for now

    Mda Xw = X; //working data

    C.allocate(M, K);
    sigma.allocate(K, 1);

    for (long k = 0; k < K; k++) {
        // C will be Mx1, F will be 1xN
        Mda C0;
        double sigma0;
        iterate_to_get_top_component(C0, sigma0, Xw, num_iterations_per_component);
        for (long m = 0; m < M; m++) {
            C.set(C0.get(m), m, k); //think about speeding this up
        }
        sigma.set(sigma0, k);
        subtract_out_rank_1(Xw, C0);
    }

    F = mult_AtransB(C, X);
}

void pca(Mda32& C, Mda32& F, Mda32& sigma, Mda32& X, int num_features)
{
    long M = X.N1();
    //long N = X.N2();
    long K = num_features;
    long num_iterations_per_component = 10; //hard-coded for now

    Mda32 Xw = X; //working data

    C.allocate(M, K);
    sigma.allocate(K, 1);

    for (long k = 0; k < K; k++) {
        // C will be Mx1, F will be 1xN
        Mda32 C0;
        double sigma0;
        iterate_to_get_top_component(C0, sigma0, Xw, num_iterations_per_component);
        for (long m = 0; m < M; m++) {
            C.set(C0.get(m), m, k); //think about speeding this up
        }
        sigma.set(sigma0, k);
        subtract_out_rank_1(Xw, C0);
    }

    F = mult_AtransB(C, X);
}

void pca_from_XXt(Mda& C, Mda& sigma, Mda& XXt, int num_features)
{
    long M = XXt.N1();
    long K = num_features;
    long num_iterations_per_component = 10; //hard-coded for now

    Mda XXtw = XXt; //working data

    C.allocate(M, K);
    sigma.allocate(K, 1);

    for (long k = 0; k < K; k++) {
        // C will be Mx1, F will be 1xN
        Mda C0;
        double sigma0;
        iterate_XXt_to_get_top_component(C0, sigma0, XXtw, num_iterations_per_component);
        for (long m = 0; m < M; m++) {
            C.set(C0.get(m), m, k); //think about speeding this up
        }
        sigma.set(sigma0, k);
        subtract_out_rank_1_from_XXt(XXtw, C0);
    }
}

void pca_from_XXt(Mda32& C, Mda32& sigma, Mda32& XXt, int num_features)
{
    long M = XXt.N1();
    long K = num_features;
    long num_iterations_per_component = 10; //hard-coded for now

    Mda32 XXtw = XXt; //working data

    C.allocate(M, K);
    sigma.allocate(K, 1);

    for (long k = 0; k < K; k++) {
        // C will be Mx1, F will be 1xN
        Mda32 C0;
        double sigma0;
        iterate_XXt_to_get_top_component(C0, sigma0, XXtw, num_iterations_per_component);
        for (long m = 0; m < M; m++) {
            C.set(C0.get(m), m, k); //think about speeding this up
        }
        sigma.set(sigma0, k);
        subtract_out_rank_1_from_XXt(XXtw, C0);
    }
}

Mda mult_AB(Mda& A, Mda& B)         // gemm for two 2D MDAs.   inner part should be BLAS3 call
{
    long M = A.N1();
    long L = A.N2();
    long N = B.N2();
    if (B.N1() != L) {
        qCritical() << "Unexpected dimensions in mult_AB" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda C(M, N);
    double* Aptr = A.dataPtr();
    double* Bptr = B.dataPtr();
    double* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            for (long l = 0; l < L; l++) {
                Cptr[iC] += Aptr[m + l * M] * Bptr[l + L * n];
            }
            iC++;
        }
    }
    return C;
}

Mda32 mult_AB(Mda32& A, Mda32& B)        // gemm for two 2D MDAs.   inner part should be BLAS3 call
{
    long M = A.N1();
    long L = A.N2();
    long N = B.N2();
    if (B.N1() != L) {
        qCritical() << "Unexpected dimensions in mult_AB" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda32 C(M, N);
    dtype32* Aptr = A.dataPtr();
    dtype32* Bptr = B.dataPtr();
    dtype32* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            for (long l = 0; l < L; l++) {
                Cptr[iC] += Aptr[m + l * M] * Bptr[l + L * n];
            }
            iC++;
        }
    }
    return C;
}

Mda mult_AtransB(Mda& A, Mda& B)    // gemm for two 2D MDAs.   inner part should be BLAS3 call
{
    long M = A.N2();
    long L = A.N1();
    long N = B.N2();
    if (B.N1() != L) {
        qCritical() << "Unexpected dimensions in mult_AtransB" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda C(M, N);
    double* Aptr = A.dataPtr();
    double* Bptr = B.dataPtr();
    double* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            double val = MLCompute::dotProduct(L, &Aptr[L * m], &Bptr[L * n]);
            Cptr[iC] = val;
            iC++;
        }
    }
    return C;
}

Mda32 mult_AtransB(Mda32& A, Mda32& B)    // gemm for two 2D MDAs.   inner part should be BLAS3 call
{
    long M = A.N2();
    long L = A.N1();
    long N = B.N2();
    if (B.N1() != L) {
        qCritical() << "Unexpected dimensions in mult_AtransB" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda32 C(M, N);
    dtype32* Aptr = A.dataPtr();
    dtype32* Bptr = B.dataPtr();
    dtype32* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            double val = MLCompute::dotProduct(L, &Aptr[L * m], &Bptr[L * n]);
            Cptr[iC] = val;
            iC++;
        }
    }
    return C;
}

Mda mult_ABtrans(Mda& A, Mda& B)    // gemm for two 2D MDAs.   inner part should be BLAS3 call
{
    long M = A.N1();
    long L = A.N2();
    long N = B.N1();
    if (B.N2() != L) {
        qCritical() << "Unexpected dimensions in mult_ABtrans" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda C(M, N);
    double* Aptr = A.dataPtr();
    double* Bptr = B.dataPtr();
    double* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            for (long l = 0; l < L; l++) {
                Cptr[iC] += Aptr[m + l * M] * Bptr[n + l * N];
            }
            iC++;
        }
    }
    return C;
}

Mda32 mult_ABtrans(Mda32& A, Mda32& B)    // gemm for two 2D MDAs.   inner part should be BLAS3 call
{
    long M = A.N1();
    long L = A.N2();
    long N = B.N1();
    if (B.N2() != L) {
        qCritical() << "Unexpected dimensions in mult_ABtrans" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda32 C(M, N);
    dtype32* Aptr = A.dataPtr();
    dtype32* Bptr = B.dataPtr();
    dtype32* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            for (long l = 0; l < L; l++) {
                Cptr[iC] += Aptr[m + l * M] * Bptr[n + l * N];
            }
            iC++;
        }
    }
    return C;
}

void subtract_out_rank_1(Mda& X, Mda& C)
{
    long M = X.N1();
    long N = X.N2();
    if ((C.N1() != M) || (C.N2() != 1)) {
        qCritical() << "Incorrect dimensions in subtract_out_rank_1" << M << N << C.N1() << C.N2();
        abort();
    }
    double* Xptr = X.dataPtr();
    double* Cptr = C.dataPtr();
    long iX = 0;
    for (long n = 0; n < N; n++) {
        double dp = MLCompute::dotProduct(M, &Xptr[iX], Cptr);
        for (long m = 0; m < M; m++) {
            Xptr[iX] -= dp * Cptr[m];
            iX++;
        }
    }
}

void subtract_out_rank_1(Mda32& X, Mda32& C)
{
    long M = X.N1();
    long N = X.N2();
    if ((C.N1() != M) || (C.N2() != 1)) {
        qCritical() << "Incorrect dimensions in subtract_out_rank_1" << M << N << C.N1() << C.N2();
        abort();
    }
    dtype32* Xptr = X.dataPtr();
    dtype32* Cptr = C.dataPtr();
    long iX = 0;
    for (long n = 0; n < N; n++) {
        double dp = MLCompute::dotProduct(M, &Xptr[iX], Cptr);
        for (long m = 0; m < M; m++) {
            Xptr[iX] -= dp * Cptr[m];
            iX++;
        }
    }
}

void subtract_out_rank_1_from_XXt(Mda& XXt, Mda& C)
{
    long M = XXt.N1();
    if ((C.N1() != M) || (C.N2() != 1)) {
        qCritical() << "Incorrect dimensions in subtract_out_rank_1_from_XXt" << M << C.N1() << C.N2();
        abort();
    }

    // X -> (1-CC')X
    // XXt -> (1-CC')XXt(1-CC')

    Mda B(M, M); //1-CC'
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < M; i++) {
            if (i == j)
                B.setValue(1 - C.value(i) * C.value(j), i, j);
            else
                B.setValue(-C.value(i) * C.value(j), i, j);
        }
    }
    Mda tmp = mult_AB(XXt, B);
    XXt = mult_AB(B, tmp);
}

void subtract_out_rank_1_from_XXt(Mda32& XXt, Mda32& C)
{
    long M = XXt.N1();
    if ((C.N1() != M) || (C.N2() != 1)) {
        qCritical() << "Incorrect dimensions in subtract_out_rank_1_from_XXt" << M << C.N1() << C.N2();
        abort();
    }

    // X -> (1-CC')X
    // XXt -> (1-CC')XXt(1-CC')

    Mda32 B(M, M); //1-CC'
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < M; i++) {
            if (i == j)
                B.setValue(1 - C.value(i) * C.value(j), i, j);
            else
                B.setValue(-C.value(i) * C.value(j), i, j);
        }
    }
    Mda32 tmp = mult_AB(XXt, B);
    XXt = mult_AB(B, tmp);
}

void normalize_vector(Mda& V)
{
    long N = V.totalSize();
    double* Vptr = V.dataPtr();
    double norm = MLCompute::norm(N, Vptr);
    if (!norm)
        return;
    for (long n = 0; n < N; n++)
        Vptr[n] /= norm;
}

void normalize_vector(Mda32& V)
{
    long N = V.totalSize();
    dtype32* Vptr = V.dataPtr();
    double norm = MLCompute::norm(N, Vptr);
    if (!norm)
        return;
    for (long n = 0; n < N; n++)
        Vptr[n] /= norm;
}

void matvec(long M, long N, double* ret, double* A, double* x)   // really this should be BLAS2 call
{
    for (long m = 0; m < M; m++)
        ret[m] = 0;
    long iA = 0;
    for (long n = 0; n < N; n++) {
        double xval = x[n];
        for (long m = 0; m < M; m++) {
            ret[m] += A[iA] * xval;
            iA++;
        }
    }
}

void matvec(long M, long N, float* ret, float* A, float* x)   // really this should be BLAS2 call
{
    for (long m = 0; m < M; m++)
        ret[m] = 0;
    long iA = 0;
    for (long n = 0; n < N; n++) {
        double xval = x[n];
        for (long m = 0; m < M; m++) {
            ret[m] += A[iA] * xval;
            iA++;
        }
    }
}

void iterate_to_get_top_component(Mda& C, double& sigma, Mda& X, int num_iterations)
{
    long M = X.N1();
    long N = X.N2();
    C.allocate(M, 1);
    for (int i = 0; i < M; i++) {
        C.set(sin(i+1), i); //pseudo-random  - ahb fixed so the M=1 case isn't the zero vector
    }
    normalize_vector(C);
    for (int it = 0; it < num_iterations; it++) {
        // C <-- X*X'*C
        Mda tmp = mult_AtransB(C, X); //tmp is 1xN
        matvec(M, N, C.dataPtr(), X.dataPtr(), tmp.dataPtr()); //C is Mx1
        sigma = MLCompute::norm(M, C.dataPtr());
        normalize_vector(C);
    }
}

void iterate_to_get_top_component(Mda32& C, double& sigma, Mda32& X, int num_iterations)
{
    long M = X.N1();
    long N = X.N2();
    C.allocate(M, 1);
    for (int i = 0; i < M; i++) {
        C.set(sin(i+1), i); //pseudo-random  - ahb fixed so the M=1 case isn't the zero vector
    }
    normalize_vector(C);
    for (int it = 0; it < num_iterations; it++) {
        // C <-- X*X'*C
        Mda32 tmp = mult_AtransB(C, X); //tmp is 1xN
        matvec(M, N, C.dataPtr(), X.dataPtr(), tmp.dataPtr()); //C is Mx1
        sigma = MLCompute::norm(M, C.dataPtr());
        normalize_vector(C);
    }
}

void iterate_XXt_to_get_top_component(Mda& C, double& sigma, Mda& XXt, int num_iterations)
{
    long M = XXt.N1();
    C.allocate(M, 1);
    for (int i = 0; i < M; i++) {
        C.set(sin(i+1), i); //pseudo-random  - ahb fixed so the M=1 case isn't the zero vector
    }
    normalize_vector(C);
    for (int it = 0; it < num_iterations; it++) {
        // V = X*X'*V
        Mda tmp(M, 1);
        matvec(M, M, tmp.dataPtr(), XXt.dataPtr(), C.dataPtr()); //V is Mx1
        C = tmp;
        sigma = MLCompute::norm(M, C.dataPtr());
        normalize_vector(C);
    }
}

void iterate_XXt_to_get_top_component(Mda32& C, double& sigma, Mda32& XXt, int num_iterations)
{
    long M = XXt.N1();
    C.allocate(M, 1);
    for (int i = 0; i < M; i++) {
        C.set(sin(i+1), i); //pseudo-random  - ahb fixed so the M=1 case isn't the zero vector
    }
    normalize_vector(C);
    for (int it = 0; it < num_iterations; it++) {
        // V = X*X'*V
        Mda32 tmp(M, 1);
        matvec(M, M, tmp.dataPtr(), XXt.dataPtr(), C.dataPtr()); //V is Mx1
        C = tmp;
        sigma = MLCompute::norm(M, C.dataPtr());
        normalize_vector(C);
    }
}

double rand01()
{
    double ret = ((qrand() % 100000) + 0.5) * 1.0 / 100000;
    return ret;
}

void pca_unit_test()
{
    int M = 4;
    int N = 500;
    Mda X(M, N);
    int num_features = M;
    for (int n = 0; n < N; n++) {
        X.setValue(1 + 0.5 * sin(n) + 0.25 * cos(n) + rand01() * 0.5, 0, n);
        X.setValue(1 - 0.5 * sin(n) + 0.25 * cos(n) + rand01() * 0.5, 1, n);
        X.setValue(1 + 0.5 * sin(n) - 0.25 * cos(n) + rand01() * 0.5, 2, n);
        X.setValue(1 - 0.5 * sin(n) - 0.25 * cos(n) + rand01() * 0.5, 3, n);
    }
    Mda CC, FF;
    Mda sigma;
    pca(CC, FF, sigma, X, M);

    printf("\n");
    for (int k = 0; k < num_features; k++) {
        printf("Component (lambda=%g): %g %g %g %g\n", sqrt(sigma.value(k)), CC.value(0, k), CC.value(1, k), CC.value(2, k), CC.value(3, k));
    }

    printf("\n");
    Mda XXt = mult_ABtrans(X, X);
    Mda CC2;
    Mda sigma2;
    pca_from_XXt(CC2, sigma2, XXt, num_features);
    for (int k = 0; k < num_features; k++) {
        printf("Component (lambda=%g): %g %g %g %g\n", sqrt(sigma2.value(k)), CC2.value(0, k), CC2.value(1, k), CC2.value(2, k), CC2.value(3, k));
    }

    printf("\nTest:\n");
    Mda test = mult_ABtrans(CC, CC);
    for (int m = 0; m < M; m++) {
        printf("%g %g %g %g\n", test.value(m, 0), test.value(m, 1), test.value(m, 2), test.value(m, 3));
    }

    printf("\nWhitening:\n");
    Mda W;
    whitening_matrix_from_XXt(W, XXt);
    for (int m = 0; m < M; m++) {
        printf("%g %g %g %g\n", W.value(m, 0), W.value(m, 1), W.value(m, 2), W.value(m, 3));
    }

    Mda Xw = mult_AB(W, X);
    Mda XwXwt = mult_ABtrans(Xw, Xw);
    printf("\nXwXwt:\n");
    for (int m = 0; m < M; m++) {
        printf("%g %g %g %g\n", XwXwt.value(m, 0), XwXwt.value(m, 1), XwXwt.value(m, 2), XwXwt.value(m, 3));
    }
}

void whitening_matrix_from_XXt(Mda& W, Mda& XXt)
{
    int M = XXt.N1();
    Mda components, sigma;
    pca_from_XXt(components, sigma, XXt, M);  // sigma is list of eigenvalues of XXt (really sigma^2 for SVD of X)

    Mda D(M, M);       // build a diagonal matrix D = 1/sqrt(eigvals)
    for (int i = 0; i < M; i++) {
        double val = 0;
        if (sigma.get(i))
            val = 1 / sqrt(sigma.get(i));
        D.setValue(val, i, i);
    }

    Mda tmp = mult_AB(components, D);
    W = mult_ABtrans(tmp, components);     // output U.D.U^T is symmetric
}

void whitening_matrix_from_XXt(Mda32& W, Mda32& XXt)
{
    int M = XXt.N1();
    Mda32 components, sigma;
    pca_from_XXt(components, sigma, XXt, M);  // sigma is list of eigenvalues of XXt (really sigma^2 for SVD of X)

    Mda32 D(M, M);                    // build a diagonal matrix D = 1/sqrt(eigvals)
    for (int i = 0; i < M; i++) {
        double val = 0;
        if (sigma.get(i))
            val = 1 / sqrt(sigma.get(i));
        D.setValue(val, i, i);
    }

    Mda32 tmp = mult_AB(components, D);
    W = mult_ABtrans(tmp, components);      // output U.D.U^T is symmetric
}
