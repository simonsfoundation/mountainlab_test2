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

class MountainsortThreadPrivate
{
public:
    MountainsortThread *q;
    QString m_processor_name;
    QMap<QString,QVariant> m_parameters;
    QString m_mscmd_server_url;
};

MountainsortThread::MountainsortThread() {
    d=new MountainsortThreadPrivate;
    d->q=this;
}

MountainsortThread::~MountainsortThread() {
    delete d;
}

void MountainsortThread::setProcessorName(const QString &pname)
{
    d->m_processor_name=pname;
}

void MountainsortThread::setParameters(const QMap<QString,QVariant> &parameters)
{
    d->m_parameters=parameters;
}

void MountainsortThread::setMscmdServerUrl(const QString &url)
{
    d->m_mscmd_server_url=url;
}

void MountainsortThread::compute()
{
    if (d->m_mscmd_server_url.isEmpty()) {
        QString mountainsort_exe=qApp->applicationDirPath()+"/../../mountainsort/bin/mountainsort";
        QStringList args;
        args << d->m_processor_name;
        QStringList keys=d->m_parameters.keys();
        foreach (QString key,keys) {
            args << QString("--%1=%2").arg(key).arg(d->m_parameters.value(key).toString());
        }
        qDebug()  << "MountainSort::::" << args;
        if (QProcess::execute(mountainsort_exe,args)!=0) {
            qWarning() << "Problem running mountainsort" << mountainsort_exe << args;
        }
    }
    else {
        QString url=d->m_mscmd_server_url+"/?";
        url+="processor="+d->m_processor_name+"&";
        QStringList keys=d->m_parameters.keys();
        foreach (QString key,keys) {
            url+=QString("%1=%2&").arg(key).arg(d->m_parameters.value(key).toString());
        }
        http_get_text(url);
    }
}

