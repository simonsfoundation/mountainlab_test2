/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/6/2016
*******************************************************/

#include "mlcommon.h"
#include "cachemanager/cachemanager.h"
#include "taskprogress/taskprogress.h"

#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QThread>
#include <QCoreApplication>
#include <QUrl>
#include <QDir>
#include <QCryptographicHash>
#include <math.h>
#include <QDataStream>

#ifdef QT_GUI_LIB
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QMessageBox>
#endif

QString TextFile::read(const QString& fname, QTextCodec* codec)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream ts(&file);
    if (codec != 0)
        ts.setCodec(codec);
    QString ret = ts.readAll();
    file.close();
    return ret;
}

bool TextFile::write(const QString& fname, const QString& txt, QTextCodec* codec)
{
    /*
     * Modification on 5/23/16 by jfm
     * We don't want an program to try to read this while we have only partially completed writing the file.
     * Therefore we now create a temporary file and then copy it over
     */

    QString tmp_fname = fname + ".tf." + MLUtil::makeRandomId(6) + ".tmp";

    //if a file with this name already exists, we need to remove it
    //(should we really do this before testing whether writing is successful? I think yes)
    if (QFile::exists(fname)) {
        if (!QFile::remove(fname)) {
            qWarning() << "Problem in TextFile::write" << __FUNCTION__ << __FILE__ << __LINE__;
            return false;
        }
    }

    //write text to temporary file
    QFile file(tmp_fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Problem in TextFile::write could not open for writing... " << __FUNCTION__ << __FILE__ << __LINE__ << tmp_fname;
        return false;
    }
    QTextStream ts(&file);
    if (codec != 0) {
        ts.setAutoDetectUnicode(false);
        ts.setCodec(codec);
    }
    ts << txt;
    ts.flush();
    file.close();

    //check the contents of the file (is this overkill?)
    QString txt_test = TextFile::read(tmp_fname, codec);
    if (txt_test != txt) {
        QFile::remove(tmp_fname);
        qWarning() << "Problem in TextFile::write" << __FUNCTION__ << __FILE__ << __LINE__;
        return false;
    }

    //finally, rename the file
    if (!QFile::rename(tmp_fname, fname)) {
        qWarning() << "Problem in TextFile::write" << __FUNCTION__ << __FILE__ << __LINE__;
        return false;
    }

    return true;
}

QChar make_random_alphanumeric_mv()
{
    static int val = 0;
    val++;
    QTime time = QTime::currentTime();
    int num = qHash(time.toString("hh:mm:ss:zzz") + QString::number(qrand() + val));
    if (num < 0)
        num = -num;
    num = num % 36;
    if (num < 26)
        return QChar('A' + num);
    else
        return QChar('0' + num - 26);
}

QString MLUtil::makeRandomId(int numchars)
{
    QString ret;
    for (int i = 0; i < numchars; i++) {
        ret.append(make_random_alphanumeric_mv());
    }
    return ret;
}

bool MLUtil::threadInterruptRequested()
{
    return QThread::currentThread()->isInterruptionRequested();
}

bool MLUtil::inGuiThread()
{
    return (QThread::currentThread() == QCoreApplication::instance()->thread());
}

QString find_ancestor_path_with_name(QString path, QString name)
{
    if (name.isEmpty())
        return "";
    while (QFileInfo(path).fileName() != name) {
        path = QFileInfo(path).path();
        if (!path.contains(name))
            return ""; //guarantees that we eventually exit the while loop
    }
    return path; //the directory name must equal the name argument
}

QString MLUtil::mountainlabBasePath()
{
    return find_ancestor_path_with_name(qApp->applicationDirPath(), "mountainlab");
}

void mkdir_if_doesnt_exist(const QString& path)
{
    if (!QDir(path).exists()) {
        QDir(QFileInfo(path).path()).mkdir(QFileInfo(path).fileName());
    }
}

