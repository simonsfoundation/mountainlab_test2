/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/13/2016
*******************************************************/

#ifndef MVTimeSeriesViewBaseBASE_H
#define MVTimeSeriesViewBaseBASE_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvviewagent.h"

/// TODO (0.9.1) on first load, multiscale file is created on server, the process is detached. Provide feedback to the user somehow

class MVTimeSeriesViewBasePrivate;
class MVTimeSeriesViewBase : public QWidget {
    Q_OBJECT
public:
    friend class MVTimeSeriesViewBasePrivate;
    MVTimeSeriesViewBase(MVViewAgent* view_agent);
    virtual ~MVTimeSeriesViewBase();

    virtual void paintContent(QPainter* painter) = 0;

    /// TODO get rid of setTimesLabels
    void setTimesLabels(const QVector<double>& times, const QVector<int>& labels);
    void setNumTimepoints(long N);

    void setTimeRange(MVRange);
    void setCurrentTimepoint(double t);
    void setSelectedTimeRange(MVRange range);
    void setActivated(bool val);
    void setMarkersVisible(bool val);
    void setMargins(double mleft, double mright, double mtop, double mbottom);

    double currentTimepoint() const;
    MVRange timeRange() const;
    double amplitudeFactor() const;
    MVViewAgent* viewAgent();

    void resizeEvent(QResizeEvent* evt);
    void paintEvent(QPaintEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void wheelEvent(QWheelEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

    static void unit_test();

protected:
    QRectF contentGeometry();
    QVector<double> times() const;
    QVector<int> labels() const;
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
