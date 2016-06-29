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
#include "mvfile.h"
#include "mvutils.h"
#include "mvviewagent.h"
#include "mvabstractcontrol.h"

class MVAbstractViewFactory;
class TabberTabWidget;
class Tabber;
class MVAbstractContextMenuHandler;

/** \class MVMainWindow
 *  \brief The main window (for now) showing an overview of the results of sorting by providing a number of interactive and synchronized views.
 *
 *  Presents user with a rich set of views. Cross-correlograms, raw data, cluster details, rotatable 3D views, firing rate vs. time view, etc.
 */

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
    MVMainWindow(MVViewAgent* view_agent, QWidget* parent = 0);
    virtual ~MVMainWindow();
    void setDefaultInitialization();

    void setMVFile(MVFile mv_file);
    MVFile getMVFile();

    void registerViewFactory(MVAbstractViewFactory* f);
    void unregisterViewFactory(MVAbstractViewFactory* f);
    const QList<MVAbstractViewFactory*>& viewFactories() const;

    void registerContextMenuHandler(MVAbstractContextMenuHandler* h);
    void unregisterContextMenuHandler(MVAbstractContextMenuHandler* h);
    const QList<MVAbstractContextMenuHandler*>& contextMenuHandlers() const;

    void addControl(MVAbstractControl* control, bool start_expanded);

    /// Witold, I am going to use this sparingly, thinking it will eventually go away
    static MVMainWindow* instance(); // helper while implementing view factories

    TabberTabWidget* tabWidget(QWidget* w) const; //should disappear
    Tabber* tabber() const; //should disappear

    MVViewAgent* viewAgent() const;

public slots:
    void openView(const QString& id);
    void recalculateViews(RecalculateViewsMode mode);

protected:
    void resizeEvent(QResizeEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

signals:

public slots:

private slots:
    //void slot_control_panel_user_action(QString str);
    //void slot_auto_correlogram_activated();
    //void slot_amplitude_histogram_activated();
    //void slot_templates_clicked();
    //void slot_cross_correlogram_computer_finished();
    //void slot_update_buttons();
    //void slot_calculator_finished();
    void slot_action_move_to_other_tab_widget();
    void slot_pop_out_widget();
    void slot_cluster_annotation_guide();

    void slot_open_view(QObject*);
    void slot_open_cluster_context_menu();
    void handleContextMenu(const QMimeData& dt, const QPoint& globalPos);

private:
    MVMainWindowPrivate* d;
    static MVMainWindow* window_instance;
};

/*
class CustomTabWidget : public QTabWidget {
	Q_OBJECT
public:
    MVMainWindow *q;
    CustomTabWidget(MVMainWindow *q);
protected:
	void mousePressEvent(QMouseEvent *evt);
private slots:
	void slot_tab_close_requested(int num);
	void slot_tab_bar_clicked();
	void slot_tab_bar_double_clicked();
	void slot_switch_to_other_tab_widget();
};
*/

#endif // MVMAINWINDOW_H