QString MLUtil::mlLogPath()
{
    QString ret = mountainlabBasePath() + "/log";
    mkdir_if_doesnt_exist(ret);
    return ret;
}

QString MLUtil::resolvePath(const QString& basepath, const QString& path)
{
    if (QFileInfo(path).isRelative()) {
        return basepath + "/" + path;
    }
    else
        return path;
}

void MLUtil::mkdirIfNeeded(const QString& path)
{
    mkdir_if_doesnt_exist(path);
}

#include "sumit.h"
QString MLUtil::computeSha1SumOfFile(const QString& path)
{
    return sumit(path);
    /*
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return "";
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);
    file.close();
    QString ret = QString(hash.result().toHex());
    return ret;
    */
}

QString MLUtil::tempPath()
{
    //QString ret = mountainlabBasePath() + "/tmp";
    QString ret = QDir::tempPath() + "/mountainlab";
    mkdir_if_doesnt_exist(ret);
    return ret;
}

QVariant clp_string_to_variant(const QString& str);

CLParams::CLParams(int argc, char* argv[])
{
    this->success = true; //let's be optimistic!

    //find the named and unnamed parameters checking for errors along the way
    for (int i = 1; i < argc; i++) {
        QString str = QString(argv[i]);
        if (str.startsWith("--")) {
            int ind2 = str.indexOf("=");
            QString name = str.mid(2, ind2 - 2);
            QString val = "";
            if (ind2 >= 0)
                val = str.mid(ind2 + 1);
            if (name.isEmpty()) {
                this->success = false;
                this->error_message = "Problem with parameter: " + str;
                return;
            }
            this->named_parameters[name] = clp_string_to_variant(val);
        }
        else {
            this->unnamed_parameters << str;
        }
    }
}

bool clp_is_int(const QString& str)
{
    bool ok;
    str.toInt(&ok);
    return ok;
}

bool clp_is_float(const QString& str)
{
    bool ok;
    str.toFloat(&ok);
    return ok;
}

QVariant clp_string_to_variant(const QString& str)
{
    if (clp_is_int(str))
        return str.toInt();
    if (clp_is_float(str))
        return str.toFloat();
    return str;
}

double MLCompute::min(const QVector<double>& X)
{
    double ret = X.value(0);
    for (long i = 0; i < X.count(); i++) {
        if (X[i] < ret)
            ret = X[i];
    }
    return ret;
}

double MLCompute::max(const QVector<double>& X)
{
    double ret = X.value(0);
    for (long i = 0; i < X.count(); i++) {
        if (X[i] > ret)
            ret = X[i];
    }
    return ret;
}

double MLCompute::sum(const QVector<double>& X)
{
    double ret = 0;
    for (long i = 0; i < X.count(); i++) {
        ret += X[i];
    }
    return ret;
}

double MLCompute::mean(const QVector<double>& X)
{
    if (X.isEmpty())
        return 0;
    return sum(X) / X.count();
}

double MLCompute::stdev(const QVector<double>& X)
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
    }
    else
        return 0;
}

double MLCompute::dotProduct(const QVector<double>& X1, const QVector<double>& X2)
{
    if (X1.count() != X2.count())
        return 0;
    double ret = 0;
    for (long i = 0; i < X1.count(); i++)
        ret += X1[i] * X2[i];
    return ret;
}

double MLCompute::norm(const QVector<double>& X)
{
    return sqrt(dotProduct(X, X));
}

double MLCompute::correlation(const QVector<double>& X1, const QVector<double>& X2)
{
    if (X1.count() != X2.count())
        return 0;
    long N = X1.count();
    double mean1 = mean(X1);
    double stdev1 = stdev(X1);
    double mean2 = mean(X2);
    double stdev2 = stdev(X2);
    if ((stdev1 == 0) || (stdev2 == 0))
        return 0;
    QVector<double> Y1(N);
    QVector<double> Y2(N);
    for (long i = 0; i < N; i++) {
        Y1[i] = (X1[i] - mean1) / stdev1;
        Y2[i] = (X2[i] - mean2) / stdev2;
    }
    return dotProduct(Y1, Y2);
}

