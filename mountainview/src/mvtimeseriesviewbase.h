/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/13/2016
*******************************************************/

#ifndef MVTimeSeriesViewBaseBASE_H
#define MVTimeSeriesViewBaseBASE_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvabstractview.h"

/// TODO (0.9.1) on first load, multiscale file is created on server, the process is detached. Provide feedback to the user somehow

struct mvtsv_colors {
    mvtsv_colors()
    {
        marker_color = QColor(200, 0, 0, 120);
        text_color = Qt::black;
        axis_color = Qt::black;
        background_color = Qt::white;
    }

    QColor marker_color, text_color, axis_color, background_color;
};

class MVTimeSeriesViewBasePrivate;
class MVTimeSeriesViewBase : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVTimeSeriesViewBasePrivate;
    MVTimeSeriesViewBase(MVViewAgent* view_agent);
    virtual ~MVTimeSeriesViewBase();

    virtual void prepareCalculation() Q_DECL_OVERRIDE;
    virtual void runCalculation() Q_DECL_OVERRIDE;
    virtual void onCalculationFinished() Q_DECL_OVERRIDE;

    virtual void paintContent(QPainter* painter) = 0;
    void setNumTimepoints(long N); //called by subclass
    void setColors(mvtsv_colors colors);

    void setActivated(bool val);
    void setMarkersVisible(bool val);
    void setMargins(double mleft, double mright, double mtop, double mbottom);

    void setTimeRange(MVRange range);
    MVRange timeRange();
    void setSelectedTimeRange(MVRange range);

    double amplitudeFactor() const;

protected:
    void resizeEvent(QResizeEvent* evt);
    void paintEvent(QPaintEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void wheelEvent(QWheelEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

protected:
    QRectF contentGeometry();
    double time2xpix(double t) const;
    double xpix2time(double x) const;

signals:
    void clicked();

private slots:
    void slot_scroll_to_current_timepoint();

private:
    MVTimeSeriesViewBasePrivate* d;
};

#endif // MVTimeSeriesViewBaseBASE_H
