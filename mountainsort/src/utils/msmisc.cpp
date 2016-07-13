#include "mlcommon.h"
#include <math.h>
#include <QDateTime>
#include <QDir>
#include <QCryptographicHash>
#include <QThread>
#include "mlcommon.h"
#include "mda.h"

#include <QCoreApplication>
#ifdef QT_GUI_LIB
#include <QMessageBox>
#endif

Mda compute_mean_clip(Mda& clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    Mda ret;
    ret.allocate(M, T);
    int aaa = 0;
    for (int i = 0; i < L; i++) {
        int bbb = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(bbb) + clips.get(aaa), bbb);
                aaa++;
                bbb++;
            }
        }
    }
    if (L) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(m, t) / L, m, t);
            }
        }
    }
    return ret;
}

Mda grab_clips_subset(Mda& clips, const QVector<int>& inds)
{
    int M = clips.N1();
    int T = clips.N2();
    int LLL = inds.count();
    Mda ret;
    ret.allocate(M, T, LLL);
    for (int i = 0; i < LLL; i++) {
        long aaa = i * M * T;
        long bbb = inds[i] * M * T;
        for (int k = 0; k < M * T; k++) {
            ret.set(clips.get(bbb), aaa);
            aaa++;
            bbb++;
        }
    }
    return ret;
}

/*
bool eigenvalue_decomposition_sym_isosplit(Mda& U, Mda& S, Mda& X)
{
    //X=U*diag(S)*U'
    //X is MxM, U is MxM, S is 1xM
    //X must be real and symmetric

    int M = X.N1();
    if (M != X.N2()) {
        qWarning() << "Unexpected problem in eigenvalue_decomposition_sym" << X.N1() << X.N2();
        exit(-1);
    }

    U.allocate(M, M);
    S.allocate(1, M);
    double* Uptr = U.dataPtr();
    //double* Sptr = S.dataPtr();
    double* Xptr = X.dataPtr();

    for (int ii = 0; ii < M * M; ii++) {
        Uptr[ii] = Xptr[ii];
    }

    //'V' means compute eigenvalues and eigenvectors (use 'N' for eigenvalues only)
    //'U' means upper triangle of A is stored.
    //QTime timer; timer.start();

    //int info = LAPACKE_dsyev(LAPACK_COL_MAJOR, 'V', 'U', M, Uptr, M, Sptr);
    int info = 0;

    //printf("Time for call to LAPACKE_dsyev: %g sec\n",timer.elapsed()*1.0/1000);
    if (info != 0) {
        qWarning() << "Error in LAPACKE_dsyev" << info;
        return false;
    }
    return true;
}
*/
