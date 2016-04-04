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
    QString m_remote_name;
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

void MountainsortThread::setRemoteName(const QString &name)
{
    d->m_remote_name=name;
}

void MountainsortThread::compute()
{
    qDebug() << "----------------------------" << d->m_remote_name << d->m_processor_name;
    if (d->m_remote_name.isEmpty()) {
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
        QString url=mscmd_url_for_remote(d->m_remote_name)+"/?";
        qDebug() << "URL=" << url << d->m_remote_name;
        url+="processor="+d->m_processor_name+"&";
        QStringList keys=d->m_parameters.keys();
        foreach (QString key,keys) {
            url+=QString("%1=%2&").arg(key).arg(d->m_parameters.value(key).toString());
        }
        qDebug() << "URL=" << url;
        http_get_text(url);
    }
}


QString create_temporary_output_file_name(const QString &processor_name,const QMap<QString,QVariant> &params,const QString &parameter_name) {
    QString str=processor_name+":";
    QStringList keys=params.keys();
    qSort(keys);
    foreach (QString key,keys) {
        str+=key+"="+params["key"].toString()+"&";
    }
    return QString("/tmp/%1_%2.tmp").arg(compute_hash(str)).arg(parameter_name);
}
