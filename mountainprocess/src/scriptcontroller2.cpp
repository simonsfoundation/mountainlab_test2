/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#include "scriptcontroller.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "cachemanager.h"
#include "processmanager.h"
#include <QTime>
#include <QCoreApplication>
#include <QDebug>
#include <unistd.h> //for usleep
#include "mpdaemon.h"
#include "mlcommon.h"

struct PipelineNode {
    PipelineNode()
    {
        completed = false;
        running = false;
        qprocess = 0;
    }
    ~PipelineNode()
    {
        if (qprocess) {
            delete qprocess;
        }
    }
    QMap<QString, QDateTime> timestamps;
    QString processor_name;
    QVariantMap parameters;
    QVariantMap prvs;
    QStringList input_pnames;
    QStringList output_pnames;
    bool completed;
    bool running;
    QProcess* qprocess;

    QStringList input_paths()
    {
        QStringList ret;
        foreach (QString pname, input_pnames)
            ret << parameters[pname].toString();
        return ret;
    }
    QStringList output_paths()
    {
        QStringList ret;
        foreach (QString pname, output_pnames)
            ret << parameters[pname].toString();
        return ret;
    }
};

class ScriptControllerPrivate {
public:
    ScriptControllerPrivate()
    {
        m_nodaemon = false;
    }

    ScriptController* q;
    bool m_nodaemon = false;
    QStringList m_server_urls;
    QString m_server_base_path;
    bool m_force_run = false;

    QList<PipelineNode> m_pipeline_nodes;

    QProcess* queue_process(QString processor_name, const QVariantMap& parameters, bool use_run, bool force_run);
    QProcess* run_process(QString processor_name, const QVariantMap& parameters, bool force_run);
    //static bool queue_process_and_wait_for_finished(QString processor_name, const QVariantMap& parameters);
    static void wait(qint64 msec);

    QString resolve_file_name_p(QString fname);
};

ScriptController::ScriptController()
{
    d = new ScriptControllerPrivate;
    d->q = this;
}

ScriptController::~ScriptController()
{
    delete d;
}

void ScriptController::setNoDaemon(bool val)
{
    d->m_nodaemon = val;
}

void ScriptController::setServerUrls(const QStringList& urls)
{
    d->m_server_urls = urls;
}

void ScriptController::setServerBasePath(const QString& path)
{
    d->m_server_base_path = path;
}

void ScriptController::setForceRun(bool force_run)
{
    d->m_force_run = force_run;
}

QJsonObject ScriptController::getResults()
{
    return QJsonObject();
}

QString ScriptController::fileChecksum(const QString& fname_in)
{
    QString fname = d->resolve_file_name_p(fname_in);
    QTime timer;
    timer.start();
    QString ret = MLUtil::computeSha1SumOfFile(fname);
    return ret;

    /*
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);
    file.close();
    QString ret = QString(hash.result().toHex());
    printf("%s -- Elapsed: %g sec\n", ret.toLatin1().data(), timer.elapsed() * 1.0 / 1000);
    return ret;
    */
}

QString ScriptController::stringChecksum(const QString& str)
{
    QByteArray X = str.toLatin1();
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(X);
    return QString(hash.result().toHex());
}

QString ScriptController::createTemporaryFileName(const QString& code)
{
    return CacheManager::globalInstance()->makeLocalFile(code, CacheManager::LongTerm);
}

/*
bool ScriptController::runProcess(const QString& processor_name, const QString& parameters_json)
{
    QJsonObject params = QJsonDocument::fromJson(parameters_json.toLatin1()).object();
    QStringList keys = params.keys();
    QMap<QString, QVariant> parameters;
    foreach (QString key, keys) {
        parameters[key] = params[key].toVariant();
    }
    ProcessManager* PM = ProcessManager::globalInstance();
    if (!PM->checkParameters(processor_name, parameters)) {
        return false;
    }
    if (PM->processAlreadyCompleted(processor_name, parameters)) {
        this->log(QString("Process already completed: %1").arg(processor_name));
        return true;
    }

    if (d->queue_process_and_wait_for_finished(processor_name, parameters)) {
        return true;
    }
    else {
        return false;
    }
}
*/

