/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVTIMESERIESVIEW_H
#define MVTIMESERIESVIEW_H

#include <QWidget>
#include <diskreadmda.h>

/// Witold is there a Qt struct that captures this?
struct MVRange {
    MVRange(double min0 = 0, double max0 = 1)
    {
        min = min0;
        max = max0;
    }
    bool operator==(const MVRange &other);
    MVRange operator+(double offset);
    MVRange operator*(double scale);
    double min, max;
};

class MVTimeSeriesViewPrivate;
class MVTimeSeriesView : public QWidget {
    Q_OBJECT
public:
    friend class MVTimeSeriesViewPrivate;
    MVTimeSeriesView();
    virtual ~MVTimeSeriesView();

    void setData(double t0, const DiskReadMda& X);
    void setTimeRange(MVRange);
    void setCurrentTimepoint(double t);
    void setSelectedTimeRange(MVRange range);

    double currentTimepoint() const;
    MVRange timeRange() const;

    void resizeEvent(QResizeEvent *evt);
    void paintEvent(QPaintEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent *evt);
    void wheelEvent(QWheelEvent *evt);

    static void unit_test();

private:
    MVTimeSeriesViewPrivate* d;
};

#endif // MVTIMESERIESVIEW_H
