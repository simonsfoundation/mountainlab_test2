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
    void wait(qint64 msec); //Maybe not used
    bool send_daemon_command(QJsonObject obj,qint64 timeout_msec);
    QString last_info_fname();
    QJsonObject get_last_info(qint64 max_elapsed_msec);
    qint64 msec_since_last_info(); //Maybe not used
    QDateTime get_time_from_timestamp_of_fname(QString fname);
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
    d->send_daemon_command(obj,5000);
    /// Witold even though this is more than enough time for Daemon to stop itself once it has acknowledge receipt of the command by deleting the file, there should be a better way
    d->wait(1000); //give it some time after command has been received
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
    return d->get_last_info(10000);
}

void MPDaemonInterface::queueScript(const MPDaemonScript &script)
{
    QJsonObject obj=script_struct_to_obj(script);
    obj["command"]="queue-script";
    d->send_daemon_command(obj,0);
}

bool MPDaemonInterfacePrivate::daemon_is_running()
{
    QJsonObject obj=get_last_info(10000);
    return obj.value("is_running").toBool();
}

void MPDaemonInterfacePrivate::wait(qint64 msec)
{
    QTime timer; timer.start();
    while (timer.elapsed()<=msec) {}
}

bool MPDaemonInterfacePrivate::send_daemon_command(QJsonObject obj,qint64 msec_timeout)
{
    static long num=100000;
    QString timestamp=MPDaemon::makeTimestamp();
    QString fname=QString("%1/commands/%2.%3.command").arg(MPDaemon::daemonPath()).arg(timestamp).arg(num);
    num++;

    QString json=QJsonDocument(obj).toJson();
    write_text_file(fname,json);
    QTime timer; timer.start();
    //wait until it has been received by the daemon
    while ((timer.elapsed()<=msec_timeout)&&(QFile::exists(fname)));
    return (!QFile::exists(fname));
}

QString MPDaemonInterfacePrivate::last_info_fname()
{
    QString path=MPDaemon::daemonPath()+"/info";
    QStringList list=QDir(path).entryList(QStringList("*.info"),QDir::Files,QDir::Name);
    if (list.isEmpty()) return "";
    return path+"/"+list[list.count()-1];
}

QJsonObject MPDaemonInterfacePrivate::get_last_info(qint64 max_elapsed_msec)
{
    QJsonObject ret;
    QString fname=last_info_fname();
    if (fname.isEmpty()) return ret;
    qint64 elapsed=get_time_from_timestamp_of_fname(fname).msecsTo(QDateTime::currentDateTime());
    if (elapsed>max_elapsed_msec) return ret;
    QString json=read_text_file(fname);
    ret=QJsonDocument::fromJson(json.toLatin1()).object();
    return ret;
}

qint64 MPDaemonInterfacePrivate::msec_since_last_info()
{
    QString fname=last_info_fname();
    if (fname.isEmpty()) return 999000;
    return get_time_from_timestamp_of_fname(fname).msecsTo(QDateTime::currentDateTime());
}

QDateTime MPDaemonInterfacePrivate::get_time_from_timestamp_of_fname(QString fname)
{
    QStringList list=QFileInfo(fname).fileName().split(".");
    return MPDaemon::parseTimestamp(list.value(0));
}
