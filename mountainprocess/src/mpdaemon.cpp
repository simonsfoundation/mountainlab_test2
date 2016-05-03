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
#include <QFileSystemWatcher>
#include <QJsonArray>
#include "textfile.h"
#include <QDebug>

/// TODO clean up the old files in mpdaemon/info
/// TODO consider separating out a class with an API other than from command files -- probably a very good idea, if I'm not too lazy

class MPDaemonPrivate {
public:
    MPDaemon* q;
    bool m_is_running;
    QFileSystemWatcher m_watcher;
    QMap<QString, MPDaemonScript> m_scripts;

    void write_info();
    void process_command(QJsonObject obj);
};

MPDaemon::MPDaemon()
{
    d = new MPDaemonPrivate;
    d->q = this;
    d->m_is_running = false;

    connect(&d->m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slot_commands_directory_changed()));
}

MPDaemon::~MPDaemon()
{
    delete d;
}

bool MPDaemon::run()
{
    if (d->m_is_running) {
        qWarning() << "Unexpected problem in MPDaemon:run(). Daemon is already running.";
        return false;
    }
    d->m_is_running = true;

    d->m_watcher.removePaths(d->m_watcher.directories());
    d->m_watcher.addPath(MPDaemon::daemonPath() + "/commands");

    QTime timer;
    timer.start();
    d->write_info();
    while (d->m_is_running) {
        if (timer.elapsed() > 5000) {
            d->write_info();
            timer.restart();
        }
        qApp->processEvents();
    }
    return true;
}

void mkdir_if_doesnt_exist(QString path)
{
    /// Witold is there a better way to mkdir if not exists?
    if (!QDir(path).exists()) {
        QDir(QFileInfo(path).path()).mkdir(QFileInfo(path).fileName());
    }
}

QString MPDaemon::daemonPath()
{
    QString ret = qApp->applicationDirPath() + "/mpdaemon";
    mkdir_if_doesnt_exist(ret);
    mkdir_if_doesnt_exist(ret + "/info");
    mkdir_if_doesnt_exist(ret + "/commands");
    return ret;
}

QString MPDaemon::makeTimestamp(const QDateTime& dt)
{
    return dt.toString("yyyy-MM-dd-hh-mm-ss-zzz");
}

QDateTime MPDaemon::parseTimestamp(const QString& timestamp)
{
    return QDateTime::fromString(timestamp, "yyyy-MM-dd-hh-mm-ss-zzz");
}

void MPDaemon::slot_commands_directory_changed()
{
    QString path = MPDaemon::daemonPath() + "/commands";
    QStringList fnames = QDir(path).entryList(QStringList("*.command"), QDir::Files, QDir::Name);
    foreach (QString fname, fnames) {
        QString path0 = path + "/" + fname;
        qint64 elapsed_sec = QFileInfo(path0).lastModified().secsTo(QDateTime::currentDateTime());
        if (elapsed_sec > 20) {
            qWarning() << "Removing old command file:" << path0;
            QFile::remove(path0);
        }
        else {
            QString json = read_text_file(path0);
            if (QFile::remove(path0)) {
                QJsonObject obj = QJsonDocument::fromJson(json.toLatin1()).object();
                d->process_command(obj);
            }
            else {
                qWarning() << "Problem removing command file: " + path0;
            }
        }
    }
}

void MPDaemonPrivate::write_info()
{
    /// Witold rather than starting at 100000, I'd like to format the num in the fname to be link 0000023. Could you please help?
    static long num = 100000;
    QString timestamp = MPDaemon::makeTimestamp();
    QString fname = QString("%1/info/%2.%3.info").arg(MPDaemon::daemonPath()).arg(timestamp).arg(num);
    num++;

    QJsonObject info;

    info["is_running"] = m_is_running;

    QJsonObject scripts;
    QStringList script_keys = m_scripts.keys();
    foreach (QString key, script_keys) {
        scripts[key] = script_struct_to_obj(m_scripts[key]);
    }
    info["scripts"] = scripts;

    QString json = QJsonDocument(info).toJson();
    write_text_file(fname + ".tmp", json);
    /// Witold I don't think rename is an atomic operation. Is there a way to guarantee that I don't read the file halfway through the rename?
    QFile::rename(fname + ".tmp", fname);
}

void MPDaemonPrivate::process_command(QJsonObject obj)
{
    QString command = obj.value("command").toString();
    if (command == "stop") {
        m_is_running = false;
        write_info();
    }
    else if (command == "queue-script") {
        MPDaemonScript S = script_obj_to_struct(obj);
        if (m_scripts.contains(S.script_id)) {
            qWarning() << "Unable to queue script. Script with this id already exists: "+S.script_id;
            return;
        }
        QString json = QJsonDocument(script_struct_to_obj(S)).toJson();
        printf("QUEUING SCRIPT:\n");
        printf("%s\n", json.toLatin1().data());
        m_scripts[S.script_id] = S;
    }
    else {
        qWarning() << "Unrecognized command: " + command;
    }
}

QJsonArray stringlist_to_json_array(QStringList list)
{
    QJsonArray ret;
    foreach (QString str, list) {
        ret << QJsonValue(str);
    }
    return ret;
}

QStringList json_array_to_stringlist(QJsonArray X)
{
    QStringList ret;
    for (int i = 0; i < X.count(); i++) {
        ret << X[i].toString();
    }
    return ret;
}

QJsonObject variantmap_to_json_obj(QVariantMap map)
{
    QJsonObject ret;
    QStringList keys = map.keys();
    foreach (QString key, keys) {
        /// Witold I would like to map numbers to numbers here. Can you help?
        ret[key] = QJsonValue(map[key].toString());
    }
    return ret;
}

QVariantMap json_obj_to_variantmap(QJsonObject obj)
{
    QVariantMap ret;
    QStringList keys = obj.keys();
    foreach (QString key, keys) {
        ret[key] = obj[key].toVariant();
    }
    return ret;
}

QJsonObject script_struct_to_obj(MPDaemonScript S)
{
    QJsonObject ret;
    ret["is_finished"] = S.is_finished;
    ret["is_running"] = S.is_running;
    ret["script_paths"] = stringlist_to_json_array(S.script_paths);
    ret["parameters"] = variantmap_to_json_obj(S.parameters);
    ret["script_id"] = S.script_id;
    return ret;
}
MPDaemonScript script_obj_to_struct(QJsonObject obj)
{
    MPDaemonScript ret;
    ret.is_finished = obj.value("is_finished").toBool();
    ret.is_running = obj.value("is_running").toBool();
    ret.script_paths = json_array_to_stringlist(obj.value("script_paths").toArray());
    ret.parameters = json_obj_to_variantmap(obj.value("parameters").toObject());
    ret.script_id = obj.value("script_id").toString();
    return ret;
}

MPDaemonScript default_daemon_script()
{
    MPDaemonScript ret;
    ret.is_finished = false;
    ret.is_running = false;
    return ret;
}
