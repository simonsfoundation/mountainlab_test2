/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/2/2016
*******************************************************/

#include "mpdaemoninterface.h"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QProcess>
#include <QTime>
#include <stdio.h>
#include "mpdaemon.h"
#include "textfile.h"

class MPDaemonInterfacePrivate {
public:
    MPDaemonInterface *q;

    bool daemon_is_running();
    void wait(int msec);
    void send_daemon_command(QJsonObject obj);
    QString last_info_fname();
    int msec_since_last_info();
    QString get_timestamp();
};

MPDaemonInterface::MPDaemonInterface()
{
    d=new MPDaemonInterfacePrivate;
    d->q=this;
}

MPDaemonInterface::~MPDaemonInterface()
{
    delete d;
}

bool MPDaemonInterface::start()
{
    if (d->daemon_is_running()) {
        printf("daemon is already running.\n");
        return true;
    }
    QString exe=qApp->applicationFilePath();
    QStringList args;
    args << "-internal-daemon-start";
    return QProcess::startDetached(exe,args);
}

bool MPDaemonInterface::stop()
{
    if (!d->daemon_is_running()) {
        printf("daemon is not running.\n");
        return true;
    }
    QJsonObject obj;
    obj["command"]="stop";
    d->send_daemon_command(obj);
    if (!d->daemon_is_running()) {
        printf("daemon has been stopped.\n");
        return true;
    }
    else {
        printf("Failed to stop daemon\n");
        return false;
    }
}

QJsonObject MPDaemonInterface::getInfo()
{
    QJsonObject ret;
    QString fname=d->last_info_fname();
    if (fname.isEmpty()) return ret;
    QString json=read_text_file(fname);
    ret=QJsonDocument::fromJson(json.toLatin1()).object();
    return ret;
}



bool MPDaemonInterfacePrivate::daemon_is_running()
{
    double elapsed=msec_since_last_info();
    if (elapsed>10000) return false;
    if (elapsed<=5000) {
        QJsonObject info=q->getInfo();
        return info["running"].toBool();
    }
    else {
        //wait up to 5 seconds for next info
        for (int i=1; i<=5; i++) {
            wait(1000);
            if (msec_since_last_info()<=5000) {
                QJsonObject info=q->getInfo();
                return info["running"].toBool();
            }
        }
    }
    return false;
}

void MPDaemonInterfacePrivate::wait(int msec)
{
    QTime timer; timer.start();
    while (timer.elapsed()<=msec) {}
}

void MPDaemonInterfacePrivate::send_daemon_command(QJsonObject obj)
{
    static int num=1;
    QString timestamp=get_timestamp();
    QString fname=QString("%1/%2.%3.command").arg(MPDaemon::daemonPath()).arg(timestamp).arg(num);
    num++;

    QString json=QJsonDocument(obj).toJson();
    write_text_file(fname,json);
    QTime timer; timer.start();
    while ((timer.elapsed()<=6000)&&(QFile::exists(fname)));
}

QString MPDaemonInterfacePrivate::last_info_fname()
{
    QString path=MPDaemon::daemonPath();
    QStringList list=QDir(path).entryList(QStringList("*.info"),QDir::Files,QDir::Time);
    if (list.isEmpty()) return "";
    return path+"/"+list[list.count()-1];
}

int MPDaemonInterfacePrivate::msec_since_last_info()
{
    QString fname=last_info_fname();
    if (fname.isEmpty()) return 9999;
    return QFileInfo(fname).lastModified().msecsTo(QDateTime::currentDateTime());
}

QString MPDaemonInterfacePrivate::get_timestamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-zzz");
}
