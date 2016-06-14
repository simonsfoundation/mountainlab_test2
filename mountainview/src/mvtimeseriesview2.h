/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVTIMESERIESVIEW2_H
#define MVTIMESERIESVIEW2_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvviewagent.h"
#include "mvtimeseriesviewbase.h"

/// TODO (0.9.1) on first load, multiscale file is created on server, the process is detached. Provide feedback to the user somehow

class MVTimeSeriesView2Private;
class MVTimeSeriesView2 : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class MVTimeSeriesView2Private;
    MVTimeSeriesView2(MVViewAgent* view_agent);
    virtual ~MVTimeSeriesView2();

    void paintContent(QPainter* painter);

    void setTimeseries(const DiskReadMda& X);
    void setMLProxyUrl(const QString& url);

    void setAmplitudeFactor(double factor); // display range will be between -1/factor and 1/factor, but not clipped (thus channel plots may overlap)
    void autoSetAmplitudeFactor();
    void autoSetAmplitudeFactorWithinTimeRange();

    DiskReadMda timeseries();
    double amplitudeFactor() const;

    void resizeEvent(QResizeEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

private:
    MVTimeSeriesView2Private* d;
};

#endif // MVTIMESERIESVIEW2_H
