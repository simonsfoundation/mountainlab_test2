/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/24/2016
*******************************************************/

#ifndef RUN_MOUNTAINVIEW_SCRIPT_H
#define RUN_MOUNTAINVIEW_SCRIPT_H

#include "mvoverview2widget.h"

#include <QString>
#include <QMap>
#include <QVariant>
#include <QJsonObject>

int run_mountainview_script(const QString& script, QMap<QString, QVariant>& params);

class MVController : public QObject {
    Q_OBJECT
public:
    MVController()
    {
    }
    virtual ~MVController()
    {
    }

    //Q_INVOKABLE QWidget* createOverview2Widget();
    Q_INVOKABLE QImage createTemplatesImage(QString timeseries,QString firings,QJsonObject object);
    Q_INVOKABLE void writeImage(const QImage& img, const QString& fname);
    Q_INVOKABLE void appExec();
};

#endif // RUN_MOUNTAINVIEW_SCRIPT_H
