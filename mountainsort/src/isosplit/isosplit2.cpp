#include "isosplit2.h"
#include <QSet>
#include <math.h>
#include "isocut.h"
#include <stdio.h>
#include <QDebug>
#include "mlcommon.h"
#include "mliterator.h"
#include "pca.h" //for whitening

QVector<int> do_kmeans(Mda32& X, int K);
bool eigenvalue_decomposition_sym_isosplit(Mda32& U, Mda32& S, Mda32& X);

struct AttemptedComparisons {
    QVector<double> centers1, centers2;
    QVector<int> counts1, counts2;
};

/*!
 * \brief returns list of indices in the vector equal to \a k
 *
 */
QList<long> find_inds(const QVector<int>& labels, int k)
{
    QList<long> ret;
    for (int i = 0; i < labels.count(); i++) {
        if (labels[i] == k)
            ret << i;
    }
    return ret;
}

void geometric_median(int M, int N, double* ret, const double* X)
{
    int num_iterations = 10;
    if (N == 0)
        return;
    if (N == 1) {
        std::copy(X, X+M, ret);
        return;
    }
    std::vector<double> weights(N, 1);
    for (int it = 1; it <= num_iterations; it++) {
        double sum_weights = std::accumulate(weights.begin(), weights.end(), 0.0);
        if (sum_weights) {
            for (int i = 0; i < N; i++)
                weights[i] /= sum_weights;
        }
        for (int m = 0; m < M; m++)
            ret[m] = 0;
        int aa = 0;
        for (int n = 0; n < N; n++) {
            for (int m = 0; m < M; m++) {
                ret[m] += X[aa] * weights[n];
                aa++;
            }
        }
        aa = 0;
        for (int n = 0; n < N; n++) {
            double sumsqr_diff = 0;
            for (int m = 0; m < M; m++) {
                double val = X[aa] - ret[m];
                sumsqr_diff = val * val;
                aa++;
            }
            if (sumsqr_diff != 0) {
                weights[n] = 1 / sqrt(sumsqr_diff);
            }
            else
                weights[n] = 0;
        }
    }
}

QVector<double> compute_centroid(Mda32& X)
{
    int M = X.N1();
    int N = X.N2();
    QVector<double> ret(M, 0);
    for (int n = 0; n < N; n++) {
        for (int m = 0; m < M; m++) {
            ret[m] += X.value(m, n);
        }
    }
    for (int i = 0; i < M; i++)
        ret[i] /= N;
    return ret;
}

QVector<double> compute_center(Mda32& X, const QList<long>& inds)
{
    int M = X.N1();
    int NN = inds.count();
    if (NN == 0) {
        return QVector<double>(M, 0);
    }
    QVector<double> XX(M*NN);
    int aa = 0;
    for (int n = 0; n < NN; n++) {
        for (int m = 0; m < M; m++) {
            XX[aa] = X.value(m, inds[n]);
            aa++;
        }
    }
    if (NN == 1) {
        // for NN = 1 geometric_median() returns the original data
        // but it copies it to the target vector.
        // We can avoid it by returning the original vector
        XX.resize(M); // it's already M but let's explicitly make sure of that
        return XX;
    }
    QVector<double> result(M);
    geometric_median(M, NN, result.data(), XX.constData());
    return result;
}

Mda32 compute_centers(Mda32& X, const QVector<int>& labels, int K)
{
    int M = X.N1();
    //int N=X.N2();
    Mda32 ret(M, K);
    for (int k = 0; k < K; k++) {
        QList<long> inds = find_inds(labels, k);
        QVector<double> ctr = compute_center(X, inds);
        for (int m = 0; m < M; m++)
            ret.set(ctr[m], m, k);
    }
    return ret;
}

