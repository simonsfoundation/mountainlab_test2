/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#include "commandlineparams.h"
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
#include "processmanager.h"
#include "textfile.h"

void print_usage();
void load_parameter_file(QVariantMap& params, const QString& fname);
bool run_script(const QStringList& script_fnames, const QVariantMap& params);
bool initialize_process_manager();

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    CLParams CLP = commandlineparams(argc, argv);

    QString arg1 = CLP.unnamed_parameters.value(0);
    QString arg2 = CLP.unnamed_parameters.value(1);

    if (arg1 == "run-process") {
        if (!initialize_process_manager()) return -1;
        ProcessManager* PM = ProcessManager::globalInstance();

        QString processor_name = arg2;
        QVariantMap parameters = CLP.named_parameters;
        if (!PM->checkParameters(processor_name, parameters)) {
            qWarning() << "Problem checking process" << processor_name;
            return -1;
        }
        QString id = PM->startProcess(processor_name, parameters);
        if (id.isEmpty()) {
            qWarning() << "Problem starting process" << processor_name;
            return -1;
        }
        if (!PM->waitForFinished(id, -1)) {
            qWarning() << "Problem waiting for process to finish" << processor_name;
            return -1;
        }
        MLProcessInfo info = PM->processInfo(id);
        if (info.exit_status == QProcess::CrashExit) {
            qWarning() << "Process crashed" << processor_name;
            return -1;
        }
        qDebug() << "==STANDARD OUTPUT===";
        qDebug() << info.standard_output;
        qDebug() << "==STANDARD ERROR===";
        qDebug() << info.standard_error;
        PM->clearProcess(id);
        return info.exit_code;
    }
    else if (arg1 == "run-script") {
        if (!initialize_process_manager()) return -1;

        QStringList script_fnames;
        QVariantMap params;
        for (int i = 0; i < CLP.unnamed_parameters.count(); i++) {
            QString str = CLP.unnamed_parameters[i];
            if (str.endsWith(".js")) {
                script_fnames << str;
            }
            if (str.endsWith(".par")) { // note that we can have multiple parameter files! the later ones override the earlier ones.
                load_parameter_file(params, str);
            }
        }
        if (!run_script(script_fnames, params))
            return -1;
        return 0;
    }
    else if (arg1 == "-internal-daemon-start") {
        MPDaemon X;
        if (!X.run())
            return -1;
        return 0;
    }
    else if (arg1 == "daemon-start") {
        MPDaemonInterface X;
        if (X.start())
            return 0;
        else
            return -1;
    }
    else if (arg1 == "daemon-stop") {
        MPDaemonInterface X;
        if (X.stop())
            return 0;
        else
            return -1;
    }
    else if (arg1 == "daemon-info") {
        MPDaemonInterface X;
        QJsonObject info = X.getInfo();
        QString json = QJsonDocument(info).toJson();
        printf("%s", json.toLatin1().data());
        return 0;
    }
    else if (arg1 == "queue-script") {
        MPDaemonScript S = default_daemon_script();
        QVariantMap params;
        for (int i = 0; i < CLP.unnamed_parameters.count(); i++) {
            QString str = CLP.unnamed_parameters[i];
            if (str.endsWith(".js")) {
                S.script_paths << str;
            }
            if (str.endsWith(".par")) { // note that we can have multiple parameter files! the later ones override the earlier ones.
                load_parameter_file(params, str);
            }
        }
        S.script_id=CLP.named_parameters["script_id"].toString();
        if (S.script_id.isEmpty()) {
            S.script_id=make_random_id();
            printf("script_id: %s\n",S.script_id.toLatin1().data());
        }
        S.parameters = params;
        MPDaemonInterface X;
        X.queueScript(S);
        return 0;
    }
    else {
        print_usage();
        return -1;
    }

    return 0;
}

bool initialize_process_manager()
{
    /*
     * Load configuration file. If it doesn't exist, copy example configuration file.
     */
    QString config_fname = qApp->applicationDirPath() + "/mountainprocess.ini";
    if (!QFile::exists(config_fname)) {
        if (!QFile::copy(config_fname + ".example", config_fname)) {
            qWarning() << "Unable to copy example configuration file to " + config_fname;
            return false;
        }
    }
    QSettings config(config_fname, QSettings::IniFormat);
    /*
     * Load the processor paths
     */
    QStringList processor_paths = config.value("processor_paths").toStringList();
    if (processor_paths.isEmpty()) {
        qWarning() << "No processor paths found in " + config_fname;
    }
    qDebug() << "Searching processor paths: " << processor_paths;

    /*
     * Load the processors
     */
    ProcessManager* PM = ProcessManager::globalInstance();
    foreach (QString processor_path, processor_paths) {
        PM->loadProcessors(processor_path);
    }

    return true;
}

void load_parameter_file(QVariantMap& params, const QString& fname)
{
    /// Witold maybe a bad idea but.... I would like for user to optionally specify the parameters in a more intuitive way than json. Like freq_min=300\nfreq_max=6000, etc. I'd like to support both formats and detect which one.
    /// Witold I use toLatin1() everywhere. Is this the appropriate way to convert to byte array?
    QJsonObject obj = QJsonDocument::fromJson(read_text_file(fname).toLatin1()).object();
    QStringList keys = obj.keys();
    foreach (QString key, keys) {
        params[key] = obj[key].toVariant();
    }
}

void display_error(QJSValue result)
{
    /// Witold there must be a better way to print the QJSValue error message out to the console.
    /// Witold In general is it possible to not display quotes around strings for qDebug?
    qDebug() << result.property("name").toString();
    qDebug() << result.property("message").toString();
    qDebug() << QString("%1 line %2").arg(result.property("fileName").toString()).arg(result.property("lineNumber").toInt());
}

bool run_script(const QStringList& script_fnames, const QVariantMap& params)
{
    QJSEngine engine;
    ScriptController Controller;
    QJSValue MP = engine.newQObject(&Controller);
    engine.globalObject().setProperty("MP", MP);
    foreach (QString fname, script_fnames) {
        QJSValue result = engine.evaluate(read_text_file(fname), fname);
        if (result.isError()) {
            display_error(result);
            qCritical() << "Error running script.";
            return false;
        }
    }

    {
        QStringList param_keys = params.keys();
        QJsonObject params_obj;
        foreach (QString key, param_keys) {
            params_obj[key] = params[key].toString();
        }
        QString params_json = QJsonDocument(params_obj).toJson(QJsonDocument::Compact);
        QString str = QString("main(JSON.parse('%1'));").arg(params_json);
        QJSValue result = engine.evaluate(str);
        if (result.isError()) {
            display_error(result);
            qCritical() << "Error running script.";
            return false;
        }
    }

    return true;
}

void print_usage()
{
    printf("Usage:\n");
    printf("mountainprocess runProcess [processor_name] --[param1]=[val1] --[param2]=[val2] ...\n");
    printf("mountainprocess runScript [script1].js [script2.js] ... [file1].par [file2].par ... \n");
    printf("mountainprocess daemon-start\n");
    printf("mountainprocess daemon-stop\n");
    printf("mountainprocess daemon-info\n");
    printf("mountainprocess queue-script --script_id=optional_unique_id [script1].js [script2.js] ... [file1].par [file2].par ... \n");
}
