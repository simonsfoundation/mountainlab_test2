/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/
#ifndef PRVGUITREEWIDGET_H
#define PRVGUITREEWIDGET_H

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QWidget>
#include <QVariant>
#include <QTreeWidget>

#include "prvgui.h"

class PrvGuiTreeWidgetPrivate;
class PrvGuiTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    friend class PrvGuiTreeWidgetPrivate;
    PrvGuiTreeWidget();
    virtual ~PrvGuiTreeWidget();
    void setPrvs(const QList<PrvRecord>& prvs);
    void setServerNames(QStringList names);
private slots:
    void slot_update_tree_data();

private:
    PrvGuiTreeWidgetPrivate* d;
};

#endif // PRVGUITREEWIDGET_H