QJsonObject make_prv_object(QString path)
{
    if (!QFile::exists(path)) {
        qWarning() << "Unable to find file (for prv):" << path;
        return QJsonObject();
    }
    QJsonObject obj;
    obj["original_path"] = path;
    obj["original_checksum"] = MLUtil::computeSha1SumOfFile(path);
    obj["original_size"] = QFileInfo(path).size();
    return obj;
}

QJsonArray get_prv_processes(const QList<PipelineNode>& nodes, const QMap<QString, int>& node_indices_for_outputs, QString path, QSet<int>& node_indices_already_used, bool* ok)
{
    QJsonArray processes;
    int ind0 = node_indices_for_outputs.value(path, -1);
    if ((ind0 >= 0) && (!node_indices_already_used.contains(ind0))) { //avoid infinite cycles, which can happen, for example, if an output file is the same as an input file
        const PipelineNode* node = &nodes[ind0];
        if (!node->processor_name.isEmpty()) {
            QJsonObject process;
            process["processor_name"] = node->processor_name;
            {
                QJsonObject inputs;
                foreach (QString pname, node->input_pnames) {
                    QJsonObject tmp;
                    tmp = make_prv_object(node->parameters[pname].toString());
                    if (tmp.isEmpty()) {
                        *ok = false;
                        return processes;
                    }
                    inputs[pname] = tmp;
                }
                process["inputs"] = inputs;
            }
            {
                QJsonObject outputs;
                foreach (QString pname, node->output_pnames) {
                    QJsonObject tmp;
                    tmp = make_prv_object(node->parameters[pname].toString());
                    if (tmp.isEmpty()) {
                        *ok = false;
                        return processes;
                    }
                    outputs[pname] = tmp;
                }
                process["outputs"] = outputs;
            }
            {
                QJsonObject parameters;
                QStringList pnames = node->parameters.keys();
                foreach (QString pname, pnames) {
                    if ((!node->input_pnames.contains(pname)) && (!node->output_pnames.contains(pname))) {
                        parameters[pname] = node->parameters[pname].toString();
                    }
                }
                process["parameters"] = parameters;
            }
            node_indices_already_used.insert(ind0);
            processes.append(process);
            foreach (QString pname, node->input_pnames) {
                QJsonArray X = get_prv_processes(nodes, node_indices_for_outputs, node->parameters[pname].toString(), node_indices_already_used, ok);
                if (!ok)
                    return processes;
                for (int a = 0; a < X.count(); a++) {
                    processes.append(X[a]);
                }
            }
        }
    }
    *ok = true;
    return processes;
}

bool write_prv_file(const QList<PipelineNode>& nodes, const QMap<QString, int>& node_indices_for_outputs, QString input_path, QString output_path)
{
    if (!output_path.endsWith(".prv")) {
        qWarning() << ".prv file must end with .prv extension";
        return false;
    }
    QJsonObject obj = make_prv_object(input_path);
    if (obj.isEmpty())
        return false;
    bool ok;
    QSet<int> node_indices_already_used; //to avoid infinite cycles, which can happen, for example, when an input file is the same as an output file
    obj["processes"] = get_prv_processes(nodes, node_indices_for_outputs, input_path, node_indices_already_used, &ok);
    if (!ok) {
        qWarning() << "Error in get_prv_processes";
        return false;
    }
    QString json = QJsonDocument(obj).toJson(QJsonDocument::Indented);
    return TextFile::write(output_path, json);
}

