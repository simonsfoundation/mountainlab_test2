/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#include "scriptcontroller2.h"

#include <QCryptographicHash>
#include <QDir>
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

struct PipelineNode2 {
    PipelineNode2()
    {
        completed = false;
        running = false;
        qprocess = 0;
    }
    ~PipelineNode2()
    {
        if (qprocess) {
            delete qprocess;
        }
    }
    QMap<QString, QDateTime> timestamps;
    QString processor_name;
    QVariantMap inputs;
    QVariantMap parameters;
    QVariantMap outputs;
    bool create_prv = false;
    bool completed;
    bool running;
    QProcess* qprocess;

    QStringList input_paths()
    {
        QStringList input_pnames = inputs.keys();
        QStringList ret;
        foreach (QString pname, input_pnames)
            ret << inputs[pname].toString();
        return ret;
    }
    QStringList output_paths()
    {
        QStringList output_pnames = outputs.keys();
        QStringList ret;
        foreach (QString pname, output_pnames)
            ret << outputs[pname].toString();
        return ret;
    }
};

QJsonArray get_prv_processes_2(const QList<PipelineNode2>& nodes, const QMap<QString, int>& node_indices_for_outputs, QString path, QSet<int>& node_indices_already_used, bool* ok);

class ScriptController2Private {
public:
    ScriptController2Private()
    {
        m_nodaemon = false;
    }

    ScriptController2* q;
    bool m_nodaemon = false;
    //QStringList m_server_urls;
    //QString m_server_base_path;
    bool m_force_run = false;
    QString m_working_path;
    QJsonObject m_results;

    QList<PipelineNode2> m_pipeline_nodes;

    QProcess* queue_process(QString processor_name, const QVariantMap& parameters, bool use_run, bool force_run);
    QProcess* run_process(QString processor_name, const QVariantMap& parameters, bool force_run);

    void resolve_file_names(QVariantMap& fnames);
    QString resolve_file_name_p(QString fname);

    bool run_or_queue_node(PipelineNode2* node, const QMap<QString, int>& node_indices_for_outputs);
    PipelineNode2* find_node_ready_to_run();
    QString create_temporary_path_for_output(QString processor_name, QVariantMap inputs, QVariantMap parameters, QString output_pname);
    bool handle_running_processes();
};

ScriptController2::ScriptController2()
{
    d = new ScriptController2Private;
    d->q = this;
}

ScriptController2::~ScriptController2()
{
    delete d;
}

void ScriptController2::setNoDaemon(bool val)
{
    d->m_nodaemon = val;
}

/*
void ScriptController2::setServerUrls(const QStringList& urls)
{
    d->m_server_urls = urls;
}

void ScriptController2::setServerBasePath(const QString& path)
{
    d->m_server_base_path = path;
}
*/

void ScriptController2::setForceRun(bool force_run)
{
    d->m_force_run = force_run;
}

void ScriptController2::setWorkingPath(QString working_path)
{
    d->m_working_path = working_path;
}

QJsonObject ScriptController2::getResults()
{
    return d->m_results;
}

QString ScriptController2::addProcess(QString processor_name, QString inputs_json, QString parameters_json, QString outputs_json)
{
    MLProcessor PP = ProcessManager::globalInstance()->processor(processor_name);
    if (PP.name != processor_name) { //rather use PP.isNull()
        qWarning() << "Unable to find processor **: " + processor_name;
        return "{}";
    }
    QMap<QString, MLParameter> PP_outputs = PP.outputs;

    PipelineNode2 node;
    /// TODO: check for json parse errors here
    node.processor_name = processor_name;
    node.inputs = QJsonDocument::fromJson(inputs_json.toLatin1()).object().toVariantMap();
    node.parameters = QJsonDocument::fromJson(parameters_json.toLatin1()).object().toVariantMap();
    if (!outputs_json.isEmpty()) {
        node.outputs = QJsonDocument::fromJson(outputs_json.toLatin1()).object().toVariantMap();
        /// TODO: check to see if outputs are consistent with PP_outputs
    }
    else {
        QStringList output_pnames = PP_outputs.keys();
        foreach (QString pname, output_pnames) {
            node.outputs[pname] = d->create_temporary_path_for_output(node.processor_name, node.inputs, node.parameters, pname);
        }
    }
    d->resolve_file_names(node.inputs);
    d->resolve_file_names(node.outputs);
    d->m_pipeline_nodes << node;

    return QJsonDocument(QJsonObject::fromVariantMap(node.outputs)).toJson();
}

