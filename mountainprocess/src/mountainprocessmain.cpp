/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#include "mpdaemon.h"
#include "mpdaemoninterface.h"
#include "scriptcontroller.h"

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QSettings>
#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include "processmanager.h"

#include "cachemanager.h"
#include "tempfilecleaner.h"
#include "mlcommon.h"

/// TODO security in scripts that are able to be submitted
/// TODO title on mountainview from mountainbrowser
/// TODO on startup of mountainprocess daemon show all the loaded processors
/// TODO remove all references to datalaboratory.org and magland.org in the repository (don't just search .h/.cpp files)
/// TODO rigorously check mpserver for potential crashes, unhandled exceptions
/// TODO make MAX_SHORT_TERM_GB and MAX_LONG_TERM_GB configurable (but default should be zero - so we don't interrupt every run -- so like the daemon should be handling it)

#define MAX_SHORT_TERM_GB 100
#define MAX_LONG_TERM_GB 100
#define MAX_MDACHUNK_GB 20

struct run_script_opts;
void print_usage();
bool load_parameter_file(QVariantMap& params, const QString& fname);
bool run_script(const QStringList& script_fnames, const QVariantMap& params, const run_script_opts& opts, QString& error_message);
bool initialize_process_manager(QString config_fname, QJsonObject config);
void remove_system_parameters(QVariantMap& params);
bool queue_pript(PriptType prtype, const CLParams& CLP);

#define EXIT_ON_CRITICAL_ERROR
void mountainprocessMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

struct run_script_opts {
    run_script_opts()
    {
        nodaemon = false;
    }

    bool nodaemon;
    QStringList server_urls;
    QString server_base_path;
};

