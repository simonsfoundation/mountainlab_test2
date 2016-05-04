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
#include "unistd.h" //for usleep

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
    bool handle_scripts();
    int num_running_scripts();
    int num_pending_scripts();
    bool launch_next_script();
    bool launch_script(QString script_id);

    /////////////////////////////////
    bool handle_processes();
    int num_running_processes();
    int num_pending_processes();
    bool launch_next_process();
    bool launch_process(QString process_id);
};

MPDaemon::MPDaemon()
{
    d = new MPDaemonPrivate;
    d->q = this;
    d->m_is_running = false;

    foreach(QProcess * P, d->m_script_qprocesses)
    {
        if (P->state() == QProcess::Running)
            P->terminate();
    }
    qDeleteAll(d->m_script_qprocesses);

    foreach(QProcess * P, d->m_process_qprocesses)
    {
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

    if (!d->m_watcher.directories().isEmpty()) {
        d->m_watcher.removePaths(d->m_watcher.directories());
    }
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

bool MPDaemon::waitForFileToAppear(QString fname, qint64 timeout_ms, bool remove_on_appear)
{
    QTime timer;
    timer.start();
    while (!QFile::exists(fname)) {
        if ((timeout_ms >= 0) && (timer.elapsed() > timeout_ms))
            return false;
        wait(100);
    }
    if (remove_on_appear) {
        QFile::remove(fname);
    }
    return true;
}

void MPDaemon::wait(qint64 msec)
{
    usleep(msec * 1000);
}

void MPDaemon::slot_commands_directory_changed()
{
    QString path = MPDaemon::daemonPath() + "/commands";
    QStringList fnames = QDir(path).entryList(QStringList("*.command"), QDir::Files, QDir::Name);
    foreach(QString fname, fnames)
    {
        QString path0 = path + "/" + fname;
        qint64 elapsed_sec = QFileInfo(path0).lastModified().secsTo(QDateTime::currentDateTime());
        if (elapsed_sec > 20) {
            qWarning() << "Removing old command file:" << path0;
            QFile::remove(path0);
        } else {
            QString json = read_text_file(path0);
            QJsonObject obj = QJsonDocument::fromJson(json.toLatin1()).object();
            d->process_command(obj);
            if (!QFile::remove(path0)) {
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
    MPDaemonScript* S = &d->m_scripts[script_id];
    S->is_finished = true;
    S->is_running = false;
    if (!S->script_output_file.isEmpty()) {
        QString run_time_results_json = read_text_file(S->script_output_file);
        if (run_time_results_json.isEmpty()) {
            S->success = false;
            S->error = "Could not read results file: " + S->script_output_file;
        } else {
            S->run_time_results = QJsonDocument::fromJson(run_time_results_json.toLatin1()).object();
            S->success = S->run_time_results["success"].toBool();
            S->error = S->run_time_results["error"].toString();
        }
    } else {
        S->success = true;
    }
    printf("  Script %s finished ", script_id.toLatin1().data());
    if (S->success)
        printf("successfully\n");
    else
        printf("with error: %s\n", S->error.toLatin1().data());
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

    MPDaemonProcess* PP = &d->m_processes[process_id];
    PP->is_finished = true;
    PP->is_running = false;
    if (!PP->process_output_file.isEmpty()) {
        QString run_time_results_json = read_text_file(PP->process_output_file);
        if (run_time_results_json.isEmpty()) {
            PP->success = false;
            PP->error = "Could not read results file: " + PP->process_output_file;
        } else {
            PP->run_time_results = QJsonDocument::fromJson(run_time_results_json.toLatin1()).object();
            PP->success = PP->run_time_results["success"].toBool();
            PP->error = PP->run_time_results["error"].toString();
        }
    } else {
        PP->success = true;
    }
    printf("  Process %s %s finished ", PP->processor_name.toLatin1().data(), process_id.toLatin1().data());
    if (PP->success)
        printf("successfully\n");
    else
        printf("with error: %s\n", PP->error.toLatin1().data());
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
        foreach(QString key, script_keys)
        {
            scripts[key] = script_struct_to_obj(m_scripts[key]);
        }
        info["scripts"] = scripts;
    }

    {
        QJsonObject processes;
        QStringList process_keys = m_processes.keys();
        foreach(QString key, process_keys)
        {
            processes[key] = process_struct_to_obj(m_processes[key]);
        }
        info["processes"] = processes;
    }

    QString json = QJsonDocument(info).toJson();
    write_text_file(fname + ".tmp", json);
    /// Witold I don't think rename is an atomic operation. Is there a way to guarantee that I don't read the file halfway through the rename?
    QFile::rename(fname + ".tmp", fname);

    //finally, clean up
    QStringList list = QDir(MPDaemon::daemonPath() + "/info").entryList(QStringList("*.info"), QDir::Files, QDir::Name);
    foreach(QString fname, list)
    {
        QString path0 = MPDaemon::daemonPath() + "/info/" + fname;
        qint64 secs = QFileInfo(path0).lastModified().secsTo(QDateTime::currentDateTime());
        if ((secs <= -60) || (secs >= 60)) { //I feel a bit paranoid. That's why I allow some future stuff.
            QFile::remove(path0);
        }
    }
}

void MPDaemonPrivate::process_command(QJsonObject obj)
{
    QString command = obj.value("command").toString();
    if (command == "stop") {
        m_is_running = false;
        write_info();
    } else if (command == "queue-script") {
        MPDaemonScript S = script_obj_to_struct(obj);
        if (m_scripts.contains(S.script_id)) {
            qWarning() << "Unable to queue script. Script with this id already exists: " + S.script_id;
            return;
        }
        printf("QUEUING SCRIPT %s\n", S.script_id.toLatin1().data());
        m_scripts[S.script_id] = S;
    } else if (command == "queue-process") {
        MPDaemonProcess P = process_obj_to_struct(obj);
        if (m_processes.contains(P.process_id)) {
            qWarning() << "Unable to queue process. Process with this id already exists: " + P.process_id;
            return;
        }
        printf("QUEUING PROCESS %s %s\n", P.processor_name.toLatin1().data(), P.process_id.toLatin1().data());
        m_processes[P.process_id] = P;
    } else {
        qWarning() << "Unrecognized command: " + command;
    }
}

bool MPDaemonPrivate::handle_scripts()
{
    if (num_running_scripts() < 1) {
        if (num_pending_scripts() > 0) {
            if (!launch_next_script()) {
                qWarning() << "Unexpected problem. Failed to launch_next_script";
                return false;
            }
        }
    }
    return true;
}

int MPDaemonPrivate::num_running_scripts()
{
    int ret = 0;
    QStringList keys = m_scripts.keys();
    foreach(QString key, keys)
    {
        if (m_scripts[key].is_running)
            ret++;
    }
    return ret;
}

int MPDaemonPrivate::num_pending_scripts()
{
    int ret = 0;
    QStringList keys = m_scripts.keys();
    foreach(QString key, keys)
    {
        if ((!m_scripts[key].is_running) && (!m_scripts[key].is_finished))
            ret++;
    }
    return ret;
}

bool MPDaemonPrivate::launch_next_script()
{
    QStringList keys = m_scripts.keys();
    foreach(QString key, keys)
    {
        if ((!m_scripts[key].is_running) && (!m_scripts[key].is_finished)) {
            return launch_script(key);
        }
    }
    return false;
}

bool MPDaemonPrivate::launch_script(QString script_id)
{
    if (!m_scripts.contains(script_id))
        return false;
    MPDaemonScript* S = &m_scripts[script_id];
    if (S->is_running)
        return false;
    if (S->is_finished)
        return false;
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "run-script";
    if (!S->script_output_file.isEmpty()) {
        args << "--~script_output=" + S->script_output_file;
    }
    foreach(QString fname, S->script_paths)
    {
        args << fname;
    }
    QJsonObject parameters = variantmap_to_json_obj(S->parameters);
    QString parameters_json = QJsonDocument(parameters).toJson();
    QString par_fname = CacheManager::globalInstance()->makeLocalFile(script_id + ".par", CacheManager::ShortTerm);
    args << par_fname;
    QProcess* qprocess = new QProcess;
    qprocess->setProperty("script_id", script_id.toLatin1().data());
    printf("   Launching script %s: ", script_id.toLatin1().data());
    foreach(QString fname, S->script_paths)
    {
        QString str = QFileInfo(fname).fileName();
        printf("%s ", str.toLatin1().data());
    }
    printf("\n");

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_script_qprocess_finished()));

    qprocess->start(exe, args);
    if (qprocess->waitForStarted()) {
        m_script_qprocesses[script_id] = qprocess;
        S->is_running = true;
        return true;

    } else {
        qWarning() << "Unable to start script: " + S->script_id;
        m_scripts.remove(script_id);
        return false;
    }
}

bool MPDaemonPrivate::handle_processes()
{
    if (num_running_processes() < 1) {
        if (num_pending_processes() > 0) {
            if (!launch_next_process()) {
                qWarning() << "Unexpected problem. Failed to launch next process.";
                return false;
            }
        }
    }
    return true;
}

int MPDaemonPrivate::num_running_processes()
{
    int ret = 0;
    QStringList keys = m_processes.keys();
    foreach(QString key, keys)
    {
        if (m_processes[key].is_running)
            ret++;
    }
    return ret;
}

int MPDaemonPrivate::num_pending_processes()
{
    int ret = 0;
    QStringList keys = m_processes.keys();
    foreach(QString key, keys)
    {
        if ((!m_processes[key].is_running) && (!m_processes[key].is_finished))
            ret++;
    }
    return ret;
}

bool MPDaemonPrivate::launch_next_process()
{
    QStringList keys = m_processes.keys();
    foreach(QString key, keys)
    {
        if ((!m_processes[key].is_running) && (!m_processes[key].is_finished)) {
            return launch_process(key);
        }
    }
    return false;
}

bool MPDaemonPrivate::launch_process(QString process_id)
{
    if (!m_processes.contains(process_id))
        return false;
    MPDaemonProcess* P = &m_processes[process_id];
    if (P->is_running)
        return false;
    if (P->is_finished)
        return false;
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "run-process";
    args << P->processor_name;
    if (!P->process_output_file.isEmpty())
        args << "--~process_output=" + P->process_output_file;
    QStringList pkeys = P->parameters.keys();
    foreach(QString pkey, pkeys)
    {
        args << QString("--%1=%2").arg(pkey).arg(P->parameters[pkey].toString());
    }
    QProcess* qprocess = new QProcess;
    qprocess->setProperty("process_id", process_id.toLatin1().data());
    printf("    Launching process %s: ", process_id.toLatin1().data());
    QString cmd = args.join(" ");
    printf("%s\n", cmd.toLatin1().data());

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_process_qprocess_finished()));

    qprocess->start(exe, args);
    if (qprocess->waitForStarted()) {
        m_process_qprocesses[process_id] = qprocess;
        P->is_running = true;
        return true;
    } else {
        qWarning() << "Unable to start process: " + P->process_id + " " + P->processor_name;
        m_processes.remove(process_id);
        return false;
    }
}

QJsonArray stringlist_to_json_array(QStringList list)
{
    QJsonArray ret;
    foreach(QString str, list)
    {
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
    foreach(QString key, keys)
    {
        /// Witold I would like to map numbers to numbers here. Can you help?
        ret[key] = QJsonValue(map[key].toString());
    }
    return ret;
}

QVariantMap json_obj_to_variantmap(QJsonObject obj)
{
    QVariantMap ret;
    QStringList keys = obj.keys();
    foreach(QString key, keys)
    {
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
    ret["script_output_file"] = S.script_output_file;
    ret["success"] = S.success;
    ret["error"] = S.error;
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
    ret.script_output_file = obj.value("script_output_file").toString();
    ret.success = obj.value("success").toBool();
    ret.error = obj.value("error").toString();
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
    ret["process_output_file"] = P.process_output_file;
    ret["success"] = P.success;
    ret["error"] = P.error;
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
    ret.process_output_file = obj.value("process_output_file").toString();
    ret.success = obj.value("success").toBool();
    ret.error = obj.value("error").toString();
    return ret;
}
