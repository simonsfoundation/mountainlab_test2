/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland, Witold Wysota
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

#include <QDebug>
#include <QSharedMemory>
#include "mlcommon.h"
#include <signal.h>
#include "localserver.h"
#include <QThread>

class MountainProcessClient : public LocalClient::Client {
public:
    MountainProcessClient(QObject *parent = 0) : LocalClient::Client(parent) {

    }
    QByteArray waitForMessage() {
        m_waitingForMessage = true;
        while(true) {
            if (!waitForReadyRead())
                return QByteArray(); // failure
//            qApp->sendPostedEvents(this);
            if (!m_waitingForMessage) {
                QByteArray result = m_msg;
                m_msg.clear();
                return result;
            }
        }
    }

protected:
    void handleMessage(const QByteArray &ba) Q_DECL_OVERRIDE
    {
        if (m_waitingForMessage) {
            m_msg = ba;
            m_waitingForMessage = false;
            return;
        }
    }
    bool m_waitingForMessage = false;
    QByteArray m_msg;
};

class MPDaemonInterfacePrivate {
public:
    MPDaemonInterfacePrivate(MPDaemonInterface *qq) : q(qq){
        client = new MountainProcessClient;
    }
    ~MPDaemonInterfacePrivate() {
        delete client;
    }

    MPDaemonInterface* q;
    MountainProcessClient* client;

    bool daemon_is_running();
    bool send_daemon_command(QJsonObject obj, qint64 timeout_msec);
    QDateTime get_time_from_timestamp_of_fname(QString fname);
    QJsonObject get_last_daemon_state();
};

MPDaemonInterface::MPDaemonInterface()
{
    d = new MPDaemonInterfacePrivate(this);
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
    args << "daemon-start";
    if (!QProcess::startDetached(exe, args)) {
        printf("Unable to startDetached: %s\n", exe.toLatin1().data());
        return false;
    }
    for (int i = 0; i < 10; i++) {
        MPDaemon::wait(100);
        if (d->daemon_is_running())
            return true;
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
    QThread::sleep(1);
    if (!d->daemon_is_running()) {
        printf("daemon has been stopped.\n");
        return true;
    }
    else {
        printf("Failed to stop daemon\n");
        return false;
    }
}

QJsonObject MPDaemonInterface::getDaemonState()
{
    return d->get_last_daemon_state();
}

static QString daemon_message = "Open a terminal and run [mountainprocess daemon-start], and keep that terminal open. Alternatively use tmux to run the daemon in the background.";

bool MPDaemonInterface::queueScript(const MPDaemonPript& script)
{
    if (!d->daemon_is_running()) {
        if (!this->start()) {
            printf("Problem in queueScript: Unable to start daemon.\n");
            return false;
        }
    }
    QJsonObject obj = pript_struct_to_obj(script, FullRecord);
    obj["command"] = "queue-script";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterface::queueProcess(const MPDaemonPript& process)
{
    if (!d->daemon_is_running()) {
        if (!this->start()) {
            printf("Problem in queueProcess: Unable to start daemon.\n");
            return false;
        }
    }
    QJsonObject obj = pript_struct_to_obj(process, FullRecord);
    obj["command"] = "queue-process";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterface::clearProcessing()
{
    QJsonObject obj;
    obj["command"] = "clear-processing";
    return d->send_daemon_command(obj, 0);
}

bool MPDaemonInterfacePrivate::daemon_is_running()
{
    QSharedMemory shm("mountainprocess");
    if (!shm.attach(QSharedMemory::ReadOnly))
        return false;
    shm.lock();
    const MountainProcessDescriptor* desc = reinterpret_cast<const MountainProcessDescriptor*>(shm.constData());
    bool ret = (kill(desc->pid, 0) == 0);
    shm.unlock();
    return ret;
}

bool MPDaemonInterfacePrivate::send_daemon_command(QJsonObject obj, qint64 msec_timeout)
{
    if (!msec_timeout)
        msec_timeout = 1000;
    if (!client->isConnected()) {
        client->connectToServer("mountainprocess.sock");
    }
    if (!client->waitForConnected())
        return false;
    client->writeMessage(QJsonDocument(obj).toJson());
    QByteArray msg = client->waitForMessage();
    if (msg.isEmpty()) return false;
    // TODO: parse message
    return true;
}

QJsonObject MPDaemonInterfacePrivate::get_last_daemon_state()
{
    QJsonObject ret;
    if (!daemon_is_running()) {
        ret["is_running"] = false;
        return ret;
    }
    client->connectToServer("mountainprocess.sock");
    if (!client->waitForConnected()) {
        qWarning() << "Can't connect to daemon";
        return ret;
    }
    QJsonObject obj;
    obj["command"] = "get-daemon-state";
    client->writeMessage(QJsonDocument(obj).toJson());
    QByteArray msg = client->waitForMessage();
    if (msg.isEmpty()) return ret;

    QJsonParseError error;
    ret = QJsonDocument::fromJson(msg, &error).object();
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error in get_last_daemon_state parsing json";
    }
    return ret;
}

QDateTime MPDaemonInterfacePrivate::get_time_from_timestamp_of_fname(QString fname)
{
    QStringList list = QFileInfo(fname).fileName().split(".");
    return MPDaemon::parseTimestamp(list.value(0));
}
