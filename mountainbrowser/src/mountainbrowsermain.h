
#ifndef MOUNTAINSORTBROWSERMAIN_H
#define MOUNTAINSORTBROWSERMAIN_H

#include <QString>
#include <QObject>
#include <QWebPage>
#include "mbcontroller.h"

class MyPage : public QWebPage {
    Q_OBJECT
public:
    MyPage();
    virtual ~MyPage() {};
    void setController(MBController *controller) {m_controller=controller;};
private slots:
    void slot_url_changed();
protected:
    virtual void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
private:
    MBController *m_controller;
};

#endif
