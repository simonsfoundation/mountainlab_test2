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
class MBController : public QObject
{
    Q_OBJECT
public:
    friend class MBControllerPrivate;
    MBController();
    virtual ~MBController();

    Q_INVOKABLE QString loadLocalStudy(QString file_path);
    Q_INVOKABLE QString loadRemoteStudy(QString url);
    Q_INVOKABLE void openSortingResult(QString json);
private:
    MBControllerPrivate *d;
};

#endif // MBCONTROLLER_H

