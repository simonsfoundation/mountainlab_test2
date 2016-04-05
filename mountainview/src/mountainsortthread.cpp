/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "mountainsortthread.h"
#include <QCoreApplication>
#include <QMap>
#include <QProcess>
#include <QVariant>
#include <QDebug>
#include "msmisc.h"
#include "mscachemanager.h"

class MountainsortThreadPrivate {
public:
    MountainsortThread* q;
    QString m_processor_name;
    QMap<QString, QVariant> m_parameters;

    QString get_remote_name_from_parameters();
    QString create_temporary_output_file_name(const QString& remote_name, const QString& processor_name, const QMap<QString, QVariant>& params, const QString& parameter_name);
};

MountainsortThread::MountainsortThread()
{
    d = new MountainsortThreadPrivate;
    d->q = this;
}

MountainsortThread::~MountainsortThread()
{
    delete d;
}

void MountainsortThread::setProcessorName(const QString& pname)
{
    d->m_processor_name = pname;
}

QString MountainsortThread::makeOutputFilePath(const QString& pname)
{
    QString ret = d->create_temporary_output_file_name(d->get_remote_name_from_parameters(), d->m_processor_name, d->m_parameters, pname);
    d->m_parameters[pname] = ret;
    return ret;
}

void MountainsortThread::setInputParameters(const QMap<QString, QVariant>& parameters)
{
    d->m_parameters = parameters;
}

void MountainsortThread::compute()
{
    QString remote_name = d->get_remote_name_from_parameters();
    qDebug() << "MountainSort::::" << d->m_processor_name << d->m_parameters;
    if (remote_name.isEmpty()) {
        QString mountainsort_exe = qApp->applicationDirPath() + "/../../mountainsort/bin/mountainsort";
        QStringList args;
        args << d->m_processor_name;
        QStringList keys = d->m_parameters.keys();
        foreach (QString key, keys) {
            args << QString("--%1=%2").arg(key).arg(d->m_parameters.value(key).toString());
        }
        if (QProcess::execute(mountainsort_exe, args) != 0) {
            qWarning() << "Problem running mountainsort" << mountainsort_exe << args;
        }
    }
    else {
        QString url = mscmd_url_for_remote(remote_name) + "/?";
        url += "processor=" + d->m_processor_name + "&";
        QStringList keys = d->m_parameters.keys();
        foreach (QString key, keys) {
            url += QString("%1=%2&").arg(key).arg(d->m_parameters.value(key).toString());
        }
        http_get_text(url);
    }
}

QString MountainsortThreadPrivate::create_temporary_output_file_name(const QString& remote_name, const QString& processor_name, const QMap<QString, QVariant>& params, const QString& parameter_name)
{
    QString str = processor_name + ":";
    QStringList keys = params.keys();
    qSort(keys);
    foreach (QString key, keys) {
        str += key + "=" + params.value(key).toString() + "&";
    }

    QString file_name=QString("%1_%2.tmp").arg(compute_hash(str)).arg(parameter_name);
    QString ret=cacheManager()->makeRemoteFile(remote_name,file_name,MSCacheManager::LongTerm);
    //QString ret = QString("tmp_long_term/%1_%2.tmp").arg(compute_hash(str)).arg(parameter_name);
    /*
    if (!remote_name.isEmpty()) {
        ret = "remote://" + remote_name + "/" + ret;
    }
    else {
        ret = "/" + ret;
    }
    */
    return ret;
}

QString MountainsortThreadPrivate::get_remote_name_from_parameters()
{
    QString ret;
    QStringList keys = m_parameters.keys();
    foreach (QString key, keys) {
        QString val = m_parameters.value(key).toString();
        if (val.startsWith("remote://")) {
            QString remote_name = remote_name_of_path(val);
            if (!remote_name.isEmpty()) {
                if (ret.isEmpty()) {
                    ret = remote_name;
                }
            }
            else {
                qWarning() << QString("More than one remote name in parameters for %1. Using %2.").arg(m_processor_name).arg(ret);
                qWarning() << m_parameters;
            }
        }
    }
    return ret;
}
