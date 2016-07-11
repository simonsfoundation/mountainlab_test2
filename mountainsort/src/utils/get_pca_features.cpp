#include "get_pca_features.h"
#include "mda.h"
#include "eigenvalue_decomposition.h"
#include "get_sort_indices.h"
#include "mlcommon.h"

#include <QTime>

/*
  Input:
  X -- MxN matrix
  components -- MxK, where K=num_features
  features -- KxN

  components are normalized so that the columns have unit norm
  features = components'*X
*/

Mda iterate_to_get_top_component(Mda& X, int num_iterations);
Mda AtransB(Mda& A, Mda& B);
void subtract_out_rank_1(Mda& X, Mda& A, Mda& B);
void normalize_vector(Mda& V);

void compute_principle_components(Mda& components, Mda& features, Mda& X, int num_features)
{
    long M = X.N1();
    long N = X.N2();
    long K = num_features;
    long num_iterations_per_component = 10; //hard-coded for now

    Mda Xw = X; //working data

    components.allocate(M, K);
    features.allocate(K, N);

    for (long k = 0; k < K; k++) {
        // C will be Mx1, F will be 1xN
        Mda C = iterate_to_get_top_component(Xw, num_iterations_per_component);
        Mda F = AtransB(C, X);
        for (long m = 0; m < M; m++) {
            components.set(C.get(m), m, k); //think about speeding this up
        }
        for (long n = 0; n < N; n++) {
            features.set(F.get(n), k, n); //think about speeding this up
        }
        subtract_out_rank_1(Xw, C, F);
    }
}

Mda AtransB(Mda& A, Mda& B)
{
    long M = A.N2();
    long L = A.N1();
    long N = B.N2();
    if (B.N1() != L) {
        qCritical() << "Unexpected dimensions in AtransB" << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    Mda C(M, N);
    double* Aptr = A.dataPtr();
    double* Bptr = B.dataPtr();
    double* Cptr = C.dataPtr();
    long iC = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            Cptr[iC] = MLCompute::dotProduct(L, &Aptr[L * m], &Bptr[L * n]);
            iC++;
        }
    }
    return C;
}

void subtract_out_rank_1(Mda& X, Mda& A, Mda& B)
{
    long M = X.N1();
    long N = X.N2();
    if ((A.N1() != M) || (A.N2() != 1) || (B.N1() != 1) || (B.N2() != N)) {
        qCritical() << "Incorrect dimensions in subtract_out_rank_1" << M << N << A.N1() << A.N2() << B.N1() << B.N2();
        abort();
    }
    double* Xptr = X.dataPtr();
    double* Aptr = A.dataPtr();
    double* Bptr = B.dataPtr();
    long iX = 0;
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            Xptr[iX] -= Aptr[m] * Bptr[n];
            iX++;
        }
    }
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

void matvec(long M, long N, double* ret, double* A, double* x)
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

Mda iterate_to_get_top_component(Mda& X, int num_iterations)
{
    long M = X.N1();
    long N = X.N2();
    Mda V(M, 1);
    for (int i = 0; i < M; i++) {
        V.set(sin(i), i); //pseudo-random
    }
    normalize_vector(V);
    for (int it = 0; it < num_iterations; it++) {
        // V = V'*X*X'*V
        Mda tmp = AtransB(V, X); //tmp is 1xN
        matvec(M, N, V.dataPtr(), X.dataPtr(), tmp.dataPtr()); //V is Mx1
        normalize_vector(V);
    }
    return V;
}

/*
bool get_pca_features(long M, long N, int num_features, double* features_out, double* X_in, long num_representatives)
{
    long increment = 1;
    if (num_representatives)
        increment = qMax(1L, N / num_representatives);
    QTime timer;
    timer.start();
    Mda XXt(M, M);
    double* XXt_ptr = XXt.dataPtr();
    for (long i = 0; i < N; i += increment) {
        double* tmp = &X_in[i * M];
        long aa = 0;
        for (long m2 = 0; m2 < M; m2++) {
            for (long m1 = 0; m1 < M; m1++) {
                XXt_ptr[aa] += tmp[m1] * tmp[m2];
                aa++;
            }
        }
    }

    Mda U;
    Mda S;
    eigenvalue_decomposition_sym(U, S, XXt);
    QVector<double> eigenvals;
    for (int i = 0; i < S.totalSize(); i++)
        eigenvals << S.get(i);
    QList<long> inds = get_sort_indices(eigenvals);

    Mda FF(num_features, N);
    long aa = 0;
    for (long i = 0; i < N; i++) {
        double* tmp = &X_in[i * M];
        for (int j = 0; j < num_features; j++) {
            if (inds.count() - 1 - j >= 0) {
                long k = inds.value(inds.count() - 1 - j);
                double val = 0;
                for (long m = 0; m < M; m++) {
                    val += U.get(m, k) * tmp[m];
                }
                FF.set(val, aa);
            }
            aa++;
        }
    }

    for (long i = 0; i < FF.totalSize(); i++) {
        features_out[i] = FF.get(i);
    }

    return true;
}

bool pca_denoise(long M, long N, int num_features, double* X_out, double* X_in, long num_representatives)
{
    long increment = 1;
    if (num_representatives)
        increment = qMax(1L, N / num_representatives);
    QTime timer;
    timer.start();
    Mda XXt(M, M);
    double* XXt_ptr = XXt.dataPtr();
    for (long i = 0; i < N; i += increment) {
        double* tmp = &X_in[i * M];
        long aa = 0;
        for (long m2 = 0; m2 < M; m2++) {
            for (long m1 = 0; m1 < M; m1++) {
                XXt_ptr[aa] += tmp[m1] * tmp[m2];
                aa++;
            }
        }
    }

    Mda U;
    Mda S;
    eigenvalue_decomposition_sym(U, S, XXt);
    QVector<double> eigenvals;
    for (int i = 0; i < S.totalSize(); i++)
        eigenvals << S.get(i);
    QList<long> inds = get_sort_indices(eigenvals);

    for (long i = 0; i < N; i++) {
        double* tmp_in = &X_in[i * M];
        double* tmp_out = &X_out[i * M];
        for (int m = 0; m < M; m++)
            tmp_out[m] = 0;
        for (int j = 0; j < num_features; j++) {
            if (inds.count() - 1 - j >= 0) {
                long k = inds.value(inds.count() - 1 - j);
                double val = 0;
                for (long m = 0; m < M; m++) {
                    val += U.get(m, k) * tmp_in[m];
                }
                for (long m = 0; m < M; m++) {
                    tmp_out[m] += U.get(m, k) * val;
                }
            }
        }
    }

    return true;
}
*/