QJsonArray monitor_stats_to_json_array(const QList<MonitorStats>& stats);
long compute_peak_mem_bytes(const QList<MonitorStats>& stats);
double compute_peak_cpu_pct(const QList<MonitorStats>& stats);

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    CLParams CLP(argc, argv);
    QString arg1 = CLP.unnamed_parameters.value(0);
    QString arg2 = CLP.unnamed_parameters.value(1);

    /// TODO get rid of mlConfigPath()
    QString config_fname = MLUtil::mountainlabBasePath() + "/labcomputer/labcomputer.json";
    QString config_path = QFileInfo(config_fname).path();
    QJsonParseError parse_error;
    QJsonObject config = QJsonDocument::fromJson(TextFile::read(config_fname).toLatin1(), &parse_error).object();
    if (parse_error.error != QJsonParseError::NoError) {
        //qWarning() << "Unable to parse confuration file (but maybe not a problem on local machine -- trying example file): " + config_fname;
        qWarning() << "trying labcomputer.json.example";
        config = QJsonDocument::fromJson(TextFile::read(config_fname + ".example").toLatin1(), &parse_error).object();
        if (parse_error.error != QJsonParseError::NoError) {
            qWarning() << "Unable to parse confuration file: " + config_fname + ".example";
            return -1;
        }
    }
    QString log_path = MLUtil::mlLogPath() + "/mountainprocess";
    QString tmp_path = MLUtil::tempPath();
    if (config.contains("temporary_path")) {
        tmp_path = config["temporary_path"].toString();
    }
    CacheManager::globalInstance()->setLocalBasePath(tmp_path);

    if (arg1.endsWith(".prv")) {
        QString path0 = resolve_prv_file(arg1);
        printf("FILE: %s\n", path0.toLatin1().data());
        return 0;
    }

    qInstallMessageHandler(mountainprocessMessageOutput);

    /// TODO don't need to always load the process manager?

    ProcessManager* PM = ProcessManager::globalInstance();
    QStringList server_urls = json_array_to_stringlist(config["server_urls"].toArray());
    PM->setServerUrls(server_urls);
    QString server_base_path = MLUtil::resolvePath(config_path, config["mdaserver_base_path"].toString());
    PM->setServerBasePath(server_base_path);

    setbuf(stdout, NULL);

    if (arg1 == "list-processors") { //Provide a human-readable list of the available processors
        if (!initialize_process_manager(config_fname, config))
            return -1; //load the processor plugins etc
        QStringList pnames = PM->processorNames();
        qSort(pnames);
        foreach (QString pname, pnames) {
            printf("%s\n", pname.toLatin1().data());
        }
        return 0;
    }
    else if (arg1 == "run-process") { //Run a process synchronously
        if (!initialize_process_manager(config_fname, config))
            return -1; //load the processor plugins etc
        QString output_fname = CLP.named_parameters.value("~process_output").toString(); //maybe the user specified where output is to be reported
        QString processor_name = arg2; //name of the processor is the second user-supplied arg
        QVariantMap process_parameters = CLP.named_parameters;
        remove_system_parameters(process_parameters); //remove parameters starting with "~"
        int ret = 0;
        QString error_message;
        MLProcessInfo info;

        if (PM->processAlreadyCompleted(processor_name, process_parameters)) { //do we have a record of this procesor already completing? If so, we save a lot of time by not re-running
            printf("Process already completed: %s\n", processor_name.toLatin1().data());
        }
        else {
            QString id;

            if (!PM->checkParameters(processor_name, process_parameters)) { //check to see that all our parameters are in order for the given processor
                error_message = "Problem checking process: " + processor_name;
                ret = -1;
            }
            else {
                id = PM->startProcess(processor_name, process_parameters); //start the process and retrieve a unique id
                if (id.isEmpty()) {
                    error_message = "Problem starting process: " + processor_name;
                    ret = -1;
                }
                if (!PM->waitForFinished(id)) { //wait for the process to finish
                    error_message = "Problem waiting for process to finish: " + processor_name;
                    ret = -1;
                }
                info = PM->processInfo(id); //get the info about the process from the process manager
                if (info.exit_status == QProcess::CrashExit) {
                    error_message = "Process crashed: " + processor_name;
                    ret = -1;
                }
                if (info.exit_code != 0) {
                    error_message = "Exit code is non-zero: " + processor_name;
                    ret = -1;
                }
                PM->clearProcess(id); //clean up
            }

            printf("---------------------------------------------------------------\n");
            printf("PROCESS COMPLETED: %s\n", info.processor_name.toLatin1().data());
            if (!error_message.isEmpty())
                printf("ERROR: %s\n", error_message.toLatin1().data());
            long mb = compute_peak_mem_bytes(info.monitor_stats) / 1000000;
            double cpu = compute_peak_cpu_pct(info.monitor_stats);
            if (cpu) {
                printf("Peak usage: %ld MB RAM / %g%% CPU\n", mb, cpu);
            }
            printf("---------------------------------------------------------------\n");
        }
        QJsonObject obj; //the output info
        obj["exe_command"] = info.exe_command;
        obj["exit_code"] = info.exit_code;
        if (info.exit_status == QProcess::CrashExit)
            obj["exit_status"] = "CrashExit";
        else
            obj["exit_status"] = "NormalExit";
        obj["finished"] = QJsonValue(info.finished);
        obj["parameters"] = variantmap_to_json_obj(info.parameters);
        obj["processor_name"] = info.processor_name;
        obj["standard_output"] = QString(info.standard_output);
        obj["standard_error"] = QString(info.standard_error);
        obj["success"] = error_message.isEmpty();
        obj["error"] = error_message;
        obj["peak_mem_bytes"] = (long long)compute_peak_mem_bytes(info.monitor_stats);
        obj["peak_cpu_pct"] = compute_peak_cpu_pct(info.monitor_stats);
        //obj["monitor_stats"]=monitor_stats_to_json_array(info.monitor_stats); -- at some point we can include this in the file. For now we only worry about the computed peak values
        if (!output_fname.isEmpty()) { //The user wants the results to go in this file
            QString obj_json = QJsonDocument(obj).toJson();
            if (!TextFile::write(output_fname, obj_json)) {
                qCritical() << "Unable to write results to: " + output_fname;
            }
        }
        if (!error_message.isEmpty()) {
            qCritical() << "Error in mountainprocessmain" << error_message;
        }
        printf("\n");

        return ret; //returns exit code 0 if okay
    }
    else if (arg1 == "run-script") { //run a script synchronously (although note that individual processes will be queued (unless --_nodaemon is specified), but the script will wait for them to complete)
        if (!initialize_process_manager(config_fname, config))
            return -1;

        QString output_fname = CLP.named_parameters.value("~script_output").toString(); //maybe the user specified where output is to be reported

        int ret = 0;
        QString error_message;

        QStringList script_fnames; //names of the javascript source files
        QVariantMap params; //parameters to be passed into the main() function of the javascript
        for (int i = 0; i < CLP.unnamed_parameters.count(); i++) {
            QString str = CLP.unnamed_parameters[i];
            if (str.endsWith(".js")) { //must be a javascript source file
                if (!QFile::exists(str)) {
                    QString str2 = MLUtil::mountainlabBasePath() + "/mountainprocess/scripts/" + str;
                    if (QFile::exists(str2)) {
                        str = str2;
                    }
                }
                if (QFile::exists(str)) {
                    script_fnames << str;
                }
                else {
                    qWarning() << "Unable to find script file: " + str;
                    return -1;
                }
            }
            if ((str.endsWith(".par")) || (str.endsWith(".json"))) { // note that we can have multiple parameter files! the later ones override the earlier ones.
                qDebug() << "Loading parameter file: " + str;
                if (!load_parameter_file(params, str)) {
                    qWarning() << "Unable to load parameter file: " + str;
                    return -1;
                }
            }
        }
        QStringList keys0 = CLP.named_parameters.keys();
        foreach (QString key0, keys0) {
            params[key0] = CLP.named_parameters[key0];
        }
        remove_system_parameters(params);

        run_script_opts opts;
        opts.nodaemon = CLP.named_parameters.contains("_nodaemon");
        opts.server_urls = server_urls;
        opts.server_base_path = server_base_path;
        if (!run_script(script_fnames, params, opts, error_message)) { //actually run the script
            ret = -1;
        }

        QJsonObject obj; //the output info
        obj["script_fnames"] = stringlist_to_json_array(script_fnames);
        obj["parameters"] = variantmap_to_json_obj(params);
        obj["success"] = (ret == 0);
        obj["error"] = error_message;
        if (!output_fname.isEmpty()) { //The user wants the results to go in this file
            QString obj_json = QJsonDocument(obj).toJson();
            if (!TextFile::write(output_fname, obj_json)) {
                qCritical() << "Unable to write results to: " + output_fname;
            }
        }

        return ret;
    }
    else if (arg1 == "daemon-start") {
        if (!initialize_process_manager(config_fname, config))
            return -1;
        TempFileCleaner cleaner;
        cleaner.addPath(MLUtil::tempPath() + "/tmp_short_term", MAX_SHORT_TERM_GB);
        cleaner.addPath(MLUtil::tempPath() + "/tmp_long_term", MAX_LONG_TERM_GB);
        cleaner.addPath(MLUtil::resolvePath(config_path, config["mdaserver_base_path"].toString()) + "/tmp_short_term", MAX_SHORT_TERM_GB);
        cleaner.addPath(MLUtil::resolvePath(config_path, config["mdaserver_base_path"].toString()) + "/tmp_long_term", MAX_LONG_TERM_GB);
        cleaner.addPath(MLUtil::resolvePath(config_path, config["mdachunk_data_path"].toString() + "/tmp_short_term"), MAX_MDACHUNK_GB);
        cleaner.addPath(MLUtil::resolvePath(config_path, config["mdachunk_data_path"].toString() + "/tmp_long_term"), MAX_MDACHUNK_GB);
        MPDaemon X;
        X.setLogPath(log_path);
        ProcessResources RR;
        RR.num_threads = qMax(1.0, config["num_threads"].toDouble());
        RR.memory_gb = qMax(1.0, config["memory_gb"].toDouble());
        X.setTotalResourcesAvailable(RR);
        if (!X.run())
            return -1;
        return 0;
    }
    /*
    else if (arg1 == "-internal-daemon-start") { //This is called internaly to start the daemon (which is the central program running in the background)
        MPDaemon X;
        if (!X.run())
            return -1;
        return 0;
    } else if (arg1 == "daemon-start") { //Start the daemon
        MPDaemonInterface X;
        if (X.start()) {
            printf("Started daemon.\n");
            return 0;
        } else {
            printf("Failed to start daemon.\n");
            return -1;
        }
    } else if (arg1 == "daemon-stop") { //Stop the daemon
        MPDaemonInterface X;
        if (X.stop())
            return 0;
        else
            return -1;
    } else if (arg1 == "daemon-restart") { //Restart the daemon
        MPDaemonInterface X;
        if (!X.stop())
            return -1;
        if (!X.start())
            return -1;
        printf("Daemon has been restarted.\n");
        return 0;
    }
    */
    else if (arg1 == "daemon-state") { //Print some information on the state of the daemon
        MPDaemonInterface X;
        QJsonObject state = X.getDaemonState();
        QString json = QJsonDocument(state).toJson();
        printf("%s", json.toLatin1().data());
        return 0;
    }
    else if (arg1 == "clear-processing") {
        MPDaemonInterface X;
        X.clearProcessing();
        return 0;
    }
    else if (arg1 == "queue-script") { //Queue a script -- to be executed when resources are available
        if (queue_pript(ScriptType, CLP))
            return 0;
        else
            return -1;
    }
    else if (arg1 == "queue-process") {
        if (queue_pript(ProcessType, CLP))
            return 0;
        else
            return -1;
    }
    else if (arg1 == "get-script") {
        if (!log_path.isEmpty()) {
            QString str = TextFile::read(log_path + "/scripts/" + CLP.named_parameters["id"].toString() + ".json");
            printf("%s", str.toLatin1().data());
        }
    }
    else if (arg1 == "get-process") {
        if (!log_path.isEmpty()) {
            QString str = TextFile::read(log_path + "/processes/" + CLP.named_parameters["id"].toString() + ".json");
            printf("%s", str.toLatin1().data());
        }
    }
    else {
        print_usage(); //print usage information
        return -1;
    }

    return 0;
}

