
#ifndef MOUNTAINSORTBROWSERMAIN_H
#define MOUNTAINSORTBROWSERMAIN_H

#include <QString>
#include <QObject>

class MyLocalStudy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString json READ json WRITE setJson)
public:
    MyLocalStudy(const QString &json) {m_json=json;};
    QString json() {return m_json;}
    void setJson(const QString &str) {m_json=str;}
private:
    QString m_json;
};

#endif
