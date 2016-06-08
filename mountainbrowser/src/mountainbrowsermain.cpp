/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include "mountainbrowsermain.h"
#include <QApplication>
#include "commandlineparams.h"
#include "textfile.h"
#include <QDebug>
#ifdef USE_WEBENGINE
#include <QWebEngineView>
#include <QWebEngineSettings>
typedef QWebEngineView QWebView;
#include <QWebChannel>
#else
#include <QWebInspector>
#include <QWebView>
#include <QWebFrame>
#endif

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include "mbcontroller.h"
#include "msmisc.h"

#include "mountainbrowsermain.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    CLParams CLP = commandlineparams(argc, argv);

    QString mlproxy_url = CLP.named_parameters.value("url", "http://datalaboratory.org:8020").toString();

    MBController controller;
    controller.setMLProxyUrl(mlproxy_url);

    QWebView* X = new QWebView;
    MyPage* page = new MyPage;
    page->setController(&controller);
    X->setPage(page);
//    X->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

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
#ifdef USE_WEBENGINE
    connect(this, SIGNAL(urlChanged(QUrl)), this, SLOT(slot_url_changed()), Qt::DirectConnection);
#else
    connect(this->mainFrame(), SIGNAL(urlChanged(QUrl)), this, SLOT(slot_url_changed()), Qt::DirectConnection);
#endif
}

#include <QtDebug>

void MyPage::setController(MBController *controller) {
    m_controller=controller;
#ifdef USE_WEBENGINE
    if (webChannel()) delete webChannel();
    QWebChannel *channel = new QWebChannel(this);
    setWebChannel(channel);
    channel->registerObject("MB", m_controller);
    qDebug() << "Registering object";
#endif
}

void MyPage::slot_url_changed()
{
#ifdef USE_WEBENGINE
//    QWebChannel *channel = new QWebChannel(this);
//    setWebChannel(channel);
//    channel->registerObject("MB", m_controller);
#else
    if (m_controller) {
        this->mainFrame()->addToJavaScriptWindowObject("MB", m_controller, QWebFrame::QtOwnership);
    }
#endif
}
#ifdef USE_WEBENGINE
void MyPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) {
    QString str = QString("JS [%1:%2]  %3").arg(sourceID).arg(lineNumber).arg(message);
    QWebEnginePage::javaScriptConsoleMessage(WarningMessageLevel, str, lineNumber, sourceID);
}
#else
void MyPage::javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID) {
{
    QString str = QString("JS [%1:%2]  %3").arg(sourceID).arg(lineNumber).arg(message);
    QWebPage::javaScriptConsoleMessage(message, lineNumber, sourceID);
}
#endif