bool initialize_process_manager(QString config_fname, QJsonObject config)
{
    /*
     * Load the processor paths
     */
    QStringList processor_paths = json_array_to_stringlist(config["processor_paths"].toArray());
    if (processor_paths.isEmpty()) {
        qCritical() << "No processor paths found in " + config_fname;
        return false;
    }
    for (int i = 0; i < processor_paths.count(); i++) {
        QString path0 = processor_paths[i];
        path0 = MLUtil::resolvePath(QFileInfo(config_fname).path(), path0);
        processor_paths[i] = path0;
    }

    /*
     * Load the processors
     */
    ProcessManager* PM = ProcessManager::globalInstance();
    foreach (QString processor_path, processor_paths) {
        QString p0 = processor_path;
        if (QFileInfo(p0).isRelative()) {
            p0 = QFileInfo(config_fname).path() + "/" + p0;
        }
        //printf("Searching for processors in %s\n", p0.toLatin1().data());
        PM->loadProcessors(p0);
        int num_processors = PM->processorNames().count();
        printf("Loaded %d processors in %s\n", num_processors, p0.toLatin1().data());
    }

    return true;
}

QString remove_comments_in_line(QString line)
{
    int ind = line.indexOf("//");
    if (ind >= 0) {
        return line.mid(0, ind);
    }
    else {
        return line;
    }
}

