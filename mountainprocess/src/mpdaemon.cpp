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
#include <sys/stat.h> //for mkfifo

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

void append_line_to_file(QString fname, QString line)
{
    QFile ff(fname);
    if (ff.open(QFile::Append | QFile::WriteOnly)) {
        ff.write(line.toLatin1());
        ff.write(QByteArray("\n"));
        ff.close();
    }
    else {
        static bool reported = false;
        if (!reported) {
            reported = true;
            qWarning() << "Unable to write to log file" << fname;
        }
    }
}

void debug_log(const char* function, const char* file, int line)
{
    QString fname = CacheManager::globalInstance()->localTempPath() + "/mpdaemon_debug.log";
    QString line0 = QString("%1: %2 %3:%4").arg(QDateTime::currentDateTime().toString("yy-MM-dd:hh:mm:ss.zzz")).arg(function).arg(file).arg(line);
    line0 += " ARGS: ";
    foreach (QString arg, qApp->arguments()) {
        line0 += arg + " ";
    }
    append_line_to_file(fname, line0);
}

MPDaemon::MPDaemon()
{
    debug_log(__FUNCTION__, __FILE__, __LINE__);

    d = new MPDaemonPrivate;
    d->q = this;
    d->m_is_running = false;

    connect(&d->m_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slot_commands_directory_changed()));
}

MPDaemon::~MPDaemon()
{
    debug_log(__FUNCTION__, __FILE__, __LINE__);

    foreach (MPDaemonPript P, d->m_pripts) {
        if (P.qprocess) {
            if (P.qprocess->state() == QProcess::Running)
                P.qprocess->terminate();
        }
    }

    delete d;
}

bool MPDaemon::run()
{
    debug_log(__FUNCTION__, __FILE__, __LINE__);

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
    QTime timer4;
    timer4.start();
    while (d->m_is_running) {
        if (timer.elapsed() > 5000) {
            d->write_info();
            timer.restart();
            printf(".");
        }
        if (timer4.elapsed() > 5 * 60000) {
            printf("\n");
        }
        QTime timer2;
        timer2.start();
        d->stop_orphan_processes_and_scripts();
        d->handle_scripts();
        d->handle_processes();
        if (timer2.elapsed() > 3000) {
            qWarning() << "This should not take this much time" << timer2.elapsed();
        }
        //
        QTime timer3;
        timer3.start();
        qApp->processEvents();
        if (timer3.elapsed() > 3000) {
            qWarning() << "Processing events should not take this much time" << timer2.elapsed();
        }
    }
    return true;
}

void mkdir_if_doesnt_exist(QString path, QFile::Permissions perm)
{
    /// Witold is there a better way to mkdir if not exists?
    if (!QDir(path).exists()) {
        QDir(QFileInfo(path).path()).mkdir(QFileInfo(path).fileName());
        QFile(path).setPermissions(perm);
    }
}

