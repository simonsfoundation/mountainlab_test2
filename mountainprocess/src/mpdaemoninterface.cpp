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
#include <QDebug>

class MPDaemonInterfacePrivate {
public:
    MPDaemonInterface* q;

    bool daemon_is_running();
    bool send_daemon_command(QJsonObject obj, qint64 timeout_msec);
    QString last_info_fname();
    QJsonObject get_last_info(qint64 max_elapsed_msec);
    qint64 msec_since_last_info(); //Maybe not used
    QDateTime get_time_from_timestamp_of_fname(QString fname);
};

MPDaemonInterface::MPDaemonInterface()
{
    d = new MPDaemonInterfacePrivate;
    d->q = this;
}

MPDaemonInterface::~MPDaemonInterface()
{
    delete d;
}

bool MPDaemonInterface::start()
{
    if (d->daemon_is_running()) {
        printf("Cannot start: daemon is already running.\n");
        return true;
    }
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "-internal-daemon-start";
    if (!QProcess::startDetached(exe, args)) {
        printf("Unable to startDetached: %s\n",exe.toLatin1().data());
        return false;
    }
    for (int i=0; i<10; i++) {
        MPDaemon::wait(100);
        if (d->daemon_is_running()) return true;
    }
    printf("Unable to start daemon after waiting.\n");
    return false;
}

bool MPDaemonInterface::stop()
{
    if (!d->daemon_is_running()) {
        printf("Cannot stop: daemon is not running.\n");
        return true;
    }
    QJsonObject obj;
    obj["command"] = "stop";
    d->send_daemon_command(obj, 5000);
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

bool MPDaemonInterface::queueScript(const MPDaemonPript& script)
{
    if (!d->daemon_is_running()) {
        printf("Problem in queueScript: Daemon is not running.\n");
        return false;
        /*
        if (!this->start()) {
            printf("Problem in queueScript: Unable to start daemon.\n");
            return false;
        }
        */
    }
    QJsonObject obj = pript_struct_to_obj(script);
    obj["command"] = "queue-script";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterface::queueProcess(const MPDaemonPript& process)
{
    if (!d->daemon_is_running()) {
        printf("Problem in queueProcess: Daemon is not running.\n");
        return false;
        /*
        if (!this->start()) {
            printf("Problem in queueProcess: Unable to start daemon.\n");
            return false;
        }
        */
    }
    QJsonObject obj = pript_struct_to_obj(process);
    obj["command"] = "queue-process";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterfacePrivate::daemon_is_running()
{
    QJsonObject obj = get_last_info(25000); //be conservative, because ideally it will have stopped gracefully
    if (!obj.contains("is_running")) {
        printf("is_running field NOT found\n");
    }
    return obj.value("is_running").toBool();
}

bool MPDaemonInterfacePrivate::send_daemon_command(QJsonObject obj, qint64 msec_timeout)
{
    if (!msec_timeout) msec_timeout=1000;

    static long num = 100000;
    QString timestamp = MPDaemon::makeTimestamp();
    QString fname = QString("%1/commands/%2.%3.command").arg(MPDaemon::daemonPath()).arg(timestamp).arg(num);
    num++;

    QString json = QJsonDocument(obj).toJson();
    write_text_file(fname, json);
    QFile::setPermissions(fname,QFile::ReadOwner|QFile::ReadGroup|QFile::ReadOther|QFile::WriteOwner|QFile::WriteGroup|QFile::WriteOther);
    QTime timer;
    timer.start();
    //wait until it has been received by the daemon
    while ((timer.elapsed() <= msec_timeout) && (QFile::exists(fname)))
        ;
    return (!QFile::exists(fname));
}

QString MPDaemonInterfacePrivate::last_info_fname()
{
    QString path = MPDaemon::daemonPath() + "/info";
    QStringList list = QDir(path).entryList(QStringList("*.info"), QDir::Files, QDir::Name);
    if (list.isEmpty())
        return "";
    return path + "/" + list[list.count() - 1];
}

QJsonObject MPDaemonInterfacePrivate::get_last_info(qint64 max_elapsed_msec)
{
    QJsonObject ret;
    QString fname = last_info_fname();
    if (fname.isEmpty())
        return ret;
    qint64 elapsed = get_time_from_timestamp_of_fname(fname).msecsTo(QDateTime::currentDateTime());
    if (elapsed > max_elapsed_msec) {
        qDebug() << QString("get_last_info: ELAPSED %1 > %2").arg(elapsed).arg(max_elapsed_msec);
        return ret;
    }
    QString json = read_text_file(fname);
    ret = QJsonDocument::fromJson(json.toLatin1()).object();
    return ret;
}

qint64 MPDaemonInterfacePrivate::msec_since_last_info()
{
    QString fname = last_info_fname();
    if (fname.isEmpty())
        return 999000;
    return get_time_from_timestamp_of_fname(fname).msecsTo(QDateTime::currentDateTime());
}

QDateTime MPDaemonInterfacePrivate::get_time_from_timestamp_of_fname(QString fname)
{
    QStringList list = QFileInfo(fname).fileName().split(".");
    return MPDaemon::parseTimestamp(list.value(0));
}
