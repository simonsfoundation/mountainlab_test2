/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include <QApplication>
#include "get_command_line_params.h"
#include "textfile.h"
#include <QDebug>
#include <QWebInspector>
#include <QWebView>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QWebFrame>
#include "mbcontroller.h"

#include "mountainbrowsermain.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    CLParams CLP = get_command_line_params(argc, argv);

    QWebView* X = new QWebView;
    X->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    /*
    QWebInspector* WI = new QWebInspector;
    WI->setPage(X->page());
    WI->show();
    WI->move(100, 100);
    WI->resize(1000, 1000);
    */

    //QString json=read_text_file("/mnt/xfs1/home/magland/dev/mountainlab/mountainbrowser/src/experiments.json");

    //MyLocalStudy *LocalStudy=new MyLocalStudy(json);

    MBController *controller=new MBController;
    X->page()->mainFrame()->addToJavaScriptWindowObject("MB",controller,QWebFrame::ScriptOwnership);

    X->load(QUrl("qrc:/html/mbstudyview.html"));
    X->show();

    /*
    MBExperimentManager* EM = new MBExperimentManager;
    EM->loadExperiments(json_txt);

    MBMainWindow* W = new MBMainWindow;
    W->resize(800, 600);
    W->show();
    W->setExperimentManager(EM);
    */

    return a.exec();
}