QString MPDaemon::daemonPath()
{
    QFile::Permissions perm = QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther | QFile::WriteOwner | QFile::WriteGroup | QFile::WriteOther | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther;
    QString ret = CacheManager::globalInstance()->localTempPath() + "/mpdaemon";
    mkdir_if_doesnt_exist(ret, perm);
    mkdir_if_doesnt_exist(ret + "/info", perm);
    mkdir_if_doesnt_exist(ret + "/commands", perm);
    mkdir_if_doesnt_exist(ret + "/completed_processes", perm);
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

bool MPDaemon::waitForFileToAppear(QString fname, qint64 timeout_ms, bool remove_on_appear, qint64 parent_pid, QString stdout_fname)
{
    debug_log(__FUNCTION__, __FILE__, __LINE__);

    QTime timer;
    timer.start();
    QFile stdout_file(stdout_fname);
    bool failed_to_open = false;

    /*
    QString i_am_alive_fname;
    if (parent_pid) {
        i_am_alive_fname=CacheManager::globalInstance()->makeLocalFile(QString("i_am_alive.%1.txt").arg(parent_pid));
    }
    QTime timer_i_am_alive; timer_i_am_alive.start();
    */

    while (1) {
        wait(200);
        /*
        if (timer_i_am_alive.elapsed()>1000) {
            if (parent_pid) {
                write_text_file(i_am_alive_fname,QString("Process is alive: %1").arg(parent_pid));
            }
            timer_i_am_alive.restart();
        }
        */

        bool terminate_file_exists = QFile::exists(fname); //do this before we check other things, like the stdout

        if ((timeout_ms >= 0) && (timer.elapsed() > timeout_ms))
            return false;
        if ((parent_pid) && (!MPDaemon::pidExists(parent_pid))) {
            qWarning() << "Exiting waitForFileToAppear because parent process is gone.";
            break;
        }
        if ((!stdout_fname.isEmpty()) && (!failed_to_open)) {
            if (QFile::exists(stdout_fname)) {
                if (!stdout_file.isOpen()) {
                    if (!stdout_file.open(QFile::ReadOnly)) {
                        qWarning() << "Unable to open stdout file for reading: " + stdout_fname;
                        failed_to_open = true;
                    }
                }
                if (stdout_file.isOpen()) {
                    QByteArray str = stdout_file.readAll();
                    if (!str.isEmpty()) {
                        printf("%s", str.data());
                    }
                }
            }
        }
        if (terminate_file_exists)
            break;
    }
    if (stdout_file.isOpen())
        stdout_file.close();
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
    foreach (QString fname, fnames) {
        QString path0 = path + "/" + fname;
        qint64 elapsed_sec = QFileInfo(path0).lastModified().secsTo(QDateTime::currentDateTime());
        if (elapsed_sec > 20) {
            qWarning() << "Removing old command file:" << path0;
            QFile::remove(path0);
        }
        else {
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
    debug_log(__FUNCTION__, __FILE__, __LINE__);

    QProcess* P = qobject_cast<QProcess*>(sender());
    if (!P)
        return;
    QString pript_id = P->property("pript_id").toString();
    MPDaemonPript* S;
    if (d->m_pripts.contains(pript_id)) {
        S = &d->m_pripts[pript_id];
    }
    else {
        qWarning() << "Unexpected problem in slot_pript_qprocess_finished. Unable to find script or process with id: " + pript_id;
        return;
    }
    S->is_finished = true;
    S->is_running = false;
    if (!S->output_fname.isEmpty()) {
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        QString run_time_results_json = read_text_file(S->output_fname);
        if (run_time_results_json.isEmpty()) {
            S->success = false;
            S->error = "Could not read results file: " + S->output_fname;
        }
        else {
            S->run_time_results = QJsonDocument::fromJson(run_time_results_json.toLatin1()).object();
            S->success = S->run_time_results["success"].toBool();
            S->error = S->run_time_results["error"].toString();
        }
    }
    else {
        S->success = true;
    }
    if (S->prtype == ScriptType) {
        printf("  Script %s finished ", pript_id.toLatin1().data());
    }
    else {
        printf("  Process %s %s finished ", S->processor_name.toLatin1().data(), pript_id.toLatin1().data());
    }
    if (S->success)
        printf("successfully\n");
    else
        printf("with error: %s\n", S->error.toLatin1().data());
    if (S->qprocess) {
        if (S->qprocess->state()==QProcess::Running) {
            if (S->qprocess->waitForFinished(1000)) {
                delete S->qprocess;
            }
            else {
                qWarning() << "Process did not finish after waiting even though we are in the slot for finished!!";
            }
        }
        S->qprocess = 0;
    }
    if (S->stdout_file) {
        if (S->stdout_file->isOpen()) {
            S->stdout_file->close();
        }
        delete S->stdout_file;
        S->stdout_file = 0;
    }
}

void MPDaemon::slot_qprocess_output()
{

    QProcess* P = qobject_cast<QProcess*>(sender());
    if (!P)
        return;
    QByteArray str = P->readAll();
    QString pript_id = P->property("pript_id").toString();
    if ((d->m_pripts.contains(pript_id)) && (d->m_pripts[pript_id].stdout_file)) {
        if (d->m_pripts[pript_id].stdout_file->isOpen()) {
            d->m_pripts[pript_id].stdout_file->write(str);
            d->m_pripts[pript_id].stdout_file->flush();
        }
    }
    else {
        printf("%s", str.data());
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

    {
        QJsonObject scripts;
        QJsonObject processes;
        QStringList keys = m_pripts.keys();
        foreach (QString key, keys) {
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
    QFile::setPermissions(fname, QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther | QFile::WriteOwner | QFile::WriteGroup | QFile::WriteOther);

    //finally, clean up
    QStringList list = QDir(MPDaemon::daemonPath() + "/info").entryList(QStringList("*.info"), QDir::Files, QDir::Name);
    foreach (QString fname, list) {
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
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        m_is_running = false;
        write_info();
    }
    else if (command == "queue-script") {
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        MPDaemonPript S = pript_obj_to_struct(obj);
        S.prtype = ScriptType;
        if (m_pripts.contains(S.id)) {
            qWarning() << "Unable to queue script. Process or script with this id already exists: " + S.id;
            return;
        }
        printf("QUEUING SCRIPT %s\n", S.id.toLatin1().data());
        m_pripts[S.id] = S;
    }
    else if (command == "queue-process") {
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        MPDaemonPript P = pript_obj_to_struct(obj);
        P.prtype = ProcessType;
        if (m_pripts.contains(P.id)) {
            qWarning() << "Unable to queue process. Process or script with this id already exists: " + P.id;
            return;
        }
        printf("QUEUING PROCESS %s %s\n", P.processor_name.toLatin1().data(), P.id.toLatin1().data());
        m_pripts[P.id] = P;
    }
    else {
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
    debug_log(__FUNCTION__, __FILE__, __LINE__);

    QStringList keys = m_pripts.keys();
    foreach (QString key, keys) {
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
    debug_log(__FUNCTION__, __FILE__, __LINE__);

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
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        args << "run-script";
        if (!S->output_fname.isEmpty()) {
            args << "--~script_output=" + S->output_fname;
        }
        foreach (QString fname, S->script_paths) {
            args << fname;
        }
        QJsonObject parameters = variantmap_to_json_obj(S->parameters);
        QString parameters_json = QJsonDocument(parameters).toJson();
        QString par_fname = CacheManager::globalInstance()->makeLocalFile(S->id + ".par", CacheManager::ShortTerm);
        write_text_file(par_fname, parameters_json);
        QFile::setPermissions(par_fname, QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther | QFile::WriteOwner | QFile::WriteGroup | QFile::WriteOther);
        args << par_fname;
    }
    else if (S->prtype == ProcessType) {
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        args << "run-process";
        args << S->processor_name;
        if (!S->output_fname.isEmpty())
            args << "--~process_output=" + S->output_fname;
        QStringList pkeys = S->parameters.keys();
        foreach (QString pkey, pkeys) {
            args << QString("--%1=%2").arg(pkey).arg(S->parameters[pkey].toString());
        }
    }
    debug_log(__FUNCTION__, __FILE__, __LINE__);
    QProcess* qprocess = new QProcess;
    qprocess->setProperty("pript_id", pript_id);
    qprocess->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(qprocess, SIGNAL(readyRead()), q, SLOT(slot_qprocess_output()));
    if (S->prtype == ScriptType) {
        printf("   Launching script %s: ", pript_id.toLatin1().data());
        foreach (QString fname, S->script_paths) {
            QString str = QFileInfo(fname).fileName();
            printf("%s ", str.toLatin1().data());
        }
        printf("\n");
    }
    else {
        printf("   Launching process %s %s: ", S->processor_name.toLatin1().data(), pript_id.toLatin1().data());
        QString cmd = args.join(" ");
        printf("%s\n", cmd.toLatin1().data());
    }

    QObject::connect(qprocess, SIGNAL(finished(int)), q, SLOT(slot_pript_qprocess_finished()));

    debug_log(__FUNCTION__, __FILE__, __LINE__);
    qprocess->start(exe, args);
    if (qprocess->waitForStarted()) {
        S->qprocess = qprocess;
        if (!S->stdout_fname.isEmpty()) {
            S->stdout_file = new QFile(S->stdout_fname);
            if (!S->stdout_file->open(QFile::WriteOnly)) {
                qWarning() << "Unable to open stdout file for writing: " + S->stdout_fname;
                delete S->stdout_file;
                S->stdout_file = 0;
            }
        }
        S->is_running = true;
        return true;
    }
    else {
        debug_log(__FUNCTION__, __FILE__, __LINE__);
        if (S->prtype == ScriptType) {
            qWarning() << "Unable to start script: " + S->id;
        }
        else {
            qWarning() << "Unable to start process: " + S->processor_name + " " + S->id;
        }
        qprocess->disconnect();
        delete qprocess;
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
    foreach (QString key, keys) {
        if (!m_pripts[key].is_finished) {
            if ((m_pripts[key].parent_pid) && (!MPDaemon::pidExists(m_pripts[key].parent_pid))) {
                debug_log(__FUNCTION__, __FILE__, __LINE__);
                if (m_pripts[key].qprocess) {
                    if (m_pripts[key].prtype == ScriptType) {
                        qWarning() << "Terminating script qprocess: " + key;
                    }
                    else {
                        qWarning() << "Terminating process qprocess: " + key;
                    }
                    m_pripts[key].qprocess->disconnect(); //so we don't go into the finished slot
                    m_pripts[key].qprocess->terminate();
                    delete m_pripts[key].qprocess;
                    m_pripts.remove(key);
                }
                else {
                    if (m_pripts[key].prtype == ScriptType) {
                        qWarning() << "Removing orphan script: " + key + " " + m_pripts[key].script_paths.value(0);
                    }
                    else {
                        qWarning() << "Removing orphan process: " + key + " " + m_pripts[key].processor_name;
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
    /// TODO is this the best way to see if process exists?
    return (kill(pid, 0) == 0);
    /*
    QString i_am_alive_fname=CacheManager::globalInstance()->makeLocalFile(QString("i_am_alive.%1.txt").arg(pid));
    return (QFileInfo(i_am_alive_fname).lastModified().secsTo(QDateTime::currentDateTime())<=5000);
    */
    //return QDir("/proc").exists(QString("%1").arg(pid));
}

bool MPDaemon::waitForFinishedAndWriteOutput(QProcess* P)
{
    debug_log(__FUNCTION__, __FILE__, __LINE__);

    P->waitForStarted();
    while (P->state() == QProcess::Running) {
        P->waitForReadyRead(100);
        QByteArray str = P->readAll();
        if (str.count() > 0) {
            printf("%s", str.data());
        }
        qApp->processEvents();
    }
    {
        P->waitForReadyRead();
        QByteArray str = P->readAll();
        if (str.count() > 0) {
            printf("%s", str.data());
        }
    }
    return (P->state() != QProcess::Running);
}

int MPDaemonPrivate::num_running_pripts(PriptType prtype)
{
    int ret = 0;
    QStringList keys = m_pripts.keys();
    foreach (QString key, keys) {
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
    foreach (QString key, keys) {
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
        ret[key] = QJsonValue::fromVariant(map[key]);
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

QJsonObject pript_struct_to_obj(MPDaemonPript S)
{
    QJsonObject ret;
    ret["is_finished"] = S.is_finished;
    ret["is_running"] = S.is_running;
    ret["parameters"] = variantmap_to_json_obj(S.parameters);
    ret["id"] = S.id;
    ret["output_fname"] = S.output_fname;
    ret["stdout_fname"] = S.stdout_fname;
    ret["success"] = S.success;
    ret["error"] = S.error;
    ret["parent_pid"] = QString("%1").arg(S.parent_pid);
    if (S.prtype == ScriptType) {
        ret["prtype"] = "script";
        ret["script_paths"] = stringlist_to_json_array(S.script_paths);
    }
    else {
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
    ret.output_fname = obj.value("output_fname").toString();
    ret.stdout_fname = obj.value("stdout_fname").toString();
    ret.success = obj.value("success").toBool();
    ret.error = obj.value("error").toString();
    ret.parent_pid = obj.value("parent_pid").toString().toLongLong();
    if (obj.value("prtype").toString() == "script") {
        ret.prtype = ScriptType;
        ret.script_paths = json_array_to_stringlist(obj.value("script_paths").toArray());
    }
    else {
        ret.prtype = ProcessType;
        ret.processor_name = obj.value("processor_name").toString();
    }
    return ret;
}