QString remove_comments(QString json)
{
    QStringList lines = json.split("\n");
    for (int i = 0; i < lines.count(); i++) {
        lines[i] = remove_comments_in_line(lines[i]);
    }
    return lines.join("\n");
}

bool load_parameter_file(QVariantMap& params, const QString& fname)
{
    QString json = TextFile::read(fname);
    if (json.isEmpty()) {
        qCritical() << "Non-existent or empty parameter file: " + fname;
        return false;
    }
    json = remove_comments(json);
    QJsonParseError error;
    /// Witold I use toLatin1() everywhere. Is this the appropriate way to convert to byte array?
    QJsonObject obj = QJsonDocument::fromJson(json.toLatin1(), &error).object();
    if (error.error != QJsonParseError::NoError) {
        qCritical() << "Error parsing json file: " + fname + " : " + error.errorString();
        return false;
    }
    QStringList keys = obj.keys();
    foreach (QString key, keys) {
        params[key] = obj[key].toVariant();
    }
    return true;
}

void display_error(QJSValue result)
{
    /// Witold there must be a better way to print the QJSValue error message out to the console.
    /// Witold In general is it possible to not display quotes around strings for qDebug?
    qDebug() << result.property("name").toString(); //okay
    qDebug() << result.property("message").toString(); //okay
    qDebug() << QString("%1 line %2").arg(result.property("fileName").toString()).arg(result.property("lineNumber").toInt()); //okay
}