void ScriptController2::addPrv(QString input_path, QString output_path)
{
    PipelineNode2 node;
    node.inputs["input"] = d->resolve_file_name_p(input_path);
    node.outputs["output"] = d->resolve_file_name_p(output_path);
    node.create_prv = true;
    d->m_pipeline_nodes << node;
}

bool ScriptController2::runPipeline()
{
    QDateTime timestamp_start = QDateTime::currentDateTime();
    //check for empty output paths
    for (int i = 0; i < d->m_pipeline_nodes.count(); i++) {
        PipelineNode2* node = &d->m_pipeline_nodes[i];
        QStringList output_paths = node->output_paths();
        foreach (QString path, output_paths) {
            if (path.isEmpty()) {
                qWarning() << "Output path is empty" << node->processor_name << node->input_paths() << node->output_paths();
                return false;
            }
        }
    }

    //check for situation where input file matches output file
    for (int i = 0; i < d->m_pipeline_nodes.count(); i++) {
        PipelineNode2* node = &d->m_pipeline_nodes[i];
        QSet<QString> input_paths_set = node->input_paths().toSet();
        QStringList output_paths = node->output_paths();
        foreach (QString output_path, output_paths) {
            if (input_paths_set.contains(output_path)) {
                qWarning() << node->processor_name << output_path;
                qWarning() << "An input path is the same as an output path. This can happen sometimes when using .prv files (checksum lookups) in the case where a process creates an output file that matches an input file.";
                return false;
            }
        }
    }

    //record which outputs get created by which nodes (by index)
    QMap<QString, int> node_indices_for_outputs;
    for (int i = 0; i < d->m_pipeline_nodes.count(); i++) {
        QStringList output_paths = d->m_pipeline_nodes[i].output_paths();
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

    bool done = false;
    while (!done) {
        bool found = true;
        while (found) {
            found = false;
            PipelineNode2* node = d->find_node_ready_to_run();
            if (node) {
                if (!d->run_or_queue_node(node, node_indices_for_outputs)) {
                    return false;
                }
                found = true;
            }
        }

        if (!d->handle_running_processes()) {
            return false;
        }

        //check to see if we are done
        if (!d->find_node_ready_to_run()) {
            int num_running = 0;
            for (int i = 0; i < d->m_pipeline_nodes.count(); i++) {
                PipelineNode2* node = &d->m_pipeline_nodes[i];
                if (node->running)
                    num_running++;
            }
            if (num_running == 0)
                done = true; //if nothing is ready to run, and nothing is running, then we are done
        }

        if (!done) {
            MPDaemon::wait(100);
            qApp->processEvents(); //important I think for detecting when processes end.
        }
    }

    //check whether everything got run
    for (int i = 0; i < d->m_pipeline_nodes.count(); i++) {
        PipelineNode2* node = &d->m_pipeline_nodes[i];
        if (!node->completed) {
            qWarning() << "Not every process in the pipeline was run. For example: " + node->processor_name + ". This could be due to a cyclic dependency.";
            return false;
        }
    }

    QDateTime timestamp_finish = QDateTime::currentDateTime();
    d->m_results["total_time_sec"] = timestamp_start.msecsTo(timestamp_finish) * 1.0 / 1000;
    return true;
}

QJsonObject make_prv_object_2(QString path)
{
    if (!QFile::exists(path)) {
        qWarning() << "Unable to find file (for prv):" << path;
        return QJsonObject();
    }
    if (path.endsWith(".prv")) {
        //this important section added on 10/12/16 by jfm
        QString json = TextFile::read(path);
        return QJsonDocument::fromJson(json.toUtf8()).object();
    }
    QJsonObject obj;
    obj["original_path"] = path;
    obj["original_checksum"] = MLUtil::computeSha1SumOfFile(path);
    obj["original_size"] = QFileInfo(path).size();
    return obj;
}

void ScriptController2::log(const QString& message)
{
    printf("SCRIPT: %s\n", message.toLatin1().data());
}

QProcess* ScriptController2Private::queue_process(QString processor_name, const QVariantMap& parameters, bool use_run, bool force_run)
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

QProcess* ScriptController2Private::run_process(QString processor_name, const QVariantMap& parameters, bool force_run)
{
    return ScriptController2Private::queue_process(processor_name, parameters, true, force_run);
}

void ScriptController2Private::resolve_file_names(QVariantMap& fnames)
{
    QStringList pnames = fnames.keys();
    foreach (QString pname, pnames) {
        fnames[pname] = resolve_file_name_p(fnames[pname].toString());
    }
}

QString ScriptController2Private::resolve_file_name_p(QString fname_in)
{
    if (fname_in.isEmpty())
        return "";
    QString ret = fname_in;
    //QString ret = resolve_file_name_2(m_server_urls, m_server_base_path, fname_in);
    if (!ret.startsWith("http:")) {
        if ((QDir::isRelativePath(ret)) && (!m_working_path.isEmpty())) {
            ret = m_working_path + "/" + ret;
        }
    }
    return ret;
}

bool ScriptController2Private::run_or_queue_node(PipelineNode2* node, const QMap<QString, int>& node_indices_for_outputs)
{
    QVariantMap parameters0;
    {
        QStringList pnames = node->inputs.keys();
        foreach (QString pname, pnames) {
            parameters0[pname] = node->inputs[pname];
        }
    }
    {
        QStringList pnames = node->parameters.keys();
        foreach (QString pname, pnames) {
            parameters0[pname] = node->parameters[pname];
        }
    }
    {
        QStringList pnames = node->outputs.keys();
        foreach (QString pname, pnames) {
            parameters0[pname] = node->outputs[pname];
        }
    }

    ProcessManager* PM = ProcessManager::globalInstance();
    if (node->create_prv) {
        QString input_path = node->inputs["input"].toString();
        QString output_path = node->outputs["output"].toString();
        if (input_path.isEmpty()) {
            qWarning() << "Input path for create_prv is empty.";
            return false;
        }
        if (!QFile::exists(input_path)) {
            qWarning() << "Input file for create_prv does not exist: " + input_path;
            return false;
        }
        if (!output_path.endsWith(".prv")) {
            qWarning() << "Problem creating .prv file. The file path must end with .prv.";
            return false;
        }

        if (!output_path.endsWith(".prv")) {
            qWarning() << ".prv file must end with .prv extension";
            return false;
        }
        QJsonObject obj = make_prv_object_2(input_path);
        if (obj.isEmpty())
            return false;
        bool ok;
        QSet<int> node_indices_already_used; //to avoid infinite cycles, which can happen, for example, when an input file is the same as an output file
        obj["processes"] = get_prv_processes_2(m_pipeline_nodes, node_indices_for_outputs, input_path, node_indices_already_used, &ok);
        if (!ok) {
            qWarning() << "Error in get_prv_processes";
            return false;
        }
        QString obj_json = QJsonDocument(obj).toJson(QJsonDocument::Indented);
        if (TextFile::write(output_path, obj_json)) {
            node->completed = true;
            return true;
        }
        else {
            qWarning() << "Unable to write prv file: " << output_path;
            return false;
        }
    }
    if (!PM->checkParameters(node->processor_name, parameters0)) {
        qWarning() << "Error checking parameters for processor: " + node->processor_name;
        return false;
    }
    if ((!m_force_run) && (PM->processAlreadyCompleted(node->processor_name, parameters0))) {
        q->log(QString("Process already completed: %1").arg(node->processor_name));
        node->completed = true;
        return true;
    }
    else {
        QProcess* P1;
        if (m_nodaemon) {
            printf("Launching process %s\n", node->processor_name.toLatin1().data());
            P1 = run_process(node->processor_name, parameters0, m_force_run);
            if (!P1) {
                qWarning() << "Unable to launch process: " + node->processor_name;
                return false;
            }
        }
        else {
            printf("Queuing process %s\n", node->processor_name.toLatin1().data());
            P1 = queue_process(node->processor_name, parameters0, false, m_force_run);
            if (!P1) {
                qWarning() << "Unable to queue process: " + node->processor_name;
                return false;
            }
        }

        node->running = true;
        node->qprocess = P1;
        return true;
    }
}

PipelineNode2* ScriptController2Private::find_node_ready_to_run()
{
    QSet<QString> file_paths_waiting_to_be_created;
    for (int i = 0; i < m_pipeline_nodes.count(); i++) {
        PipelineNode2* node = &m_pipeline_nodes[i];
        if (!node->completed) {
            QStringList pnames = node->outputs.keys();
            foreach (QString pname, pnames) {
                QString output_path = node->outputs[pname].toString();
                file_paths_waiting_to_be_created.insert(output_path);
            }
        }
    }
    for (int i = 0; i < m_pipeline_nodes.count(); i++) {
        PipelineNode2* node = &m_pipeline_nodes[i];
        if ((!node->completed) && (!node->running)) {
            bool ready_to_run = true;
            QStringList pnames = node->inputs.keys();
            foreach (QString pname, pnames) {
                QString input_path = node->inputs[pname].toString();
                if (file_paths_waiting_to_be_created.contains(input_path)) {
                    ready_to_run = false;
                }
            }
            if (ready_to_run) {
                return node;
            }
        }
    }
    return 0;
}

QString ScriptController2Private::create_temporary_path_for_output(QString processor_name, QVariantMap inputs, QVariantMap parameters, QString output_pname)
{
    QJsonObject obj;
    obj["processor_name"] = processor_name;
    obj["inputs"] = QJsonObject::fromVariantMap(inputs);
    obj["parameters"] = QJsonObject::fromVariantMap(parameters);
    obj["output_pname"] = output_pname;
    QString json = QJsonDocument(obj).toJson();
    QString code = MLUtil::computeSha1SumOfString(json);
    return CacheManager::globalInstance()->makeLocalFile(code + "-" + processor_name + "-" + output_pname + ".tmp", CacheManager::LongTerm);
}

bool ScriptController2Private::handle_running_processes()
{
    for (int i = 0; i < m_pipeline_nodes.count(); i++) {
        PipelineNode2* node = &m_pipeline_nodes[i];
        if (node->running) {
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
                node->completed = true;
                node->running = false;
                delete node->qprocess;
                node->qprocess = 0;
            }
        }
    }
    return true;
}

