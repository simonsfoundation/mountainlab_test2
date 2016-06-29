#include "msmisc.h"
#include <math.h>
#include <QDateTime>
#include <QDir>
#include <QCryptographicHash>
#include <QThread>
#include "cachemanager.h"
#include "mlutils.h"

//I have to temporarily put this code in to get code completion to work in QtCreator
//even though DEFINES+=USE_NETWORK is in the .pro file.
//#define USE_NETWORK

#ifdef USE_NETWORK
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryFile>
#include "taskprogress.h"
#endif

#include <QCoreApplication>
#ifdef QT_GUI_LIB
#include <QMessageBox>
#endif

#include "textfile.h"

double compute_min(const QVector<double>& X)
{
    double ret = X.value(0);
    for (int i = 0; i < X.count(); i++)
        if (X[i] < ret)
            ret = X[i];
    return ret;
}

double compute_max(const QVector<double>& X)
{
    double ret = X.value(0);
    for (int i = 0; i < X.count(); i++)
        if (X[i] > ret)
            ret = X[i];
    return ret;
}

int compute_max(const QVector<int>& X)
{
    int ret = X.value(0);
    for (int i = 0; i < X.count(); i++)
        if (X[i] > ret)
            ret = X[i];
    return ret;
}

long compute_max(const QList<long>& X)
{
    long ret = X.value(0);
    for (int i = 0; i < X.count(); i++)
        if (X[i] > ret)
            ret = X[i];
    return ret;
}

double compute_norm(long N, double* X)
{
    double sumsqr = 0;
    for (long i = 0; i < N; i++)
        sumsqr += X[i] * X[i];
    return sqrt(sumsqr);
}

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

double compute_mean(long N, double* X)
{
    double sum = 0;
    for (int i = 0; i < N; i++)
        sum += X[i];
    if (N)
        sum /= N;
    return sum;
}

double compute_mean(const QVector<double>& X)
{
    double sum = 0;
    for (int i = 0; i < X.count(); i++)
        sum += X[i];
    if (X.count())
        sum /= X.count();
    return sum;
}

double compute_stdev(const QVector<double>& X)
{
    double sumsqr = 0;
    for (int i = 0; i < X.count(); i++)
        sumsqr += X[i] * X[i];
    double sum = 0;
    for (int i = 0; i < X.count(); i++)
        sum += X[i];
    int ct = X.count();
    if (ct >= 2) {
        return sqrt((sumsqr - sum * sum / ct) / (ct - 1));
    } else
        return 0;
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

double compute_max(long N, double* X)
{
    if (N == 0)
        return 0;
    double ret = X[0];
    for (long i = 0; i < N; i++) {
        if (X[i] > ret)
            ret = X[i];
    }
    return ret;
}

double compute_min(long N, double* X)
{
    if (N == 0)
        return 0;
    double ret = X[0];
    for (long i = 0; i < N; i++) {
        if (X[i] < ret)
            ret = X[i];
    }
    return ret;
}

QString compute_hash(const QString& str)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(str.toLatin1());
    return QString(hash.result().toHex());
}
