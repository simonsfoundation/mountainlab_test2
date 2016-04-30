/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#include "commandlineparams.h"

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QSettings>
#include "processmanager.h"

void print_usage();

int main(int argc,char *argv[]) {
    QCoreApplication app(argc,argv);
    CLParams CLP=commandlineparams(argc,argv);

    /*
     * Load configuration file. If it doesn't exist, copy example configuration file.
     */
    QString config_fname=app.applicationDirPath()+"/mountainprocess.ini";
    if (!QFile::exists(config_fname)) {
        if (!QFile::copy(config_fname+".example",config_fname)) {
            qWarning() << "Unable to copy example configuration file to "+config_fname;
            return -1;
        }
    }
    QSettings config(config_fname,QSettings::IniFormat);

    /*
     * Load the processor paths
     */
    QStringList processor_paths=config.value("processor_paths").toStringList();
    if (processor_paths.isEmpty()) {
        qWarning() << "No processor paths found in "+config_fname;
    }
    qDebug()  << "Searching processor paths: " << processor_paths;

    /*
     * Initialize the process manager
     */
    ProcessManager PM;
    foreach (QString processor_path,processor_paths) {
        PM.loadProcessors(processor_path);
    }

    QString arg1=CLP.unnamed_parameters.value(0);
    QString arg2=CLP.unnamed_parameters.value(1);

    if (arg1=="runProcess") {
        QString processor_name=arg2;
        QVariantMap parameters=CLP.named_parameters;
        if (!PM.checkParameters(processor_name,parameters)) {
            qWarning() << "Problem checking process" << processor_name;
            return -1;
        }
        QString id=PM.startProcess(processor_name,parameters);
        if (id.isEmpty()) {
            qWarning() << "Problem starting process" << processor_name;
            return -1;
        }
        if (!PM.waitForFinished(id,-1)) {
            qWarning() << "Problem waiting for process to finish" << processor_name;
            return -1;
        }
        MLProcessInfo info=PM.processInfo(id);
        if (info.exit_status==QProcess::CrashExit) {
            qWarning() << "Process crashed" << processor_name;
            return -1;
        }
        qDebug()  << "==STANDARD OUTPUT===";
        qDebug()  << info.standard_output;
        qDebug()  << "==STANDARD ERROR===";
        qDebug()  << info.standard_error;
        PM.clearProcess(id);
        return info.exit_code;
    }
    else {
        print_usage();
        return -1;
    }

    return 0;
}

void print_usage() {
    printf("Usage:\n");
    printf("mountainprocess runProcess [processor_name] --[param1]=[val1] --[param2]=[val2] ...\n");
}
