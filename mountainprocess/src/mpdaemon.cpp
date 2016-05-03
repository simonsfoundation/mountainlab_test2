/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/2/2016
*******************************************************/

#include "mpdaemon.h"
#include <QEventLoop>
#include <QCoreApplication>
#include <QTimer>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "textfile.h"

class MPDaemonPrivate {
public:
    MPDaemon *q;
    bool m_stop_requested;
    bool m_is_running;

    void write_info();
};

MPDaemon::MPDaemon()
{
    d=new MPDaemonPrivate;
    d->q=this;
    d->m_stop_requested=false;
    d->m_is_running=false;

    QTimer::singleShot(1000,this,SLOT(slot_timer()));
}

MPDaemon::~MPDaemon()
{
    delete d;
}

bool MPDaemon::run()
{
    d->m_stop_requested=false;

    while (!d->m_stop_requested) {
        qApp->processEvents();
    }
    return true;
}

QString MPDaemon::daemonPath()
{
    QString ret=qApp->applicationDirPath()+"/mpdaemon";
    if (!QDir(ret).exists()) {
        /// Witold is there a better way to mkdir if not exists?
        QDir(QFileInfo(ret).path()).mkdir(QFileInfo(ret).fileName());
    }
    return ret;
}

QString MPDaemon::makeTimestamp(const QDateTime &dt)
{
    return dt.toString("yyyy-MM-dd-hh-mm-ss-zzz");
}

void MPDaemon::slot_timer()
{
    d->write_info();
    QTimer::singleShot(4000,this,SLOT(slot_timer()));
}

void MPDaemonPrivate::write_info()
{
    /// Witold rather than starting at 100000, I'd like to format the num in the fname to be link 0000023. Could you please help?
    long num=100000;
    QString timestamp=MPDaemon::makeTimestamp();
    QString fname=QString("%1/%2.%3.info").arg(MPDaemon::daemonPath()).arg(num).arg(timestamp);
    num++;
    QJsonObject info;
    info["is_running"]=m_is_running;
    QString json=QJsonDocument(info).toJson();
    write_text_file(fname+".tmp",json);
    /// Witold I don't think rename is an atomic operation. Is there a way to guarantee that I don't read the file halfway through the rename?
    QFile::rename(fname+".tmp",fname);
}
