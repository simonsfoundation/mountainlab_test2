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
    int num_running_pripts(PriptType prtype);
    int num_pending_pripts(PriptType prtype);
    bool launch_next_pript(PriptType prtype);
    bool launch_pript(PriptType prtype, QString id);

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
    bool launch_script(QString id)
    {
        return launch_pript(ScriptType, id);
    }
    bool launch_process(QString id)
    {
        return launch_pript(ProcessType, id);
    }

    void stop_qprocesses_whose_parents_are_gone();
    bool pid_is_gone(qint64 pid);
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
        d->stop_qprocesses_whose_parents_are_gone();
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

void MPDaemon::slot_pript_qprocess_finished()
{
    QProcess* P = qobject_cast<QProcess*>(sender());
    if (!P)
        return;
    QString pript_id = P->property("pript_id").toString();
    MPDaemonPript* S = 0;
    bool is_script;
    QString processor_name;
    if (d->m_scripts.contains(pript_id)) {
        S = &d->m_scripts[pript_id];
        is_script = true;
    } else if (d->m_processes.contains(pript_id)) {
        S = &d->m_processes[pript_id];
        is_script = false;
        processor_name = ((MPDaemonProcess*)S)->processor_name;
    }
    if (!S) {
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
    if (is_script) {
        printf("  Script %s finished ", pript_id.toLatin1().data());
    } else {
        printf("  Process %s %s finished ", processor_name.toLatin1().data(), pript_id.toLatin1().data());
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
        if (m_scripts.contains(S.id)) {
            qWarning() << "Unable to queue script. Script with this id already exists: " + S.id;
            return;
        }
        printf("QUEUING SCRIPT %s\n", S.id.toLatin1().data());
        m_scripts[S.id] = S;
    } else if (command == "queue-process") {
        MPDaemonProcess P = process_obj_to_struct(obj);
        if (m_processes.contains(P.id)) {
            qWarning() << "Unable to queue process. Process with this id already exists: " + P.id;
            return;
        }
        printf("QUEUING PROCESS %s %s\n", P.processor_name.toLatin1().data(), P.id.toLatin1().data());
        m_processes[P.id] = P;
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
    if (prtype == ScriptType) {
        QStringList keys = m_scripts.keys();
        foreach(QString key, keys)
        {
            if ((!m_scripts[key].is_running) && (!m_scripts[key].is_finished)) {
                return launch_script(key);
            }
        }
    } else {
        QStringList keys = m_processes.keys();
        foreach(QString key, keys)
        {
            if ((!m_processes[key].is_running) && (!m_processes[key].is_finished)) {
                return launch_process(key);
            }
        }
    }
    return false;
}

bool MPDaemonPrivate::launch_pript(PriptType prtype, QString pript_id)
{
    MPDaemonPript* S = 0;
    QString processor_name;
    QStringList script_paths;
    if (prtype == ScriptType) {
        if (!m_scripts.contains(pript_id))
            return false;
        S = &m_scripts[pript_id];
        script_paths = ((MPDaemonScript*)S)->script_paths;
    } else {
        if (!m_processes.contains(pript_id))
            return false;
        S = &m_processes[pript_id];
        processor_name = ((MPDaemonProcess*)S)->processor_name;
    }

    if (S->is_running)
        return false;
    if (S->is_finished)
        return false;

    QString exe = qApp->applicationFilePath();
    QStringList args;

    if (prtype == ScriptType) {
        args << "run-script";
        if (!S->output_file.isEmpty()) {
            args << "--~script_output=" + S->output_file;
        }
        foreach(QString fname, script_paths)
        {
            args << fname;
        }
        QJsonObject parameters = variantmap_to_json_obj(S->parameters);
        QString parameters_json = QJsonDocument(parameters).toJson();
        QString par_fname = CacheManager::globalInstance()->makeLocalFile(S->id + ".par", CacheManager::ShortTerm);
        args << par_fname;
    } else {
        args << "run-process";
        args << processor_name;
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
    if (prtype == ScriptType) {
        printf("   Launching script %s: ", pript_id.toLatin1().data());
        foreach(QString fname, script_paths)
        {
            QString str = QFileInfo(fname).fileName();
            printf("%s ", str.toLatin1().data());
        }
        printf("\n");
    } else {
        printf("   Launching process %s %s: ", processor_name.toLatin1().data(), pript_id.toLatin1().data());
        QString cmd = args.join(" ");
        printf("%s\n", cmd.toLatin1().data());
    }

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_pript_qprocess_finished()));

    qprocess->start(exe, args);
    if (qprocess->waitForStarted()) {
        if (prtype == ScriptType) {
            m_script_qprocesses[pript_id] = qprocess;
        } else {
            m_process_qprocesses[pript_id] = qprocess;
        }
        S->is_running = true;
        return true;

    } else {
        if (prtype == ScriptType) {
            qWarning() << "Unable to start script: " + S->id;
            m_scripts.remove(pript_id);
        } else {
            qWarning() << "Unable to start process: " + processor_name + " " + S->id;
            m_processes.remove(pript_id);
        }
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

void MPDaemonPrivate::stop_qprocesses_whose_parents_are_gone()
{
    {
        QStringList keys=m_scripts.keys();
        foreach (QString key,keys) {
            if (!m_scripts[key].is_finished) {
                if (pid_is_gone(m_scripts[key].parent_pid)) {
                    if (m_scripts[key].is_running) {
                        qWarning() << "Terminating script qprocess: "+key;
                        if (m_script_qprocesses[key]) {
                            m_script_qprocesses[key]->kill();
                        }
                    }
                    else {
                        qWarning() << "Removing script: "+key;
                        m_scripts.remove(key);
                    }

                }
            }
        }
    }
    {
        QStringList keys=m_processes.keys();
        foreach (QString key,keys) {
            if (!m_processes[key].is_finished) {
                if (pid_is_gone(m_processes[key].parent_pid)) {
                    if (m_processes[key].is_running) {
                        qWarning() << "Terminating process qprocess: "+key;
                        if (m_process_qprocesses[key]) {
                            m_process_qprocesses[key]->kill();
                        }
                    }
                    else {
                        qWarning() << "Removing process: "+key;
                        m_processes.remove(key);
                    }

                }
            }
        }
    }
}

#include "signal.h"
bool MPDaemonPrivate::pid_is_gone(qint64 pid)
{
    if (pid==0) return false;
    if (kill(pid, 0)!=0) return true;
    return false;
}

int MPDaemonPrivate::num_running_pripts(PriptType prtype)
{
    int ret = 0;
    if (prtype == ScriptType) {
        QStringList keys = m_scripts.keys();
        foreach(QString key, keys)
        {
            if (m_scripts[key].is_running)
                ret++;
        }
    } else {
        QStringList keys = m_processes.keys();
        foreach(QString key, keys)
        {
            if (m_processes[key].is_running)
                ret++;
        }
    }
    return ret;
}

int MPDaemonPrivate::num_pending_pripts(PriptType prtype)
{
    int ret = 0;
    if (prtype == ScriptType) {
        QStringList keys = m_scripts.keys();
        foreach(QString key, keys)
        {
            if ((!m_scripts[key].is_running) && (!m_scripts[key].is_finished))
                ret++;
        }
    } else {
        QStringList keys = m_processes.keys();
        foreach(QString key, keys)
        {
            if ((!m_processes[key].is_running) && (!m_processes[key].is_finished))
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
    return ret;
}

QJsonObject script_struct_to_obj(MPDaemonScript S)
{
    QJsonObject ret = pript_struct_to_obj(S);
    ret["script_paths"] = stringlist_to_json_array(S.script_paths);
    return ret;
}

void pript_obj_to_struct(MPDaemonPript& ret, QJsonObject obj)
{
    ret.is_finished = obj.value("is_finished").toBool();
    ret.is_running = obj.value("is_running").toBool();
    ret.parameters = json_obj_to_variantmap(obj.value("parameters").toObject());
    ret.id = obj.value("id").toString();
    ret.output_file = obj.value("output_file").toString();
    ret.success = obj.value("success").toBool();
    ret.error = obj.value("error").toString();
    ret.parent_pid = obj.value("parent_pid").toString().toLongLong();
}

MPDaemonScript script_obj_to_struct(QJsonObject obj)
{
    MPDaemonScript ret;
    pript_obj_to_struct(ret, obj);
    ret.script_paths = json_array_to_stringlist(obj.value("script_paths").toArray());
    return ret;
}

QJsonObject process_struct_to_obj(MPDaemonProcess P)
{
    QJsonObject ret = pript_struct_to_obj(P);
    ret["processor_name"] = P.processor_name;
    return ret;
}

MPDaemonProcess process_obj_to_struct(QJsonObject obj)
{
    MPDaemonProcess ret;
    pript_obj_to_struct(ret, obj);
    ret.processor_name = obj.value("processor_name").toString();
    return ret;
}