/*
QString resolve_file_name_2(QStringList server_urls, QString server_base_path, QString fname_in)
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

    return fname;
}
*/

QJsonArray get_prv_processes_2(const QList<PipelineNode2>& nodes, const QMap<QString, int>& node_indices_for_outputs, QString path, QSet<int>& node_indices_already_used, bool* ok)
{
    QJsonArray processes;
    int ind0 = node_indices_for_outputs.value(path, -1);
    if ((ind0 >= 0) && (!node_indices_already_used.contains(ind0))) { //avoid infinite cycles, which can happen, for example, if an output file is the same as an input file
        const PipelineNode2* node = &nodes[ind0];
        if (!node->processor_name.isEmpty()) {
            QJsonObject process;
            process["processor_name"] = node->processor_name;
            {
                QStringList input_pnames = node->inputs.keys();
                QJsonObject inputs;
                foreach (QString pname, input_pnames) {
                    QJsonObject tmp;
                    tmp = make_prv_object_2(node->inputs[pname].toString());
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
                QStringList output_pnames = node->outputs.keys();
                foreach (QString pname, output_pnames) {
                    QJsonObject tmp;
                    tmp = make_prv_object_2(node->outputs[pname].toString());
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
                    parameters[pname] = node->parameters[pname].toString();
                }
                process["parameters"] = parameters;
            }
            node_indices_already_used.insert(ind0);
            processes.append(process);
            {
                QStringList input_pnames = node->inputs.keys();
                foreach (QString pname, input_pnames) {
                    QJsonArray X = get_prv_processes_2(nodes, node_indices_for_outputs, node->inputs[pname].toString(), node_indices_already_used, ok);
                    if (!ok)
                        return processes;
                    for (int a = 0; a < X.count(); a++) {
                        processes.append(X[a]);
                    }
                }
            }
        }
    }
    *ok = true;
    return processes;
}
