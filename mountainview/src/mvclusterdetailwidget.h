/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLUSTERDETAILWIDGET_H
#define MVCLUSTERDETAILWIDGET_H

#include "diskreadmda.h"
#include "mda.h"
#include <QWidget>
#include "mvabstractview.h"
#include "mvabstractviewfactory.h"

/** \class MVClusterDetailWidget
 *  \brief Display a view of each cluster -- mainly the template shapes and some stats
 *
 *  The user may click to change the current cluster, or use the Ctrl/Shift keys to select multiple clusters.
 */

class ClusterView;
class MVClusterDetailWidgetPrivate;
class MVClusterDetailWidget : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVClusterDetailWidgetPrivate;
    friend class ClusterView;
    MVClusterDetailWidget(MVViewAgent* view_agent, MVAbstractViewFactory* factory = 0);
    virtual ~MVClusterDetailWidget();

    MVAbstractViewFactory* viewFactory() const Q_DECL_OVERRIDE;

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

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
    void slot_export_image();
    void slot_toggle_stdev_shading();
    void slot_zoom_in();
    void slot_zoom_out();
    void slot_vertical_zoom_in();
    void slot_vertical_zoom_out();

private:
    MVClusterDetailWidgetPrivate* d;
};

class MVClusterDetailsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVClusterDetailsFactory(MVViewAgent* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // MVCLUSTERDETAILWIDGET_H