bool run_script(const QStringList& script_fnames, const QVariantMap& params, const run_script_opts& opts, QString& error_message)
{
    QJsonObject parameters = variantmap_to_json_obj(params);

    QJSEngine engine;
    ScriptController Controller;
    Controller.setNoDaemon(opts.nodaemon);
    Controller.setServerUrls(opts.server_urls);
    Controller.setServerBasePath(opts.server_base_path);
    QJSValue MP = engine.newQObject(&Controller);
    engine.globalObject().setProperty("MP", MP);
    foreach (QString fname, script_fnames) {
        QJSValue result = engine.evaluate(TextFile::read(fname), fname);
        if (result.isError()) {
            display_error(result);
            error_message = "Error running script";
            qCritical() << "Error running script.";
            return false;
        }
    }
    {
        QString parameters_json = QJsonDocument(parameters).toJson(QJsonDocument::Compact);
        //Think about passing the parameters object directly in to the main() rather than converting to json string
        QString code = QString("main(JSON.parse('%1'));").arg(parameters_json);
        QJSValue result = engine.evaluate(code);
        if (result.isError()) {
            display_error(result);
            error_message = "Error running script: " + QString("%1 line %2: %3").arg(result.property("fileName").toString()).arg(result.property("lineNumber").toInt()).arg(result.property("message").toString());
            qCritical() << error_message;
            return false;
        }
    }

    return true;
}

void print_usage()
{
    printf("Usage:\n");
    printf("mountainprocess run-process [processor_name] --[param1]=[val1] --[param2]=[val2] ...\n");
    printf("mountainprocess run-script [script1].js [script2.js] ... [file1].par [file2].par ... \n");
    printf("mountainprocess daemon-start\n");
    printf("mountainprocess daemon-stop\n");
    printf("mountainprocess daemon-restart\n");
    printf("mountainprocess daemon-state\n");
    printf("mountainprocess queue-script --~script_output=[optional_output_fname] [script1].js [script2.js] ... [file1].par [file2].par ... \n");
    printf("mountainprocess queue-process [processor_name] --~process_output=[optional_output_fname] --[param1]=[val1] --[param2]=[val2] ...\n");
}

void remove_system_parameters(QVariantMap& params)
{
    QStringList keys = params.keys();
    foreach (QString key, keys) {
        if (key.startsWith("~")) {
            params.remove(key);
        }
    }
}

