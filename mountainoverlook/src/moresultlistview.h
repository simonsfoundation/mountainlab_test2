#ifndef MORESULTLISTVIEW_H
#define MORESULTLISTVIEW_H

#include <QWidget>
#include "mofile.h"
#include <QTreeWidgetItem>

class MOResultListViewPrivate;
class MOResultListView : public QWidget {
    Q_OBJECT
public:
    friend class MOResultListViewPrivate;
    MOResultListView(MOFile* mof);
    virtual ~MOResultListView();
signals:
    void resultActivated(QString name);
private slots:
    void slot_refresh();
    void slot_item_activated(QTreeWidgetItem* it);

private:
    MOResultListViewPrivate* d;
};

#endif // MORESULTLISTVIEW_H
