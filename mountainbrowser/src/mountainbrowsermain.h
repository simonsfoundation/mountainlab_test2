
#ifndef MOUNTAINSORTBROWSERMAIN_H
#define MOUNTAINSORTBROWSERMAIN_H

#include <QtGlobal>
#if QT_VERSION >= 0x050600
#define USE_WEBENGINE
#endif

#include <QString>
#include <QObject>
#ifdef USE_WEBENGINE
#include <QWebEnginePage>
typedef QWebEnginePage QWebPage;
#else
#include <QWebPage>
#endif
#include "mbcontroller.h"

class MyPage : public QWebPage {
    Q_OBJECT
public:
    MyPage();
    virtual ~MyPage() {};
    void setController(MBController *controller);;
private slots:
    void slot_url_changed();
protected:
#ifdef USE_WEBENGINE
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString & message, int lineNumber, const QString & sourceID);
#else
    virtual void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
#endif
private:
    MBController *m_controller;
};

#endif