bool queue_pript(PriptType prtype, const CLParams& CLP)
{
    MPDaemonPript PP;

    bool detach = CLP.named_parameters.value("~detach", false).toBool();

    if (prtype == ScriptType) {
        QVariantMap params;
        for (int i = 0; i < CLP.unnamed_parameters.count(); i++) {
            QString str = CLP.unnamed_parameters[i];
            if (str.endsWith(".js")) {
                PP.script_paths << str;
                PP.script_path_checksums << MLUtil::computeSha1SumOfFile(str);
            }
            if ((str.endsWith(".par")) || (str.endsWith(".json"))) { // note that we can have multiple parameter files! the later ones override the earlier ones.
                if (!load_parameter_file(params, str)) {
                    qWarning() << "Error loading parameter file" << str;
                    return false;
                }
            }
        }
        QStringList pkeys = CLP.named_parameters.keys();
        foreach (QString pkey, pkeys) {
            params[pkey] = CLP.named_parameters[pkey];
        }
        remove_system_parameters(params);
        PP.parameters = params;
        PP.prtype = ScriptType;
    }
    else {
        PP.parameters = CLP.named_parameters;
        PP.prtype = ProcessType;
        remove_system_parameters(PP.parameters);
        PP.processor_name = CLP.unnamed_parameters.value(1); //arg2
    }

    PP.id = make_random_id();

    if (prtype == ScriptType) {
        PP.output_fname = CLP.named_parameters["~script_output"].toString();
    }
    else {
        PP.output_fname = CLP.named_parameters["~process_output"].toString();
    }
    if (PP.output_fname.isEmpty()) {
        if (prtype == ScriptType)
            PP.output_fname = CacheManager::globalInstance()->makeLocalFile("script_output." + PP.id + ".json", CacheManager::ShortTerm);
        else
            PP.output_fname = CacheManager::globalInstance()->makeLocalFile("process_output." + PP.id + ".json", CacheManager::ShortTerm);
    }
    if (prtype == ScriptType)
        PP.stdout_fname = CacheManager::globalInstance()->makeLocalFile("script_stdout." + PP.id + ".txt", CacheManager::ShortTerm);
    else
        PP.stdout_fname = CacheManager::globalInstance()->makeLocalFile("process_stdout." + PP.id + ".txt", CacheManager::ShortTerm);

    if (!detach) {
        PP.parent_pid = QCoreApplication::applicationPid();
    }

    MPDaemonInterface X;
    if (prtype == ScriptType) {
        if (!X.queueScript(PP)) { //queue the script
            qWarning() << "Error queueing script";
            return false;
        }
    }
    else {
        if (!X.queueProcess(PP)) { //queue the process
            qWarning() << "Error queueing process";
            return false;
        }
    }
    if (!detach) {
        qint64 parent_pid = CLP.named_parameters.value("~parent_pid", 0).toLongLong();
        MPDaemon::waitForFileToAppear(PP.output_fname, -1, false, parent_pid, PP.stdout_fname);
        QJsonParseError error;
        QJsonObject results_obj = QJsonDocument::fromJson(TextFile::read(PP.output_fname).toLatin1(), &error).object();
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Error in queue_pript in parsing output json file.";
            return false;
        }
        bool success = results_obj["success"].toBool();
        if (!success) {
            if (prtype == ScriptType) {
                qWarning() << "Error in script " + PP.id + ": " + results_obj["error"].toString();
            }
            else {
                qWarning() << "Error in process " + PP.processor_name + " " + PP.id + ": " + results_obj["error"].toString();
            }
            return false;
        }
    }
    return true;
}

void mountainprocessMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        //write to error log here
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
#ifdef EXIT_ON_CRITICAL_ERROR
        exit(-1);
#endif
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        exit(-1);
    }
}

QJsonArray monitor_stats_to_json_array(const QList<MonitorStats>& stats)
{
    QJsonArray ret;
    for (int i = 0; i < stats.count(); i++) {
        MonitorStats X = stats[i];
        QJsonObject obj;
        obj["timestamp"] = X.timestamp.toMSecsSinceEpoch();
        obj["mem_bytes"] = (long long)X.mem_bytes;
        obj["cpu_pct"] = X.cpu_pct;
        ret << obj;
    }
    return ret;
}
long compute_peak_mem_bytes(const QList<MonitorStats>& stats)
{
    long ret = 0;
    for (int i = 0; i < stats.count(); i++) {
        ret = qMax(ret, stats[i].mem_bytes);
    }
    return ret;
}

double compute_peak_cpu_pct(const QList<MonitorStats>& stats)
{
    double ret = 0;
    for (int i = 0; i < stats.count(); i++) {
        ret = qMax(ret, stats[i].cpu_pct);
    }
    return ret;
}
