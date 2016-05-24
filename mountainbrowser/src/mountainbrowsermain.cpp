/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include <QApplication>
#include "commandlineparams.h"
#include "textfile.h"
#include <QDebug>
#include <QWebInspector>
#include <QWebView>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QWebFrame>
#include <QJsonDocument>
#include "mbcontroller.h"
#include "msmisc.h"

#include "mountainbrowsermain.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    CLParams CLP = commandlineparams(argc, argv);

    QString mountainbrowser_url = CLP.named_parameters.value("url", "http://datalaboratory.org:8002").toString();

    QString config_json = http_get_text(mountainbrowser_url + "?a=getConfig");
    QJsonObject config = (QJsonDocument::fromJson(config_json.toLatin1())).object();

    QString mdaserver_url = config["mdaserver_url"].toString();
    //QString mscmdserver_url = config["mscmdserver_url"].toString();
    QString mpserver_url = config["mpserver_url"].toString();

    MBController controller;
    controller.setMountainBrowserUrl(mountainbrowser_url);
    //controller.setMscmdServerUrl(mscmdserver_url);
    controller.setMPServerUrl(mpserver_url);
    controller.setMdaServerUrl(mdaserver_url);

    QWebView* X = new QWebView;
    MyPage* page = new MyPage;
    page->setController(&controller);
    X->setPage(page);
    X->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    /*
    QWebInspector* WI = new QWebInspector;
    WI->setPage(X->page());
    WI->show();
    WI->move(100, 100);
    WI->resize(1000, 1000);
    */

    X->load(QUrl("qrc:/html/mbstudyview.html"));
    X->show();

    return a.exec();
}

MyPage::MyPage()
{
    m_controller = 0;
    connect(this->mainFrame(), SIGNAL(urlChanged(QUrl)), this, SLOT(slot_url_changed()), Qt::DirectConnection);
}

void MyPage::slot_url_changed()
{
    if (m_controller) {
        this->mainFrame()->addToJavaScriptWindowObject("MB", m_controller, QWebFrame::QtOwnership);
    }
}

void MyPage::javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID)
{
    QString str = QString("JS [%1:%2]  %3").arg(sourceID).arg(lineNumber).arg(message);
    QWebPage::javaScriptConsoleMessage(message, lineNumber, sourceID);
}
