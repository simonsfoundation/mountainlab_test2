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
#include "mvabstractview.h"

class MVAbstractViewFactory;
class TabberTabWidget;
class Tabber;
class MVAbstractContextMenuHandler;

class MVMainWindowPrivate;
class MVMainWindow : public QWidget {
    Q_OBJECT
public:
    //Witold, is this the right place to put it?
    enum RecalculateViewsMode {
        All,
        AllVisible,
        Suggested,
        SuggestedVisible
    };

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

    void setCurrentContainerName(const QString& name);

    QList<MVAbstractView*> allViews();
    QJsonObject exportStaticViews() const;

    MVContext* mvContext() const;

signals:
    void viewsChanged();
    void signalExportMVFile();
    void signalExportFiringsFile();
    void signalExportClusterAnnotationFile();
    void signalExportStaticViews();
    void signalShareViewsOnWeb();

public slots:
    void closeAllViews();
    void openView(const QString& id);
    void recalculateViews(RecalculateViewsMode mode);
    void extractSelectedClusters();

protected:
    void resizeEvent(QResizeEvent* evt);

signals:

public slots:

private slots:
    void slot_pop_out_widget();
    /// TODO: (HIGH) cluster annotation guide doesn't belong in main window
    void slot_cluster_annotation_guide();
    void slot_guide_v1();
    void slot_guide_v2();

    void slot_open_view(QObject*);
    void handleContextMenu(const QMimeData& dt, const QPoint& globalPos);

private:
    MVMainWindowPrivate* d;
};

#endif // MVMAINWINDOW_H
