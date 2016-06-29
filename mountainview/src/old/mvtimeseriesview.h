/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVTIMESERIESVIEW_H
#define MVTIMESERIESVIEW_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvcontext.h"

class MVTimeSeriesViewPrivate;
class MVTimeSeriesView : public QWidget {
    Q_OBJECT
public:
    friend class MVTimeSeriesViewPrivate;
    MVTimeSeriesView(MVContext* view_agent);
    virtual ~MVTimeSeriesView();

    void setSampleRate(double samplerate);
    void setTimeseries(const DiskReadMda& X);
    void setMLProxyUrl(const QString& url);
    void setTimesLabels(const QVector<double>& times, const QVector<int>& labels);

    void setTimeRange(MVRange);
    void setCurrentTimepoint(double t);
    void setSelectedTimeRange(MVRange range);
    void setAmplitudeFactor(double factor); // display range will be between -1/factor and 1/factor, but not clipped (thus channel plots may overlap)
    void autoSetAmplitudeFactor();
    void autoSetAmplitudeFactorWithinTimeRange();
    void setActivated(bool val);

    double currentTimepoint() const;
    MVRange timeRange() const;
    double amplitudeFactor() const;
    DiskReadMda timeseries();

    void resizeEvent(QResizeEvent* evt);
    void paintEvent(QPaintEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void wheelEvent(QWheelEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

    static void unit_test();

signals:
    void clicked();

private slots:
    void slot_scroll_to_current_timepoint();

private:
    MVTimeSeriesViewPrivate* d;
};

#endif // MVTIMESERIESVIEW_H
