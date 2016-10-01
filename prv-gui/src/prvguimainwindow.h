/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/
#ifndef PRVGUIMAINWINDOW_H
#define PRVGUIMAINWINDOW_H

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QWidget>
#include <QVariant>

#include "prvgui.h"

class PrvGuiMainWindowPrivate;
class PrvGuiMainWindow : public QWidget {
    Q_OBJECT
public:
    friend class PrvGuiMainWindowPrivate;
    PrvGuiMainWindow();
    virtual ~PrvGuiMainWindow();
    void setPrvs(const QList<PrvRecord>& prvs);
    void setServerNames(QStringList names);
    void refresh();
private slots:

protected:
    void resizeEvent(QResizeEvent* evt);

private:
    PrvGuiMainWindowPrivate* d;
};

#endif // PRVGUIMAINWINDOW_H
