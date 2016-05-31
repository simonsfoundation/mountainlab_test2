/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MULTISCALEMDA_H
#define MULTISCALEMDA_H

#include "diskreadmda.h"


class MultiScaleTimeSeriesPrivate;
class MultiScaleTimeSeries
{
public:
    friend class MultiScaleTimeSeriesPrivate;
    MultiScaleTimeSeries();
    virtual ~MultiScaleTimeSeries();
    void setData(const DiskReadMda &X);

    bool getData(Mda &min, Mda &max, long t1, long t2, long ds_factor); //returns values at timepoints i1*ds_factor:ds_factor:i2*ds_factor

    static bool unit_test(long M=20, long N=1000);
    static long smallest_power_of_3_larger_than(long N);

private:
    MultiScaleTimeSeriesPrivate *d;
};

#endif // MULTISCALEMDA_H

