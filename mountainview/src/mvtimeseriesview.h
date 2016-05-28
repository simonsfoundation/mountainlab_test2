/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVTIMESERIESVIEW_H
#define MVTIMESERIESVIEW_H

#include <QWidget>
#include <diskreadmda.h>

class MVTimeSeriesViewPrivate;
class MVTimeSeriesView : public QWidget {
    Q_OBJECT
public:
    friend class MVTimeSeriesViewPrivate;
    MVTimeSeriesView();
    virtual ~MVTimeSeriesView();
    void setData(double t0, double t_step, const DiskReadMda& X);
    void setTimeRange(double t1, double t2);

private:
    MVTimeSeriesViewPrivate* d;
};

#endif // MVTIMESERIESVIEW_H
