/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVFIRINGEVENTVIEW2_H
#define MVFIRINGEVENTVIEW2_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvtimeseriesviewbase.h"

/// TODO (0.9.1) on first load, multiscale file is created on server, the process is detached. Provide feedback to the user somehow

class MVFiringEventView2Private;
class MVFiringEventView2 : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class MVFiringEventView2Private;
    MVFiringEventView2(MVViewAgent* view_agent);
    virtual ~MVFiringEventView2();

    void setLabelsToUse(const QSet<int> &labels_to_use);

    void paintContent(QPainter* painter);

    //void setTimeseries(const DiskReadMda& X);
    //void setMLProxyUrl(const QString& url);
    void setChannelColors(const QList<QColor>& colors);

    void setAmplitudeRange(MVRange range);
    void autoSetAmplitudeRange();

private slots:
    void slot_restart_calculation();
    void slot_computation_finished();

private:
    MVFiringEventView2Private* d;
};

#endif // MVFiringEventView2_H
