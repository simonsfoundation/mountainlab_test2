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

class MPDaemonPrivate {
public:
    MPDaemon *q;
    bool m_stop_requested;
    bool m_is_running;
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

void MPDaemon::slot_timer()
{
    d->write_info();
    QTimer::singleShot(4000,this,SLOT(slot_timer()));
}
