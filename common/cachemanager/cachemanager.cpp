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
#include "mlutils.h"

#define DEFAULT_LOCAL_BASE_PATH mlTmpPath()

class CacheManagerPrivate {
public:
    CacheManager* q;
    QString m_local_base_path;
    double m_max_short_term_gb;
    double m_max_long_term_gb;
    QTime m_last_cleanup_timer;

    QString create_random_file_name();
    void remove_old_files(const QString& path, double max_gb);
};

CacheManager::CacheManager()
{
    d = new CacheManagerPrivate;
    d->q = this;
    d->m_max_short_term_gb = 0;
    d->m_max_long_term_gb = 0;
    d->m_last_cleanup_timer.start();
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

    if (d->m_last_cleanup_timer.elapsed()>1000*60) {
        this->cleanUp();
        d->m_last_cleanup_timer.restart();
    }


    return ret;
}

QString CacheManager::localTempPath()
{
    return d->m_local_base_path;
}

void CacheManager::setMaxShortTermGB(double val)
{
    d->m_max_short_term_gb = val;
    this->cleanUp();
}

void CacheManager::setMaxLongTermGB(double val)
{
    d->m_max_long_term_gb = val;
    this->cleanUp();
}

void CacheManager::cleanUp()
{
    if (d->m_max_short_term_gb)
        d->remove_old_files(d->m_local_base_path + "/tmp_short_term", d->m_max_short_term_gb);
    if (d->m_max_long_term_gb)
        d->remove_old_files(d->m_local_base_path + "/tmp_long_term", d->m_max_long_term_gb);
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

struct FileRec {
    QString path;
    long elapsed_sec;
    double size_gb;
};

struct FileRec_comparer {
    bool operator()(const FileRec& a, const FileRec& b) const
    {
        if (a.elapsed_sec > b.elapsed_sec)
            return true;
        else
            return false;
    }
};

void sort_by_elapsed(QList<FileRec>& records)
{
    qSort(records.begin(), records.end(), FileRec_comparer());
}

void CacheManagerPrivate::remove_old_files(const QString& path, double max_gb)
{
    if (!max_gb)
        return;
    if (!QFileInfo(path).fileName().startsWith("tmp")) {
        qWarning() << "As a precaution not removing files from path that does not begin with tmp" << path;
        return;
    }
    QStringList fnames = QDir(path).entryList(QStringList("*"), QDir::Files, QDir::Name);
    QList<FileRec> records;
    foreach(QString fname, fnames)
    {
        FileRec rec;
        rec.path = path + "/" + fname;
        rec.elapsed_sec = QFileInfo(rec.path).lastModified().secsTo(QDateTime::currentDateTime());
        rec.size_gb = QFileInfo(rec.path).size() * 1.0 / 1e9;
        records << rec;
    }
    double total_size_gb = 0;
    for (int i = 0; i < records.count(); i++) {
        total_size_gb += records[i].size_gb;
    }
    long num_files_removed = 0;
    double amount_removed = 0;
    if (total_size_gb > max_gb) {
        double amount_to_remove = total_size_gb - 0.75 * max_gb; //let's get it down to 75% of the max allowed
        sort_by_elapsed(records);
        for (int i = 0; i < records.count(); i++) {
            if (amount_removed >= amount_to_remove) {
                break;
            }
            if (!QFile::remove(records[i].path)) {
                qWarning() << "Unable to remove file while cleaning up cache: " + records[i].path;
                return;
            }
            amount_removed += records[i].size_gb;
            num_files_removed++;
        }
    }
    if (num_files_removed) {
        qWarning() << QString("CacheManager removed %1 GB from %2 files").arg(amount_removed).arg(num_files_removed);
    }
}
