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
#include <QProcess>
#include "textfile.h"
#include <QDebug>
#include "cachemanager.h"

/// TODO clean up the old files in mpdaemon/info
/// TODO consider separating out a class with an API other than from command files -- probably a very good idea, if I'm not too lazy

class MPDaemonPrivate {
public:
    MPDaemon* q;
    bool m_is_running;
    QFileSystemWatcher m_watcher;
    QMap<QString, MPDaemonScript> m_scripts;
    QMap<QString, QProcess*> m_script_qprocesses;
    QMap<QString, MPDaemonProcess> m_processes;
    QMap<QString, QProcess*> m_process_qprocesses;

    void write_info();
    void process_command(QJsonObject obj);

    /////////////////////////////////
    void handle_scripts();
    int num_running_scripts();
    int num_pending_scripts();
    void launch_next_script();
    void launch_script(QString script_id);

    /////////////////////////////////
    void handle_processes();
    int num_running_processes();
    int num_pending_processes();
    void launch_next_process();
    void launch_process(QString process_id);
};

MPDaemon::MPDaemon()
{
    d = new MPDaemonPrivate;
    d->q = this;
    d->m_is_running = false;

    foreach (QProcess* P, d->m_script_qprocesses) {
        if (P->state() == QProcess::Running)
            P->terminate();
    }
    qDeleteAll(d->m_script_qprocesses);

    foreach (QProcess* P, d->m_process_qprocesses) {
        if (P->state() == QProcess::Running)
            P->terminate();
    }
    qDeleteAll(d->m_process_qprocesses);

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
        d->handle_scripts();
        d->handle_processes();
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
    mkdir_if_doesnt_exist(ret + "/completed_processes");
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

void MPDaemon::slot_script_qprocess_finished()
{
    QProcess* P = qobject_cast<QProcess*>(sender());
    if (!P)
        return;
    QString script_id = P->property("script_id").toString();
    if (!d->m_scripts.contains(script_id)) {
        qWarning() << "Unexpected problem in slot_process_qprocess_finished. Unable to find script with id: " + script_id;
        return;
    }
    d->m_scripts[script_id].is_finished = true;
    d->m_scripts[script_id].is_running = false;
    /// TODO read the standard output and standard error
}

void MPDaemon::slot_process_qprocess_finished()
{
    QProcess* P = qobject_cast<QProcess*>(sender());
    if (!P)
        return;
    QString process_id = P->property("process_id").toString();
    if (!d->m_processes.contains(process_id)) {
        qWarning() << "Unexpected problem in slot_process_qprocess_finished. Unable to find process with id: " + process_id;
        return;
    }
    d->m_processes[process_id].is_finished = true;
    d->m_processes[process_id].is_running = false;
    /// TODO read the standard output and standard error
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

    {
        QJsonObject scripts;
        QStringList script_keys = m_scripts.keys();
        foreach (QString key, script_keys) {
            scripts[key] = script_struct_to_obj(m_scripts[key]);
        }
        info["scripts"] = scripts;
    }

    {
        QJsonObject processes;
        QStringList process_keys = m_processes.keys();
        foreach (QString key, process_keys) {
            processes[key] = process_struct_to_obj(m_processes[key]);
        }
        info["processes"] = processes;
    }

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
            qWarning() << "Unable to queue script. Script with this id already exists: " + S.script_id;
            return;
        }
        QString json = QJsonDocument(script_struct_to_obj(S)).toJson(); //there's a reason we convert back in this way. don't be so hasty to judge
        printf("QUEUING SCRIPT:\n");
        printf("%s\n", json.toLatin1().data());
        m_scripts[S.script_id] = S;
    }
    else if (command == "queue-process") {
        MPDaemonProcess P = process_obj_to_struct(obj);
        if (m_processes.contains(P.process_id)) {
            qWarning() << "Unable to queue process. Process with this id already exists: " + P.process_id;
            return;
        }
        QString json = QJsonDocument(process_struct_to_obj(P)).toJson(); //there's a reason we convert back in this way. don't be so hasty to judge
        printf("QUEUING PROCESS:\n");
        printf("%s\n", json.toLatin1().data());
        m_processes[P.process_id] = P;
    }
    else {
        qWarning() << "Unrecognized command: " + command;
    }
}

void MPDaemonPrivate::handle_scripts()
{
    if (num_running_scripts() < 1) {
        if (num_pending_scripts() > 0) {
            launch_next_script();
        }
    }
}

int MPDaemonPrivate::num_running_scripts()
{
    int ret = 0;
    QStringList keys = m_scripts.keys();
    foreach (QString key, keys) {
        if (m_scripts[key].is_running)
            ret++;
    }
    return ret;
}

