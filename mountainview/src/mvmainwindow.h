/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVMAINWINDOW_H
#define MVMAINWINDOW_H

#include <QWidget>
#include "mda.h"
#include <QTabWidget>
#include "clustermerge.h"
#include "mvfile.h"
#include "mvutils.h"
#include "mvviewagent.h"

class MVAbstractViewFactory;
class TabberTabWidget;
class Tabber;

/** \class MVMainWindow
 *  \brief The main window (for now) showing an overview of the results of sorting by providing a number of interactive and synchronized views.
 *
 *  Presents user with a rich set of views. Cross-correlograms, raw data, cluster details, rotatable 3D views, firing rate vs. time view, etc.
 */

class MVMainWindowPrivate;
class MVMainWindow : public QWidget {
    Q_OBJECT
public:
    friend class MVMainWindowPrivate;
    MVMainWindow(MVViewAgent* view_agent, QWidget* parent = 0);
    virtual ~MVMainWindow();
    void setCurrentTimeseriesName(const QString& name);
    void setDefaultInitialization();
    void setEpochs(const QList<Epoch>& epochs); //put in view agent
    void setClusterMerge(ClusterMerge CM); //put in view agent

    void setMVFile(MVFile mv_file);
    MVFile getMVFile();
    void registerViewFactory(MVAbstractViewFactory *f);
    void unregisterViewFactory(MVAbstractViewFactory *f);
    const QList<MVAbstractViewFactory *> &viewFactories() const;

    void applyUserAction(QString action);

    static MVMainWindow* instance(); // helper while implementing view factories
    TabberTabWidget* tabWidget(QWidget *w) const;
    Tabber* tabber() const;
    void openView(const QString &id);
    MVViewAgent* viewAgent() const;

    const QList<MVAbstractViewFactory*>& viewFactories() const;

    static MVMainWindow* instance(); // helper while implementing view factories
    TabberTabWidget* tabWidget(QWidget *w) const;
    Tabber* tabber() const;
    void openView(const QString &id);
    MVViewAgent* viewAgent() const;

protected:
    void resizeEvent(QResizeEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

signals:

public
slots:

private
slots:
    void slot_control_panel_user_action(QString str);
    void slot_auto_correlogram_activated();
    void slot_amplitude_histogram_activated();
    //void slot_templates_clicked();
#if 0
    void slot_details_template_activated();
#endif
    //void slot_cross_correlogram_computer_finished();
    void slot_update_buttons();
    //void slot_calculator_finished();
    void slot_action_move_to_other_tab_widget();
    void slot_pop_out_widget();
    void slot_cluster_annotation_guide();

    void slot_open_view(QObject*);

private:
    MVMainWindowPrivate* d;
    static MVMainWindow *window_instance;
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