double distance_between_vectors(int M, float* v1, float* v2)
{
    double sumsqr = 0;
    for (int i = 0; i < M; i++) {
        double val = v1[i] - v2[i];
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

double distance_between_vectors(int M, const double* v1, const double* v2)
{
    double sumsqr = 0;
    for (int i = 0; i < M; i++) {
        double val = v1[i] - v2[i];
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

double distance_between_vectors(int M, const double* v1, const float* v2)
{
    double sumsqr = 0;
    for (int i = 0; i < M; i++) {
        double val = v1[i] - v2[i];
        sumsqr += val * val;
    }
    return sqrt(sumsqr);
}

bool was_already_attempted(int M, const AttemptedComparisons& attempted_comparisons, float* center1, float* center2, int count1, int count2, double repeat_tolerance)
{
    double tol = repeat_tolerance;
    for (int i = 0; i < attempted_comparisons.counts1.count(); i++) {
        double diff_count1 = fabs(attempted_comparisons.counts1[i] - count1);
        double avg_count1 = (attempted_comparisons.counts1[i] + count1) / 2;
        if (diff_count1 <= tol * avg_count1) {
            double diff_count2 = fabs(attempted_comparisons.counts2[i] - count2);
            double avg_count2 = (attempted_comparisons.counts2[i] + count2) / 2;
            if (diff_count2 <= tol * avg_count2) {
#if 0
                float C1[M];
                for (int m = 0; m < M; m++)
                    C1[m] = attempted_comparisons.centers1[i * M + m];
                float C2[M];
                for (int m = 0; m < M; m++)
                    C2[m] = attempted_comparisons.centers2[i * M + m];
#else
                // We don't have to copy into C1 and C2
                // attempted_comparisons.centers{1,2} already contain all the data we need
                // we just need to point to the proper section of the data
                // the only problem is these are doubles and not floats
                // but this shouldn't affect speed that much
                const double *C1 = attempted_comparisons.centers1.constData()+(i*M);
                const double *C2 = attempted_comparisons.centers2.constData()+(i*M);
#endif
                const double dist0 = distance_between_vectors(M, C1, C2);
                if (dist0 > 0) {
                    double dist1 = distance_between_vectors(M, C1, center1);
                    double dist2 = distance_between_vectors(M, C2, center2);
                    double frac1 = dist1 / dist0;
                    double frac2 = dist2 / dist0;
                    if ((frac1 <= tol * 1 / sqrt(count1)) && (frac2 <= tol * 1 / sqrt(count2))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void find_next_comparison(int M, int K, int& k1, int& k2, const bool* active_labels, float* Cptr, int* counts, const AttemptedComparisons& attempted_comparisons, double repeat_tolerance)
{
    QVector<int> active_inds;
    active_inds.reserve(1+K/2); // just in case => going to realloc at most once
    for (int k = 0; k < K; k++)
        if (active_labels[k])
            active_inds << k;
    if (active_inds.isEmpty()) {
        k1 = -1;
        k2 = -1;
        return;
    }
    int Nactive = active_inds.count();
    double dists[Nactive][Nactive];
    for (int a = 0; a < Nactive; a++) {
        for (int b = 0; b < Nactive; b++) {
            dists[a][b] = distance_between_vectors(M, &Cptr[active_inds[a] * M], &Cptr[active_inds[b] * M]);
        }
        dists[a][a] = -1; //don't use it
    }
    while (true) {
        int best_a = -1, best_b = -1;
        double best_dist = -1;
        for (int a = 0; a < Nactive; a++) {
            for (int b = 0; b < Nactive; b++) {
                double val = dists[a][b];
                if (val >= 0) {
                    if ((best_dist < 0) || (val < best_dist)) {
                        best_a = a;
                        best_b = b;
                        best_dist = val;
                    }
                }
            }
        }
        if (best_a < 0) {
            k1 = -1;
            k2 = -1;
            return;
        }
        k1 = active_inds[best_a];
        k2 = active_inds[best_b];
        if ((counts[k1] > 0) && (counts[k2] > 0)) { //just to make sure (zero was actually happening some times, but not sure why)
            if (!was_already_attempted(M, attempted_comparisons, &Cptr[k1 * M], &Cptr[k2 * M], counts[k1], counts[k2], repeat_tolerance)) {
                //hurray!
                return;
            }
        }
        dists[best_a][best_b] = -1;
        dists[best_b][best_a] = -1;
    }
    k1 = -1;
    k2 = -1;
}

#if 0
Mda32 matrix_transpose_isosplit(const Mda32& A)
{
    Mda32 ret(A.N2(), A.N1());
    for (int i = 0; i < A.N1(); i++) {
        for (int j = 0; j < A.N2(); j++) {
            ret.set(A.get(i, j), j, i);
        }
    }
    return ret;
}
#endif

Mda32 matrix_multiply_isosplit(const Mda32& A, const Mda32& B)
{
    int N1 = A.N1();
    int N2 = A.N2();
    int N2B = B.N1();
    int N3 = B.N2();
    if (N2 != N2B) {
        qWarning() << "Unexpected problem in matrix_multiply" << N1 << N2 << N2B << N3;
        exit(-1);
    }
    Mda32 ret(N1, N3);
    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < N3; j++) {
            double val = 0;
            for (int k = 0; k < N2; k++) {
                val += A.get(i, k) * B.get(k, j);
            }
            ret.set(val, i, j);
        }
    }
    return ret;
}

#if 0
Mda32 matrix_multiply_transposed_isosplit(const Mda32 &A)
{
    int N1 = A.N1();
    int N2 = A.N2();

    Mda32 ret(N1, N1);
    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < N1; j++) {
            double val = 0;
            for (int k = 0; k < N2; k++) {
                val += A.get(i, k) * A.get(j, k);
            }
            ret.set(val, i, j);
        }
    }
    return ret;
}
#else
Mda32 matrix_multiply_transposed_isosplit(const Mda32 &A)
{
    int N1 = A.N1();
    int N2 = A.N2();

    Mda32 ret(N1, N1);
    for (int j = 0; j < N1; j++) {
        for (int i = 0; i < j+1; i++) {
            double val = 0;
            for (int k = 0; k < N2; k++) {
                val += A.get(i, k) * A.get(j, k);
            }
            ret.set(val, i, j);
            ret.set(val, j, i); // resulting matrix is symmetric
        }
    }
    return ret;
}
#endif
/*
Mda32 get_whitening_matrix_isosplit(Mda32& COV)
{
    int M = COV.N1();
    Mda32 U(M, M), S(1, M);
    eigenvalue_decomposition_sym_isosplit(U, S, COV);
    Mda32 S2(M, M);
    for (int m = 0; m < M; m++) {
        if (S.get(m)) {
            S2.set(1 / sqrt(S.get(m)), m, m);
        }
    }
    Mda32 W = matrix_multiply_isosplit(matrix_multiply_isosplit(U, S2), matrix_transpose_isosplit(U));
    return W;
}
*/

void whiten_two_clusters(double* V, Mda32& X1, Mda32& X2)
{
    int M = X1.N1();
    int N1 = X1.N2();
    int N2 = X2.N2();
    int N = N1 + N2;

    QVector<double> center1 = compute_centroid(X1);
    QVector<double> center2 = compute_centroid(X2);

    if (M < N) { //otherwise there are too few points to whiten

        Mda32 XX(M, N);
        for (int i = 0; i < N1; i++) {
            for (int j = 0; j < M; j++) {
                XX.set(X1.get(j, i) - center1[j], j, i);
            }
        }
        for (int i = 0; i < N2; i++) {
            for (int j = 0; j < M; j++) {
                XX.set(X2.get(j, i) - center2[j], j, i + N1);
            }
        }
        /// TODO: This can definitely be optimized as it should be perfectly possible
        ///       to multiply a matrix with a transposed matrix without precalculating
        ///       the transposed matrix.
        ///
        ///       (AA^T)^T = AA^T (the matrix is symmetric) therefore we can calculate
        ///                       just half of the resulting matrix
#if 0
        Mda32 XXt = matrix_multiply_isosplit(XX, matrix_transpose_isosplit(XX));
#else
        Mda32 XXt = matrix_multiply_transposed_isosplit(XX);
#endif
        //Mda32 W = get_whitening_matrix_isosplit(COV);
        Mda32 W;
        whitening_matrix_from_XXt(W, XXt);
        X1 = matrix_multiply_isosplit(W, X1);
        X2 = matrix_multiply_isosplit(W, X2);
    }

    //compute the vector
    center1 = compute_centroid(X1);
    center2 = compute_centroid(X2);
    for (int m = 0; m < M; m++) {
        V[m] = center2[m] - center1[m];
    }
}

QVector<int> test_redistribute(bool& do_merge, Mda32& Y1, Mda32& Y2, double isocut_threshold)
{
    Mda32 X1;
    X1 = Y1;
    Mda32 X2;
    X2 = Y2;
    int M = X1.N1();
    int N1 = X1.N2();
    int N2 = X2.N2();
    QVector<int> ret(N1+N2, 1);
    do_merge = true;
    double V[M];
    whiten_two_clusters(V, X1, X2);
    double normv = 0;
    {
        double sumsqr = 0;
        for (int m = 0; m < M; m++)
            sumsqr += V[m] * V[m];
        normv = sqrt(sumsqr);
    }
    if (!normv) {
        printf("Warning: isosplit2: vector V is null.\n");
        return ret;
    }
    if (N1 + N2 <= 5) {
        //avoid a crash?
        return ret;
    }

    //project onto line connecting the centers
    QVector<double> XX;
    XX.reserve(N1+N2);
    for (int i = 0; i < N1; i++) {
        double val = 0;
        for (int m = 0; m < M; m++)
            val += X1.value(m, i) * V[m];
        XX << val;
    }
    for (int i = 0; i < N2; i++) {
        double val = 0;
        for (int m = 0; m < M; m++)
            val += X2.value(m, i) * V[m];
        XX << val;
    }
#if 0
    QVector<double> XXs = XX;
    qSort(XXs);

    const double* XXX = XXs.constData();


    double cutpoint;
    bool do_cut = isocut(N1 + N2, &cutpoint, XXX, isocut_threshold, 5);
#else
    double cutpoint;
    bool do_cut = isocut(N1 + N2, &cutpoint, XX.constData(), isocut_threshold, 5);
#endif

    if (do_cut) {
        do_merge = false;
        for (int i = 0; i < N1 + N2; i++) {
            if (XX[i] <= cutpoint)
                ret[i] = 1;
            else
                ret[i] = 2;
        }
    }
    return ret;
}

QVector<int> test_redistribute(bool& do_merge, Mda32& X, const QList<long>& inds1, const QList<long>& inds2, double isocut_threshold)
{
    int M = X.N1();
    Mda32 X1(M, inds1.count());
    for (int i = 0; i < inds1.count(); i++) {
        for (int m = 0; m < M; m++) {
            X1.setValue(X.value(m, inds1[i]), m, i);
        }
    }
    Mda32 X2(M, inds2.count());
    for (int i = 0; i < inds2.count(); i++) {
        for (int m = 0; m < M; m++) {
            X2.setValue(X.value(m, inds2[i]), m, i);
        }
    }
    return test_redistribute(do_merge, X1, X2, isocut_threshold);
}

QVector<int> isosplit2(Mda32& X, float isocut_threshold, int K_init, bool verbose)
{
    double repeat_tolerance = 0.2;

    int M = X.N1();
    int N = X.N2();
    QVector<int> labels = do_kmeans(X, K_init);

    QVector<bool> active_labels(K_init, true);
    Mda32 centers = compute_centers(X, labels, K_init); //M x K_init
    int counts[K_init];
    for (int ii = 0; ii < K_init; ii++)
        counts[ii] = 0;
    for (int i = 0; i < N; i++)
        counts[labels[i]]++;
    dtype32* Cptr = centers.dataPtr();

    AttemptedComparisons attempted_comparisons;

    int num_iterations = 0;
    int max_iterations = 1000;
    while ((true) && (num_iterations < max_iterations)) {
        num_iterations++;
        if (verbose)
            printf("isosplit2: iteration %d\n", num_iterations);
        int k1, k2;
        find_next_comparison(M, K_init, k1, k2, active_labels.constData(), Cptr, counts, attempted_comparisons, repeat_tolerance);
        if (k1 < 0)
            break;
        if (verbose)
            printf("compare %d(%d),%d(%d) --- ", k1, counts[k1], k2, counts[k2]);

        const QList<long> inds1 = find_inds(labels, k1);
        const QList<long> inds2 = find_inds(labels, k2);
        QList<long> inds12 = inds1 + inds2;
        for (int m = 0; m < M; m++) {
            attempted_comparisons.centers1 << Cptr[m + k1 * M];
            attempted_comparisons.centers2 << Cptr[m + k2 * M];
        }
        attempted_comparisons.counts1 << inds1.count();
        attempted_comparisons.counts2 << inds2.count();
        for (int m = 0; m < M; m++) {
            attempted_comparisons.centers2 << Cptr[m + k1 * M];
            attempted_comparisons.centers1 << Cptr[m + k2 * M];
        }
        attempted_comparisons.counts2 << inds1.count();
        attempted_comparisons.counts1 << inds2.count();

        bool do_merge;

        QVector<int> labels0 = test_redistribute(do_merge, X, inds1, inds2, isocut_threshold);
        int max_label = *std::max_element(labels0.constBegin(), labels0.constEnd());
        if ((do_merge) || (max_label == 1)) {
            if (verbose)
                printf("merging size=%d.\n", inds12.count());
            for (int i = 0; i < N; i++) {
                if (labels[i] == k2)
                    labels[i] = k1;
            }
            QVector<double> ctr = compute_center(X, inds12);
            for (int m = 0; m < M; m++) {
                centers.setValue(ctr[m], m, k1);
            }
            counts[k1] = inds12.count();
            counts[k2] = 0;
            active_labels[k2] = false;
        }
        else {

            QList<long> indsA0 = find_inds(labels0, 1);
            QList<long> indsB0 = find_inds(labels0, 2);
            QList<long> indsA, indsB;
            indsA.reserve(indsA0.count());
            indsB.reserve(indsB0.count());
            for (int i = 0; i < indsA0.count(); i++)
                indsA << inds12[indsA0[i]];
            for (int i = 0; i < indsB0.count(); i++)
                indsB << inds12[indsB0[i]];
            for (int i = 0; i < indsA.count(); i++) {
                labels[indsA[i]] = k1;
            }
            for (int i = 0; i < indsB.count(); i++) {
                labels[indsB[i]] = k2;
            }
            if (verbose)
                printf("redistributing sizes=(%d,%d).\n", indsA.count(), indsB.count());
            QVector<double> ctr = compute_center(X, indsA);
            for (int m = 0; m < M; m++) {
                centers.setValue(ctr[m], m, k1);
            }
            ctr = compute_center(X, indsB);
            for (int m = 0; m < M; m++) {
                centers.setValue(ctr[m], m, k2);
            }
            counts[k1] = indsA.count();
            counts[k2] = indsB.count();
        }
    }

    int labels_map[K_init];
    for (int k = 0; k < K_init; k++)
        labels_map[k] = 0;
    int kk = 1;
    for (int j = 0; j < K_init; j++) {
        if ((active_labels[j]) && (counts[j] > 0)) {
            labels_map[j] = kk;
            kk++;
        }
    }
    QVector<int> ret;
    ret.reserve(N);
    for (int i = 0; i < N; i++) {
        ret << labels_map[labels[i]];
    }
    return ret;
}

//choose K distinct (sorted) integers between 0 and N-1. If K>N then it will repeat the last integer a suitable number of times
QVector<int> choose_random_indices(int N, int K)
{
    QVector<int> ret;
    ret.reserve(K);
    if (K >= N) {
#if 1
        for (int i = 0; i < N; i++)
            ret << i;
        while (ret.count() < K)
            ret << N - 1;
        return ret;
#else
        QVector<int> ret;
        std::copy(ML::counting_iterator<int>(0),
                  ML::counting_iterator<int>(N),
                  std::back_inserter(ret));
        while(ret.size() < K) ret << N-1;
        return ret;
#endif
    }
#if 0
    /// TODO: This can actually starve the function
    ///       better make it deterministic
    QSet<int> theset;
    while (theset.count() < K) {
        int ind = (qrand() % N);
        theset.insert(ind);
    }
    ret = theset.toList().toVector();
    qSort(ret);
    return ret;
#else
    // fill vector with numbers [0, N-1]
    // shuffle vector getting a random permutation
    // return first K elements
    ret.reserve(N);
    std::copy(ML::counting_iterator<int>(0),
              ML::counting_iterator<int>(N),
              std::back_inserter(ret));
    std::random_shuffle(ret.begin(), ret.end());
    ret.resize(K); // truncate to K
    qSort(ret);
    return ret;
#endif
}

//do k-means with K clusters -- X is MxN representing N points in M-dimensional space. Returns a labels vector of size N.
QVector<int> do_kmeans(Mda32& X, int K)
{
    int M = X.N1();
    int N = X.N2();
    if (N == 0)
        return QVector<int>(); //added 4/8/16 to prevent crash
    dtype32* Xptr = X.dataPtr();
    Mda32 centroids_mda;
    centroids_mda.allocate(M, K);
    dtype32* centroids = centroids_mda.dataPtr();
    QVector<int> labels(N, -1);
    QVector<int> counts(K);

    //initialize the centroids
    QVector<int> initial = choose_random_indices(N, K);
    for (int j = 0; j < K; j++) {
        int ind = initial[j];
        int jj = ind * M;
        int ii = j * M;
        for (int m = 0; m < M; m++) {
            centroids[m + ii] = Xptr[m + jj];
        }
    }

    bool something_changed = true;
    while (something_changed) {
        something_changed = false;
        //Assign the labels
        for (int n = 0; n < N; n++) {
            int jj = n * M;
            double best_distsqr = 0;
            int best_k = 0;
            for (int k = 0; k < K; k++) {
                int ii = k * M;
                double tmp = 0;
                for (int m = 0; m < M; m++) {
                    tmp += (centroids[m + ii] - Xptr[m + jj]) * (centroids[m + ii] - Xptr[m + jj]);
                }
                if ((k == 0) || (tmp < best_distsqr)) {
                    best_distsqr = tmp;
                    best_k = k;
                }
            }
            if (labels[n] != best_k) {
                labels[n] = best_k;
                something_changed = true;
            }
        }

        if (something_changed) {
            //Compute the centroids
            for (int k = 0; k < K; k++) {
                int ii = k * M;
                for (int m = 0; m < M; m++) {
                    centroids[m + ii] = 0;
                }
                counts[k] = 0;
            }
            for (int n = 0; n < N; n++) {
                int jj = n * M;
                int k = labels[n];
                int ii = k * M;
                for (int m = 0; m < M; m++) {
                    centroids[m + ii] += Xptr[m + jj];
                }
                counts[k]++;
            }
            for (int k = 0; k < K; k++) {
                int ii = k * M;
                if (counts[k]) {
                    for (int m = 0; m < M; m++)
                        centroids[m + ii] /= counts[k];
                }
            }
        }
    }

    return labels;
}

/*
void test_isosplit2_routines()
{
    { //whiten two clusters
        //compare this with the test in matlab isosplit2('test')
        Mda32 X1, X2;
        int M = 4;
        X1.allocate(M, M);
        X2.allocate(M, M);
        for (int m1 = 0; m1 < M; m1++) {
            for (int m2 = 0; m2 < M; m2++) {
                X1.setValue(sin(m1 + m2) + sin(m1 * m2), m1, m2);
                X2.setValue(cos(m1 + m2) - cos(m1 * m2), m1, m2);
            }
        }
        printf("X1:\n");
        for (int m2 = 0; m2 < M; m2++) {
            for (int m1 = 0; m1 < M; m1++) {
                printf("%g ", X1.value(m2, m1));
            }
            printf("\n");
        }
        double V[M];
        whiten_two_clusters(V, X1, X2);
        printf("V: ");
        for (int m = 0; m < M; m++) {
            printf("%g ", V[m]);
        }
        printf("\n");
    }

    {
        int M = 2;
        int N = 120;
        Mda32 X;
        X.allocate(M, N);
        for (int i = 0; i < N; i++) {
            double r1 = (qrand() % 100000) * 1.0 / 100000;
            double r2 = (qrand() % 100000) * 1.0 / 100000;
            if (i < N / 3) {
                double val1 = r1;
                double val2 = r2;
                X.setValue(val1, 0, i);
                X.setValue(val2, 1, i);
            }
            else if (i < 2 * N / 3) {
                double val1 = 1.5 + r1;
                double val2 = r2;
                X.setValue(val1, 0, i);
                X.setValue(val2, 1, i);
            }
            else {
                double val1 = r1;
                double val2 = 1.5 + r2;
                X.setValue(val1, 0, i);
                X.setValue(val2, 1, i);
            }
        }

        QVector<int> labels = isosplit2(X, 1.5, 30, false);
        printf("Labels:\n");
        for (int i = 0; i < labels.count(); i++) {
            printf("%d ", labels[i]);
        }
        printf("\n");
    }
}
*/
