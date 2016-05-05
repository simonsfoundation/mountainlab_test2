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

/// TODO check for bad situation of more than one daemon running simultaneously

class MPDaemonPrivate {
public:
    MPDaemon* q;
    bool m_is_running;
    QFileSystemWatcher m_watcher;
    QMap<QString, MPDaemonPript> m_pripts;

    void write_info();
    void process_command(QJsonObject obj);

    bool handle_scripts();
    bool handle_processes();
    int num_running_scripts()
    {
        return num_running_pripts(ScriptType);
    }
    int num_running_processes()
    {
        return num_running_pripts(ProcessType);
    }
    int num_pending_scripts()
    {
        return num_pending_pripts(ScriptType);
    }
    int num_pending_processes()
    {
        return num_pending_pripts(ProcessType);
    }
    bool launch_next_script()
    {
        return launch_next_pript(ScriptType);
    }
    bool launch_next_processes()
    {
        return launch_next_pript(ProcessType);
    }

    void stop_orphan_processes_and_scripts();

    /////////////////////////////////
    int num_running_pripts(PriptType prtype);
    int num_pending_pripts(PriptType prtype);
    bool launch_next_pript(PriptType prtype);
    bool launch_pript(QString id);
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
    foreach(MPDaemonPript P, d->m_pripts)
    {
        if (P.qprocess) {
            if (P.qprocess->state() == QProcess::Running)
                P.qprocess->terminate();
        }
    }

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
        d->stop_orphan_processes_and_scripts();
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

bool MPDaemon::waitForFileToAppear(QString fname, qint64 timeout_ms, bool remove_on_appear, qint64 parent_pid)
{
    QTime timer;
    timer.start();
    while (!QFile::exists(fname)) {
        if ((timeout_ms >= 0) && (timer.elapsed() > timeout_ms))
            return false;
        if ((parent_pid) && (!MPDaemon::pidExists(parent_pid))) {
            qWarning() << "Exiting waitForFileToAppear because parent process is gone.";
            break;
        }
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

void MPDaemon::slot_pript_qprocess_finished()
{
    QProcess* P = qobject_cast<QProcess*>(sender());
    if (!P)
        return;
    QString pript_id = P->property("pript_id").toString();
    MPDaemonPript* S;
    if (d->m_pripts.contains(pript_id)) {
        S = &d->m_pripts[pript_id];
    } else {
        qWarning() << "Unexpected problem in slot_pript_qprocess_finished. Unable to find script or process with id: " + pript_id;
        return;
    }
    S->is_finished = true;
    S->is_running = false;
    if (!S->output_file.isEmpty()) {
        QString run_time_results_json = read_text_file(S->output_file);
        if (run_time_results_json.isEmpty()) {
            S->success = false;
            S->error = "Could not read results file: " + S->output_file;
        } else {
            S->run_time_results = QJsonDocument::fromJson(run_time_results_json.toLatin1()).object();
            S->success = S->run_time_results["success"].toBool();
            S->error = S->run_time_results["error"].toString();
        }
    } else {
        S->success = true;
    }
    if (S->prtype == ScriptType) {
        printf("  Script %s finished ", pript_id.toLatin1().data());
    } else {
        printf("  Process %s %s finished ", S->processor_name.toLatin1().data(), pript_id.toLatin1().data());
    }
    if (S->success)
        printf("successfully\n");
    else
        printf("with error: %s\n", S->error.toLatin1().data());
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
        QJsonObject processes;
        QStringList keys = m_pripts.keys();
        foreach(QString key, keys)
        {
            if (m_pripts[key].prtype == ScriptType)
                scripts[key] = pript_struct_to_obj(m_pripts[key]);
            else
                processes[key] = pript_struct_to_obj(m_pripts[key]);
        }
        info["scripts"] = scripts;
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
        MPDaemonPript S = pript_obj_to_struct(obj);
        S.prtype = ScriptType;
        if (m_pripts.contains(S.id)) {
            qWarning() << "Unable to queue script. Process or script with this id already exists: " + S.id;
            return;
        }
        printf("QUEUING SCRIPT %s\n", S.id.toLatin1().data());
        m_pripts[S.id] = S;
    } else if (command == "queue-process") {
        MPDaemonPript P = pript_obj_to_struct(obj);
        P.prtype = ProcessType;
        if (m_pripts.contains(P.id)) {
            qWarning() << "Unable to queue process. Process or script with this id already exists: " + P.id;
            return;
        }
        printf("QUEUING PROCESS %s %s\n", P.processor_name.toLatin1().data(), P.id.toLatin1().data());
        m_pripts[P.id] = P;
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

bool MPDaemonPrivate::launch_next_pript(PriptType prtype)
{
    QStringList keys = m_pripts.keys();
    foreach(QString key, keys)
    {
        if (m_pripts[key].prtype == prtype) {
            if ((!m_pripts[key].is_running) && (!m_pripts[key].is_finished)) {
                return launch_pript(key);
            }
        }
    }
    return false;
}

bool MPDaemonPrivate::launch_pript(QString pript_id)
{
    MPDaemonPript* S;
    if (!m_pripts.contains(pript_id))
        return false;
    S = &m_pripts[pript_id];
    if (S->is_running)
        return false;
    if (S->is_finished)
        return false;

    QString exe = qApp->applicationFilePath();
    QStringList args;

    if (S->prtype == ScriptType) {
        args << "run-script";
        if (!S->output_file.isEmpty()) {
            args << "--~script_output=" + S->output_file;
        }
        foreach(QString fname, S->script_paths)
        {
            args << fname;
        }
        QJsonObject parameters = variantmap_to_json_obj(S->parameters);
        QString parameters_json = QJsonDocument(parameters).toJson();
        QString par_fname = CacheManager::globalInstance()->makeLocalFile(S->id + ".par", CacheManager::ShortTerm);
        args << par_fname;
    } else if (S->prtype == ProcessType) {
        args << "run-process";
        args << S->processor_name;
        if (!S->output_file.isEmpty())
            args << "--~process_output=" + S->output_file;
        QStringList pkeys = S->parameters.keys();
        foreach(QString pkey, pkeys)
        {
            args << QString("--%1=%2").arg(pkey).arg(S->parameters[pkey].toString());
        }
    }
    QProcess* qprocess = new QProcess;
    qprocess->setProperty("pript_id", pript_id.toLatin1().data());
    if (S->prtype == ScriptType) {
        printf("   Launching script %s: ", pript_id.toLatin1().data());
        foreach(QString fname, S->script_paths)
        {
            QString str = QFileInfo(fname).fileName();
            printf("%s ", str.toLatin1().data());
        }
        printf("\n");
    } else {
        printf("   Launching process %s %s: ", S->processor_name.toLatin1().data(), pript_id.toLatin1().data());
        QString cmd = args.join(" ");
        printf("%s\n", cmd.toLatin1().data());
    }

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_pript_qprocess_finished()));

    qprocess->start(exe, args);
    if (qprocess->waitForStarted()) {
        S->qprocess = qprocess;
        S->is_running = true;
        return true;

    } else {
        if (S->prtype == ScriptType) {
            qWarning() << "Unable to start script: " + S->id;
        } else {
            qWarning() << "Unable to start process: " + S->processor_name + " " + S->id;
        }
        m_pripts.remove(pript_id);
        return false;
    }
}

bool MPDaemonPrivate::handle_processes()
{
    if (num_running_pripts(ProcessType) < 1) {
        if (num_pending_pripts(ProcessType) > 0) {
            if (!launch_next_pript(ProcessType)) {
                qWarning() << "Unexpected problem. Failed to launch next process.";
                return false;
            }
        }
    }
    return true;
}

void MPDaemonPrivate::stop_orphan_processes_and_scripts()
{
    QStringList keys = m_pripts.keys();
    foreach(QString key, keys)
    {
        if (!m_pripts[key].is_finished) {
            if ((m_pripts[key].parent_pid)&&(!MPDaemon::pidExists(m_pripts[key].parent_pid))) {
                if (m_pripts[key].is_running) {
                    if (m_pripts[key].qprocess) {
                        if (!m_pripts[key].qprocess->property("terminating").toBool()) {
                            if (m_pripts[key].prtype == ScriptType) {
                                qWarning() << "Terminating script qprocess: " + key;
                            } else {
                                qWarning() << "Terminating process qprocess: " + key;
                            }
                            m_pripts[key].qprocess->terminate();
                            m_pripts[key].qprocess->setProperty("terminating", true);
                            //Now the process should end and we will remove it in the slot
                        }
                    }
                } else {
                    if (m_pripts[key].prtype == ScriptType) {
                        qWarning() << "Removing script: " + key;
                    } else {
                        qWarning() << "Removing process: " + key;
                    }
                    m_pripts.remove(key);
                }
            }
        }
    }
}

#include "signal.h"
bool MPDaemon::pidExists(qint64 pid)
{
    return (kill(pid, 0) == 0);
}

int MPDaemonPrivate::num_running_pripts(PriptType prtype)
{
    int ret = 0;
    QStringList keys = m_pripts.keys();
    foreach(QString key, keys)
    {
        if (m_pripts[key].is_running) {
            if (m_pripts[key].prtype == prtype)
                ret++;
        }
    }
    return ret;
}

int MPDaemonPrivate::num_pending_pripts(PriptType prtype)
{
    int ret = 0;
    QStringList keys = m_pripts.keys();
    foreach(QString key, keys)
    {
        if ((!m_pripts[key].is_running) && (!m_pripts[key].is_finished)) {
            if (m_pripts[key].prtype == prtype)
                ret++;
        }
    }
    return ret;
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

QJsonObject pript_struct_to_obj(MPDaemonPript S)
{
    QJsonObject ret;
    ret["is_finished"] = S.is_finished;
    ret["is_running"] = S.is_running;
    ret["parameters"] = variantmap_to_json_obj(S.parameters);
    ret["id"] = S.id;
    ret["output_file"] = S.output_file;
    ret["success"] = S.success;
    ret["error"] = S.error;
    ret["parent_pid"] = QString("%1").arg(S.parent_pid);
    if (S.prtype == ScriptType) {
        ret["prtype"] = "script";
        ret["script_paths"] = stringlist_to_json_array(S.script_paths);
    } else {
        ret["prtype"] = "process";
        ret["processor_name"] = S.processor_name;
    }
    return ret;
}

MPDaemonPript pript_obj_to_struct(QJsonObject obj)
{
    MPDaemonPript ret;
    ret.is_finished = obj.value("is_finished").toBool();
    ret.is_running = obj.value("is_running").toBool();
    ret.parameters = json_obj_to_variantmap(obj.value("parameters").toObject());
    ret.id = obj.value("id").toString();
    ret.output_file = obj.value("output_file").toString();
    ret.success = obj.value("success").toBool();
    ret.error = obj.value("error").toString();
    ret.parent_pid = obj.value("parent_pid").toString().toLongLong();
    if (obj.value("prtype").toString() == "script") {
        ret.prtype = ScriptType;
        ret.script_paths = json_array_to_stringlist(obj.value("script_paths").toArray());
    } else {
        ret.prtype = ProcessType;
        ret.processor_name = obj.value("processor_name").toString();
    }
    return ret;
}
