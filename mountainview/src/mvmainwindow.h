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
    void setEpochs(const QList<Epoch>& epochs);
    void setMLProxyUrl(const QString& url);
    void setClusterMerge(ClusterMerge CM);
    void setChannelColors(const QList<QColor>& colors);
    void setLabelColors(const QList<QColor>& colors);

    void setMVFile(MVFile mv_file);

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
    //void slot_templates_clicked();
    void slot_details_template_activated();
    //void slot_cross_correlogram_computer_finished();
    void slot_update_buttons();
    void slot_calculator_finished();

private:
    MVMainWindowPrivate* d;
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
