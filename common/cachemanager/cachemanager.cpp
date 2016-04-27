/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/5/2016
*******************************************************/

#include "cachemanager.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QThread>

#define DEFAULT_LOCAL_BASE_PATH QDir::tempPath()+"/mountainlab"

class MSCacheManagerPrivate {
public:
    MSCacheManager* q;
    QString m_local_base_path;

    QString create_random_file_name();
};

MSCacheManager::MSCacheManager()
{
    d = new MSCacheManagerPrivate;
    d->q = this;
}

MSCacheManager::~MSCacheManager()
{
    delete d;
}

void MSCacheManager::setLocalBasePath(const QString& path)
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

QString MSCacheManager::makeRemoteFile(const QString& remote_url, const QString& file_name_in, MSCacheManager::Duration duration)
{
    if (remote_url.isEmpty()) {
        return makeLocalFile(file_name_in,duration);
    }

    QString file_name=file_name_in;
    if (file_name.isEmpty()) file_name=d->create_random_file_name();
    QString str;
    if (duration == ShortTerm)
        str = "tmp_short_term";
    else if (duration == LongTerm)
        str = "tmp_long_term";
    else {
        qWarning() << "Unexpected problem" << __FUNCTION__ << __FILE__ << __LINE__;
        return "";
    }
    return QString("%1/%2/%3").arg(remote_url).arg(str).arg(file_name);
}

QString MSCacheManager::makeLocalFile(const QString& file_name_in, MSCacheManager::Duration duration)
{
    QString file_name=file_name_in;
    if (file_name.isEmpty()) file_name=d->create_random_file_name();
    if (d->m_local_base_path.isEmpty()) {
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
    QString ret=QString("%1/%2/%3").arg(d->m_local_base_path).arg(str).arg(file_name);
    return ret;
}

void MSCacheManager::cleanUp()
{
}

Q_GLOBAL_STATIC(MSCacheManager,theInstance)
MSCacheManager *MSCacheManager::globalInstance()
{
    return theInstance;
}

void MSCacheManager::slot_remove_on_delete()
{
    QString fname=sender()->property("MSCacheManager_file_to_remove").toString();
    if (!fname.isEmpty()) {
        if (!QFile::remove(fname)) {
            qWarning() << "Unable to remove local cached file:" << fname;
        }
    }
    else {
        qWarning() << "Unexpected problem" << __FUNCTION__ << __FILE__ << __LINE__;
    }
}

QString MSCacheManagerPrivate::create_random_file_name()
{

    long num1=QDateTime::currentMSecsSinceEpoch();
    long num2=(long)QThread::currentThreadId();
    long num3=qrand();
    return QString("ms.%1.%2.%3.tmp").arg(num1).arg(num2).arg(num3);
}
