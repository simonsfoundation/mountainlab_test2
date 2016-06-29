/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVMAINWINDOW_H
#define MVMAINWINDOW_H

#include <QWidget>
#include "mda.h"
#include <QMimeData>
#include <QTabWidget>
#include "clustermerge.h"
#include "mvutils.h"
#include "mvcontext.h"
#include "mvabstractcontrol.h"

class MVAbstractViewFactory;
class TabberTabWidget;
class Tabber;
class MVAbstractContextMenuHandler;

/// Witold, please help do this a better way
enum RecalculateViewsMode {
    All,
    AllVisible,
    Suggested,
    SuggestedVisible
};

class MVMainWindowPrivate;
class MVMainWindow : public QWidget {
    Q_OBJECT
public:
    friend class MVMainWindowPrivate;
    MVMainWindow(MVContext* context, QWidget* parent = 0);
    virtual ~MVMainWindow();
    void setDefaultInitialization();

    void registerViewFactory(MVAbstractViewFactory* f);
    void unregisterViewFactory(MVAbstractViewFactory* f);
    const QList<MVAbstractViewFactory*>& viewFactories() const;

    void registerContextMenuHandler(MVAbstractContextMenuHandler* h);
    void unregisterContextMenuHandler(MVAbstractContextMenuHandler* h);
    const QList<MVAbstractContextMenuHandler*>& contextMenuHandlers() const;

    void addControl(MVAbstractControl* control, bool start_expanded);

    MVContext* mvContext() const;

public slots:
    void openView(const QString& id);
    void recalculateViews(RecalculateViewsMode mode);

protected:
    void resizeEvent(QResizeEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

signals:

public slots:

private slots:
    void slot_action_move_to_other_tab_widget();
    void slot_pop_out_widget();
    /// TODO cluster annotation guide doesn't belong in main window
    void slot_cluster_annotation_guide();

    void slot_open_view(QObject*);
    void handleContextMenu(const QMimeData& dt, const QPoint& globalPos);

private:
    MVMainWindowPrivate* d;
};

#endif // MVMAINWINDOW_H
