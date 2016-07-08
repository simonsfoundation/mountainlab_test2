#include "tempfilecleaner.h"
#include <QFileInfo>
#include <QTimer>
#include <qdir.h>
#include <QDebug>
#include <QDateTime>

struct TFCRec {
    TFCRec(QString path0, double max_gb0)
    {
        path = path0;
        max_gb = max_gb0;
    }

    QString path;
    double max_gb;
};

class TempFileCleanerPrivate {
public:
    TempFileCleaner* q;
    QList<TFCRec> m_records;

    void clean_path(QString path, double max_gb);
};

TempFileCleaner::TempFileCleaner()
{
    d = new TempFileCleanerPrivate;
    d->q = this;
    QTimer::singleShot(5000, this, SLOT(slot_timer()));
}

TempFileCleaner::~TempFileCleaner()
{
    delete d;
}

void TempFileCleaner::addPath(QString path, double max_gb)
{
    d->m_records << TFCRec(path, max_gb);
}

void TempFileCleaner::slot_timer()
{
    for (int i = 0; i < d->m_records.count(); i++) {
        d->clean_path(d->m_records[i].path, d->m_records[i].max_gb);
    }
    QTimer::singleShot(30 * 1000, this, SLOT(slot_timer()));
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

void TempFileCleanerPrivate::clean_path(QString path, double max_gb)
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
        qWarning() << QString(":::::::::::::::::::::::::::::::: CacheManager removed %1 GB and %2 files").arg(amount_removed).arg(num_files_removed);
    }
}
