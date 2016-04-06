/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include <QApplication>
#include "get_command_line_params.h"
#include "mbmainwindow.h"
#include "mbexperimentmanager.h"
#include "textfile.h"
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    CLParams CLP = get_command_line_params(argc, argv);

    QString json_fname = qApp->applicationDirPath() + "/../src/experiments.json";
    QString json_txt = read_text_file(json_fname);

    MBExperimentManager* EM = new MBExperimentManager;
    EM->loadExperiments(json_txt);

    MBMainWindow* W = new MBMainWindow;
    W->resize(800, 600);
    W->show();
    W->setExperimentManager(EM);

    return a.exec();
}
