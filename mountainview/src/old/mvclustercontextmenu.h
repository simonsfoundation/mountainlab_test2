#ifndef MVCLUSTERCONTEXTMENU_H
#define MVCLUSTERCONTEXTMENU_H

#include <QMenu>
#include "mvmainwindow.h"
#include "mvcontext.h"

class MVClusterContextMenuPrivate;
class MVClusterContextMenu : public QMenu {
    Q_OBJECT
public:
    friend class MVClusterContextMenuPrivate;
    MVClusterContextMenu(MVContext* mvcontext, MVMainWindow* mw, const QSet<int>& cluster_numbers);
    virtual ~MVClusterContextMenu();

private slots:
    void slot_add_tag();
    void slot_remove_tag();
    void slot_clear_tags();
    void slot_open_cross_correlograms();
    void slot_open_matrix_of_cross_correlograms();
    void slot_merge_selected_clusters();
    void slot_unmerge_selected_clusters();

private:
    MVClusterContextMenuPrivate* d;
};

#endif // MVCLUSTERCONTEXTMENU_H