int MPDaemonPrivate::num_pending_scripts()
{
    int ret = 0;
    QStringList keys = m_scripts.keys();
    foreach (QString key, keys) {
        if ((!m_scripts[key].is_running) && (!m_scripts[key].is_finished))
            ret++;
    }
    return ret;
}

void MPDaemonPrivate::launch_next_script()
{
    QStringList keys = m_scripts.keys();
    foreach (QString key, keys) {
        if ((!m_scripts[key].is_running) && (!m_scripts[key].is_finished)) {
            launch_script(key);
            return;
        }
    }
}

void MPDaemonPrivate::launch_script(QString script_id)
{
    if (!m_scripts.contains(script_id))
        return;
    MPDaemonScript* S = &m_scripts[script_id];
    if (S->is_running)
        return;
    if (S->is_finished)
        return;
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "run-script";
    if (!S->script_output_file.isEmpty()) {
        args << "--~script_output=" + S->script_output_file;
    }
    foreach (QString fname, S->script_paths) {
        args << fname;
    }
    QJsonObject parameters = variantmap_to_json_obj(S->parameters);
    QString parameters_json = QJsonDocument(parameters).toJson();
    QString par_fname = CacheManager::globalInstance()->makeLocalFile(script_id + ".par", CacheManager::ShortTerm);
    args << par_fname;
    QProcess* qprocess = new QProcess;
    qprocess->setProperty("script_id", script_id.toLatin1().data());
    printf("Launching script %s: ", script_id.toLatin1().data());
    foreach (QString fname, S->script_paths) {
        QString str = QFileInfo(fname).fileName();
        printf("%s ", str.toLatin1().data());
    }
    printf("\n");

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_script_qprocess_finished()));

    qprocess->start(exe, args);
    m_script_qprocesses[script_id] = qprocess;
    S->is_running = true;
}

void MPDaemonPrivate::handle_processes()
{
    if (num_running_processes() < 1) {
        if (num_pending_processes() > 0) {
            launch_next_process();
        }
    }
}

int MPDaemonPrivate::num_running_processes()
{
    int ret = 0;
    QStringList keys = m_processes.keys();
    foreach (QString key, keys) {
        if (m_processes[key].is_running)
            ret++;
    }
    return ret;
}

int MPDaemonPrivate::num_pending_processes()
{
    int ret = 0;
    QStringList keys = m_processes.keys();
    foreach (QString key, keys) {
        if ((!m_processes[key].is_running) && (!m_processes[key].is_finished))
            ret++;
    }
    return ret;
}

void MPDaemonPrivate::launch_next_process()
{
    QStringList keys = m_processes.keys();
    foreach (QString key, keys) {
        if ((!m_processes[key].is_running) && (!m_processes[key].is_finished)) {
            launch_process(key);
            return;
        }
    }
}

void MPDaemonPrivate::launch_process(QString process_id)
{
    if (!m_processes.contains(process_id))
        return;
    MPDaemonProcess* P = &m_processes[process_id];
    if (P->is_running)
        return;
    if (P->is_finished)
        return;
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "run-process";
    args << P->processor_name;
    if (!P->process_output_file.isEmpty())
        args << "--~process_output=" + P->process_output_file;
    QStringList pkeys = P->parameters.keys();
    foreach (QString pkey, pkeys) {
        args << QString("--%1=%2").arg(pkey).arg(P->parameters[pkey].toString());
    }
    QProcess* qprocess = new QProcess;
    qprocess->setProperty("process_id", process_id.toLatin1().data());
    printf("Launching process %s: ", process_id.toLatin1().data());
    QString cmd = args.join(" ");
    printf("%s\n", cmd.toLatin1().data());

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_process_qprocess_finished()));

    qprocess->start(exe, args);
    m_process_qprocesses[process_id] = qprocess;
    P->is_running = true;
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

QJsonObject process_struct_to_obj(MPDaemonProcess P)
{
    QJsonObject ret;
    ret["is_finished"] = P.is_finished;
    ret["is_running"] = P.is_running;
    ret["processor_name"] = P.processor_name;
    ret["parameters"] = variantmap_to_json_obj(P.parameters);
    ret["process_id"] = P.process_id;
    return ret;
}

MPDaemonProcess process_obj_to_struct(QJsonObject obj)
{
    MPDaemonProcess ret;
    ret.is_finished = obj.value("is_finished").toBool();
    ret.is_running = obj.value("is_running").toBool();
    ret.processor_name = obj.value("processor_name").toString();
    ret.parameters = json_obj_to_variantmap(obj.value("parameters").toObject());
    ret.process_id = obj.value("process_id").toString();
    return ret;
}
