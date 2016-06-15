/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLUSTERDETAILWIDGET_H
#define MVCLUSTERDETAILWIDGET_H

#include "diskreadmda.h"
#include "mda.h"
#include <QWidget>
#include "mvviewagent.h"

/** \class MVClusterDetailWidget
 *  \brief Display a view of each cluster -- mainly the template shapes and some stats
 *
 *  The user may click to change the current cluster, or use the Ctrl/Shift keys to select multiple clusters.
 */

class MVClusterDetailWidgetPrivate;
class MVClusterDetailWidget : public QWidget {
    Q_OBJECT
public:
    friend class MVClusterDetailWidgetPrivate;
    MVClusterDetailWidget(MVViewAgent* view_agent, QWidget* parent = 0);
    virtual ~MVClusterDetailWidget();

    void zoomAllTheWayOut();

    ///Create an image of the current view
    QImage renderImage(int W = 0, int H = 0);

protected:
    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void mouseDoubleClickEvent(QMouseEvent* evt);
    void wheelEvent(QWheelEvent* evt);
signals:
    ///A cluster has been double-clicked (or enter pressed?)
    void signalTemplateActivated();
private slots:
    //void slot_context_menu(const QPoint& pos);
    void slot_calculator_finished();
    void slot_export_image();
    void slot_toggle_stdev_shading();
    void slot_recalculate();
    void slot_view_agent_option_changed(QString name);

private:
    MVClusterDetailWidgetPrivate* d;
};

#endif // MVCLUSTERDETAILWIDGET_H
