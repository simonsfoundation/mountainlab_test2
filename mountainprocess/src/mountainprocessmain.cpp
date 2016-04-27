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

    /*
     * Initialize the process manager
     */
    ProcessManager PM;
    foreach (QString processor_path,processor_paths) {
        PM.loadProcessors(processor_path);
    }

    QString arg1=CLP.unnamed_parameters.value(0);
    QString arg2=CLP.unnamed_parameters.value(1);

    if (arg1=="run") {

    }
    else {
        print_usage();
        return -1;
    }

    return 0;
}

void print_usage() {
    printf("Usage:\n");
    printf("mountainprocess run [processor_name] --[param1]=[val1] --[param2]=[val2] ...\n");
}
