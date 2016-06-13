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
    MVTimeSeriesViewBasePrivate* d;
};

#endif // MVTimeSeriesViewBaseBASE_H
