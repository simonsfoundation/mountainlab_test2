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