bool write_prv_files_for_process(const QList<PipelineNode>& nodes, const QMap<QString, int>& node_indices_for_outputs, PipelineNode* node)
{
    QStringList prv_keys = node->prvs.keys();
    foreach (QString key0, prv_keys) {
        if (!node->parameters.contains(key0)) {
            qWarning() << "No such parameter for prv key:" << key0 << node->processor_name;
            return false;
        }
        QString input_path = node->parameters.value(key0).toString();
        QString output_path = node->prvs.value(key0).toString();
        printf("Writing prv file: %s -> %s\n", input_path.toLatin1().data(), output_path.toLatin1().data());
        if (!write_prv_file(nodes, node_indices_for_outputs, input_path, output_path)) {
            qWarning() << "Error in write_prv_file";
            return false;
        }
    }
    return true;
}

bool ScriptController::runPipeline(const QString& json)
{
    ProcessManager* PM = ProcessManager::globalInstance();

    //parse the json
    QJsonParseError parse_err;
    QJsonObject pipe_obj = QJsonDocument::fromJson(json.toLatin1(), &parse_err).object();
    if (parse_err.error != QJsonParseError::NoError) {
        qWarning() << "runPipeline: error parsing json: " + parse_err.errorString();
        return false;
    }
    QJsonArray processes = pipe_obj["processes"].toArray();

    //set up the node tree
    QList<PipelineNode> nodes;
    for (int i = 0; i < processes.count(); i++) {
        QJsonObject process = processes[i].toObject();
        PipelineNode node;
        node.parameters = json_obj_to_variantmap(process["parameters"].toObject());
        node.prvs = json_obj_to_variantmap(process["prvs"].toObject());
        node.processor_name = process["processor_name"].toString();
        MLProcessor PP = PM->processor(node.processor_name);
        if (PP.name != node.processor_name) {
            qWarning() << "Unable to find processor *: " + node.processor_name;
            return false;
        }
        node.input_pnames = PP.inputs.keys();
        node.output_pnames = PP.outputs.keys();
        nodes << node;
    }

    //check for situation where input file matches output file
    for (int i = 0; i < nodes.count(); i++) {
        PipelineNode* node = &nodes[i];
        foreach (QString pname_input, node->input_pnames) {
            QString path_input = node->parameters[pname_input].toString();
            foreach (QString pname_output, node->output_pnames) {
                QString path_output = node->parameters[pname_output].toString();
                if ((!path_input.isEmpty()) && (path_input == path_output)) {
                    qDebug() << path_input;
                    qDebug() << node->processor_name;
                    qWarning() << "An input path is the same as an output path. This can happen sometimes when using .prv files (checksum lookups) in the case where a process creates an output file that matches an input file.";
                    return false;
                }
            }
        }
    }

    //record which outputs get created by which nodes (by index)
    QMap<QString, int> node_indices_for_outputs;
    for (int i = 0; i < nodes.count(); i++) {
        QStringList output_paths = nodes[i].output_paths();
        foreach (QString path, output_paths) {
            if (!path.isEmpty()) {
                if (node_indices_for_outputs.contains(path)) {
                    qWarning() << "Same output is created twice in pipeline.";
                    return false;
                }
            }
            node_indices_for_outputs[path] = i;
        }
    }

    //run the processing in parallel
    bool done = false;
    while (!done) {
        QVector<int> node_indices_ready_to_be_run;
        QVector<int> node_indices_not_ready_to_be_run;
        QVector<int> node_indices_running;
        QVector<int> node_indices_completed;
        for (int i = 0; i < nodes.count(); i++) {
            if (nodes[i].completed) {
                node_indices_completed << i;
            }
            else if (nodes[i].running) {
                node_indices_running << i;
            }
            else {
                bool ready_to_go = true;
                QStringList input_paths = nodes[i].input_paths();
                foreach (QString path, input_paths) {
                    if (node_indices_for_outputs.contains(path)) {
                        int ii = node_indices_for_outputs[path];
                        if (!nodes[ii].completed) {
                            ready_to_go = false;
                        }
                    }
                }
                if (ready_to_go)
                    node_indices_ready_to_be_run << i;
                else
                    node_indices_not_ready_to_be_run << i;
            }
        }
        if ((node_indices_running.isEmpty()) && (node_indices_ready_to_be_run.isEmpty()) && (node_indices_not_ready_to_be_run.isEmpty())) {
            //we are done!
            done = true;
        }
        if ((node_indices_running.isEmpty()) && (node_indices_ready_to_be_run.isEmpty()) && (!node_indices_not_ready_to_be_run.isEmpty())) {
            //Somehow we are not done, but nothing is ready and nothing is running.
            qWarning() << "Unable to run all processes in pipeline. Perhaps there are cyclic dependencies. For example a process may have an input file that matches an output file.";
            return false;
        }
        if (!node_indices_ready_to_be_run.isEmpty()) {
            //something is ready to run
            int ii = node_indices_ready_to_be_run[0];
            PipelineNode* node = &nodes[ii];
            if (!PM->checkParameters(node->processor_name, node->parameters)) {
                qWarning() << "Error checking parameters for processor: " + node->processor_name;
                return false;
            }
            if (node->processor_name.isEmpty()) { //empty processor name used for writing .prv files when the file is raw (ie does not come from an output)
                if (!write_prv_files_for_process(nodes, node_indices_for_outputs, node)) {
                    qWarning() << "Error in write_prv_files_for_process ***" << node->processor_name;
                    return false;
                }
                node->completed = true;
            }
            else if ((!d->m_force_run) && (PM->processAlreadyCompleted(node->processor_name, node->parameters))) {
                this->log(QString("Process already completed: %1").arg(node->processor_name));
                if (!write_prv_files_for_process(nodes, node_indices_for_outputs, node)) {
                    qWarning() << "Error in write_prv_files_for_process" << node->processor_name;
                    return false;
                }
                node->completed = true;
            }
            else {
                QProcess* P1;
                if (d->m_nodaemon) {
                    printf("Launching process %s\n", node->processor_name.toLatin1().data());
                    P1 = d->run_process(node->processor_name, node->parameters, d->m_force_run);
                    if (!P1) {
                        qWarning() << "Unable to launch process: " + node->processor_name;
                        return false;
                    }
                }
                else {
                    printf("Queuing process %s\n", node->processor_name.toLatin1().data());
                    P1 = d->queue_process(node->processor_name, node->parameters, false, d->m_force_run);
                    if (!P1) {
                        qWarning() << "Unable to queue process: " + node->processor_name;
                        return false;
                    }
                }

                node->running = true;
                node->qprocess = P1;
            }
        }
        //check for completed processes
        for (int i = 0; i < node_indices_running.count(); i++) {
            PipelineNode* node = &nodes[node_indices_running[i]];
            if (!node->qprocess) {
                qCritical() << "Unexpected problem" << __FILE__ << __LINE__;
                return false;
            }
            {
                QByteArray str = node->qprocess->readAll();
                if (!str.isEmpty()) {
                    printf("%s:: %s", node->processor_name.toLatin1().data(), str.data());
                }
            }
            if (node->qprocess->state() == QProcess::NotRunning) {
                printf("Process finished: %s\n", node->processor_name.toLatin1().data());
                node->qprocess->waitForReadyRead();
                QByteArray str = node->qprocess->readAll();
                if (!str.isEmpty()) {
                    printf("%s", str.data());
                }
                if (node->qprocess->exitStatus() == QProcess::CrashExit) {
                    qWarning() << "Process crashed: " + node->processor_name;
                    return false;
                }
                if (node->qprocess->exitCode() != 0) {
                    qWarning() << "Process returned with non-zero exit code: " + node->processor_name;
                    return false;
                }
                if (!write_prv_files_for_process(nodes, node_indices_for_outputs, node)) {
                    return false;
                }

                node->completed = true;
                node->running = false;
                delete node->qprocess;
                node->qprocess = 0;
            }
        }
        if (!done) {
            MPDaemon::wait(100);
            qApp->processEvents(); //important I think for detecting when processes end.
        }
    }

    d->m_pipeline_nodes = nodes;

    return true;
}

