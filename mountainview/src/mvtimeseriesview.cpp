/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "mvtimeseriesview.h"

class MVTimeSeriesViewPrivate {
public:
    MVTimeSeriesView* q;
    DiskReadMda m_data;
    double m_t0;
    double m_t_step;
    double m_t1, m_t2;
};

MVTimeSeriesView::MVTimeSeriesView()
{
    d = new MVTimeSeriesViewPrivate;
    d->q = this;
}

MVTimeSeriesView::~MVTimeSeriesView()
{
    delete d;
}

void MVTimeSeriesView::setData(double t0, double t_step, const DiskReadMda& X)
{
    d->m_t0 = t0;
    d->m_t_step = t_step;
    update();
}

void MVTimeSeriesView::setTimeRange(double t1, double t2)
{
    d->m_t1 = t1;
    d->m_t2 = t2;
    update();
}
