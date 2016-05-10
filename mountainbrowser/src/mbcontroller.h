/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef MBCONTROLLER_H
#define MBCONTROLLER_H

#include <QJsonObject>
#include <QObject>
#include <QString>

struct MBExperiment {
    QString exp_id;
    QJsonObject json;
};

class MBControllerPrivate;
class MBController : public QObject {
    Q_OBJECT
public:
    friend class MBControllerPrivate;
    MBController();
    virtual ~MBController();

    void setMountainBrowserUrl(const QString& url);
    void setMscmdServerUrl(const QString& url);
    void setMdaServerUrl(const QString& url);
    void setMPServerUrl(const QString &url);
    Q_INVOKABLE QString mountainBrowserUrl();
    Q_INVOKABLE QString mpServerUrl();

    Q_INVOKABLE QString getJson(QString url_or_path);
    Q_INVOKABLE QString getText(QString url_or_path);
    Q_INVOKABLE void openSortingResult(QString json);
private slots:
    void slot_ready_read();

private:
    MBControllerPrivate* d;
};

#endif // MBCONTROLLER_H