void ScriptController::log(const QString& message)
{
    printf("SCRIPT: %s\n", message.toLatin1().data());
}

QProcess* ScriptControllerPrivate::queue_process(QString processor_name, const QVariantMap& parameters, bool use_run, bool force_run)
{
    QString exe = qApp->applicationFilePath();
    QStringList args;
    if (use_run) {
        args << "run-process";
    }
    else {
        args << "queue-process";
    }
    args << processor_name;
    QStringList pkeys = parameters.keys();
    foreach (QString pkey, pkeys) {
        args << QString("--%1=%2").arg(pkey).arg(parameters[pkey].toString());
    }
    args << QString("--_parent_pid=%1").arg(QCoreApplication::applicationPid());
    if (force_run) {
        args << "--_force_run";
    }
    QProcess* P1 = new QProcess;
    P1->setReadChannelMode(QProcess::MergedChannels);
    P1->start(exe, args);
    if (!P1->waitForStarted()) {
        qWarning() << "Error waiting for process to start: " + processor_name;
        delete P1;
        return 0;
    }
    return P1;
}

QProcess* ScriptControllerPrivate::run_process(QString processor_name, const QVariantMap& parameters, bool force_run)
{
    return ScriptControllerPrivate::queue_process(processor_name, parameters, true, force_run);
}

