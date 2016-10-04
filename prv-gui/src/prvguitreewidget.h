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
    QList<PrvRecord> prvs() const;
    void setServerNames(QStringList names);
    void replacePrv(QString original_path, const PrvRecord& prv_new);
    void refresh();
    bool isDirty() const;
    void setDirty(bool val);
    QList<PrvRecord> selectedPrvs();
    QStringList serverNames() const;
    QVariantMap currentItemDetails() const; //not a great way to do this
signals:
    void dirtyChanged();
private slots:
    void slot_update_tree_data();

private:
    PrvGuiTreeWidgetPrivate* d;
};

#endif // PRVGUITREEWIDGET_H