double MLCompute::norm(long N, double* X)
{
    return sqrt(dotProduct(N, X, X));
}

double MLCompute::dotProduct(long N, double* X1, double* X2)
{
    double ret = 0;
    for (long i = 0; i < N; i++)
        ret += X1[i] * X2[i];
    return ret;
}

QString MLUtil::computeSha1SumOfString(const QString& str)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(str.toLatin1());
    return QString(hash.result().toHex());
}

double MLCompute::sum(long N, double* X)
{
    double ret = 0;
    for (long i = 0; i < N; i++) {
        ret += X[i];
    }
    return ret;
}

double MLCompute::mean(long N, double* X)
{
    if (!N)
        return 0;
    return sum(N, X) / N;
}

double MLCompute::max(long N, double* X)
{
    if (!N)
        return 0;
    double ret = X[0];
    for (long i = 0; i < N; i++) {
        if (X[i] > ret)
            ret = X[i];
    }
    return ret;
}

double MLCompute::min(long N, double* X)
{
    if (!N)
        return 0;
    double ret = X[0];
    for (long i = 0; i < N; i++) {
        if (X[i] < ret)
            ret = X[i];
    }
    return ret;
}

QList<int> MLUtil::stringListToIntList(const QStringList& list)
{
    QList<int> ret;
    foreach (QString str, list) {
        ret << str.toInt();
    }
    return ret;
}

QStringList MLUtil::intListToStringList(const QList<int>& list)
{
    QStringList ret;
    foreach (int a, list) {
        ret << QString("%1").arg(a);
    }
    return ret;
}

void MLUtil::fromJsonValue(QByteArray& X, const QJsonValue& val)
{
    X = QByteArray::fromBase64(val.toString().toLatin1());
}

void MLUtil::fromJsonValue(QList<int>& X, const QJsonValue& val)
{
    X.clear();
    QByteArray ba;
    MLUtil::fromJsonValue(ba, val);
    QDataStream ds(&ba, QIODevice::ReadOnly);
    while (!ds.atEnd()) {
        int val;
        ds >> val;
        X << val;
    }
}

void MLUtil::fromJsonValue(QVector<int>& X, const QJsonValue& val)
{
    X.clear();
    QByteArray ba;
    MLUtil::fromJsonValue(ba, val);
    QDataStream ds(&ba, QIODevice::ReadOnly);
    while (!ds.atEnd()) {
        int val;
        ds >> val;
        X << val;
    }
}

void MLUtil::fromJsonValue(QVector<double>& X, const QJsonValue& val)
{
    X.clear();
    QByteArray ba;
    MLUtil::fromJsonValue(ba, val);
    QDataStream ds(&ba, QIODevice::ReadOnly);
    while (!ds.atEnd()) {
        double val;
        ds >> val;
        X << val;
    }
}

QJsonValue MLUtil::toJsonValue(const QByteArray& X)
{
    return QString(X.toBase64());
}

QJsonValue MLUtil::toJsonValue(const QList<int>& X)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < X.count(); i++) {
        ds << X[i];
    }
    return toJsonValue(ba);
}

QJsonValue MLUtil::toJsonValue(const QVector<int>& X)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < X.count(); i++) {
        ds << X[i];
    }
    return toJsonValue(ba);
}

QJsonValue MLUtil::toJsonValue(const QVector<double>& X)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < X.count(); i++) {
        ds << X[i];
    }
    return toJsonValue(ba);
}

QByteArray MLUtil::readByteArray(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    QByteArray ret = file.readAll();
    file.close();
    return ret;
}

bool MLUtil::writeByteArray(const QString& path, const QByteArray& X)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Unable to open file for writing byte array: " + path;
        return false;
    }
    if (file.write(X) != X.count()) {
        qWarning() << "Problem writing byte array: " + path;
        return false;
    }
    file.close();
    return true;
}