/*
bool ScriptControllerPrivate::queue_process_and_wait_for_finished(QString processor_name, const QVariantMap& parameters)
{
    QString exe = qApp->applicationFilePath();
    QStringList args;
    args << "queue-process";
    args << processor_name;
    QStringList pkeys = parameters.keys();
    foreach (QString pkey, pkeys) {
        args << QString("--%1=%2").arg(pkey).arg(parameters[pkey].toString());
    }
    args << QString("--_parent_pid=%1").arg(QCoreApplication::applicationPid());
    QProcess P1;
    P1.setReadChannelMode(QProcess::MergedChannels);
    P1.start(exe, args);
    if (!MPDaemon::waitForFinishedAndWriteOutput(&P1)) {
        //if (!P1.waitForFinished(-1)) {
        printf("Error waiting for queue-process to finish: %s\n", processor_name.toLatin1().data());
        return false;
    }
    if (P1.exitStatus() == QProcess::CrashExit) {
        printf("Error -- queue-process crashed: %s\n", processor_name.toLatin1().data());
        return false;
    }
    if (P1.exitCode() != 0) {
        printf("Error -- queue-process returned non-zero exit code: %s\n", processor_name.toLatin1().data());
        return false;
    }
    return true;
}
*/

void ScriptControllerPrivate::wait(qint64 msec)
{
    usleep(msec * 1000);
}

QString ScriptControllerPrivate::resolve_file_name_p(QString fname_in)
{
    return resolve_file_name(m_server_urls, m_server_base_path, fname_in);
}

QString resolve_file_name(QStringList server_urls, QString server_base_path, QString fname_in)
{
    //This is terrible, we need to fix it!
    QString fname = fname_in;
    foreach (QString str, server_urls) {
        if (fname.startsWith(str + "/mdaserver")) {
            fname = server_base_path + "/" + fname.mid((str + "/mdaserver").count());
            if (fname.mid(server_base_path.count()).contains("..")) {
                qWarning() << "Illegal .. in file path: " + fname;
                fname = "";
            }
        }
    }
    /*
    Taking this out to resolve Jason's problem.
    /// TODO Not sure why I had this in ... raw data does not have to be in this path... I think it was for security. Think about it.
    if (!fname.startsWith(server_base_path)) {
        qWarning() << "Path does not start with " + server_base_path + ": " + fname;
        fname = "";
    }
    */

    return fname;
}
