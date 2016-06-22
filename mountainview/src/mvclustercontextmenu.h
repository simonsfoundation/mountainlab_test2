#ifndef MVCLUSTERCONTEXTMENU_H
#define MVCLUSTERCONTEXTMENU_H

#include <QMenu>
#include "mvviewagent.h"

class MVClusterContextMenuPrivate;
class MVClusterContextMenu : public QMenu {
    Q_OBJECT
public:
    friend class MVClusterContextMenuPrivate;
    MVClusterContextMenu(MVContext* mvcontext, QSet<int> cluster_numbers);
    virtual ~MVClusterContextMenu();

private
slots:
    void slot_add_tag();
    void slot_remove_tag();
    void slot_clear_tags();

private:
    MVClusterContextMenuPrivate* d;
};

#endif // MVCLUSTERCONTEXTMENU_H
