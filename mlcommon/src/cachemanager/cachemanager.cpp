/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/5/2016
*******************************************************/

#include "cachemanager.h"
#include "mlcommon.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QThread>

#define DEFAULT_LOCAL_BASE_PATH MLUtil::tempPath()

class CacheManagerPrivate {
public:
    CacheManager* q;
    QString m_local_base_path;

    QString create_random_file_name();
};

CacheManager::CacheManager()
{
    d = new CacheManagerPrivate;
    d->q = this;
}

CacheManager::~CacheManager()
{
    delete d;
}

void CacheManager::setLocalBasePath(const QString& path)
{
    d->m_local_base_path = path;
    if (!QDir(path).exists()) {
        QString parent = QFileInfo(path).path();
        QString name = QFileInfo(path).fileName();
        if (!QDir(parent).mkdir(name)) {
            qWarning() << "Unable to create local base path" << path;
        }
    }
    if (!QDir(path + "/tmp_short_term").exists()) {
        QDir(path).mkdir("tmp_short_term");
    }
    if (!QDir(path + "/tmp_long_term").exists()) {
        QDir(path).mkdir("tmp_long_term");
    }
}

QString CacheManager::makeRemoteFile(const QString& mlproxy_url, const QString& file_name_in, CacheManager::Duration duration)
{
    if (mlproxy_url.isEmpty()) {
        return makeLocalFile(file_name_in, duration);
    }

    QString file_name = file_name_in;
    if (file_name.isEmpty())
        file_name = d->create_random_file_name();
    QString str;
    if (duration == ShortTerm)
        str = "tmp_short_term";
    else if (duration == LongTerm)
        str = "tmp_long_term";
    else {
        qWarning() << "Unexpected problem" << __FUNCTION__ << __FILE__ << __LINE__;
        return "";
    }
    return QString("%1/mdaserver/%2/%3").arg(mlproxy_url).arg(str).arg(file_name);
}

QString CacheManager::makeLocalFile(const QString& file_name_in, CacheManager::Duration duration)
{
    QString file_name = file_name_in;
    if (file_name.isEmpty())
        file_name = d->create_random_file_name();
    if (d->m_local_base_path.isEmpty()) {
        qWarning() << "Local base path has not been set. Using default: " + QString(DEFAULT_LOCAL_BASE_PATH);
        this->setLocalBasePath(DEFAULT_LOCAL_BASE_PATH);
    }
    QString str;
    if (duration == ShortTerm)
        str = "tmp_short_term";
    else if (duration == LongTerm)
        str = "tmp_long_term";
    else {
        qWarning() << "Unexpected problem" << __FUNCTION__ << __FILE__ << __LINE__;
        return "";
    }
    QString ret = QString("%1/%2/%3").arg(d->m_local_base_path).arg(str).arg(file_name);

    return ret;
}

QString CacheManager::localTempPath()
{
    return d->m_local_base_path;
}

Q_GLOBAL_STATIC(CacheManager, theInstance)
CacheManager* CacheManager::globalInstance()
{
    return theInstance;
}

/*
void CacheManager::slot_remove_on_delete()
{
    QString fname=sender()->property("CacheManager_file_to_remove").toString();
    if (!fname.isEmpty()) {
        if (!QFile::remove(fname)) {
            qWarning() << "Unable to remove local cached file:" << fname;
        }
    }
    else {
        qWarning() << "Unexpected problem" << __FUNCTION__ << __FILE__ << __LINE__;
    }
}
*/

QString CacheManagerPrivate::create_random_file_name()
{

    long num1 = QDateTime::currentMSecsSinceEpoch();
    long num2 = (long)QThread::currentThreadId();
    long num3 = qrand();
    return QString("ms.%1.%2.%3.tmp").arg(num1).arg(num2).arg(num3);
}
