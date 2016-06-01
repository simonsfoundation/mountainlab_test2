/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/1/2016
*******************************************************/

#ifndef MVTIMESERIESRENDERMANAGER_H
#define MVTIMESERIESRENDERMANAGER_H

#include "multiscaletimeseries.h"

#include <QImage>
#include <QThread>

struct MVTimeSeriesRenderManagerPrefs {
    double margins[4];
    double space_between_channels;
    MVTimeSeriesRenderManagerPrefs() {
        margins[0]=margins[1]=margins[2]=margins[3]=30;
        space_between_channels=8;
    }
};

class MVTimeSeriesRenderManagerPrivate;
class MVTimeSeriesRenderManager : public QObject {
    Q_OBJECT
public:
    friend class MVTimeSeriesRenderManagerPrivate;
    MVTimeSeriesRenderManager();
    virtual ~MVTimeSeriesRenderManager();

    void setMultiScaleTimeSeries(MultiScaleTimeSeries* ts);
    void setPrefs(MVTimeSeriesRenderManagerPrefs prefs);

    QImage getImage(double t1, double t2, double amp_factor, double W, double H);

signals:
    void updated();

private slots:
    void slot_thread_finished();

private:
    MVTimeSeriesRenderManagerPrivate* d;
};

class MVTimeSeriesRenderManagerThread : public QThread {
    Q_OBJECT
public:
    //input
    double t1, t2, amp_factor;
    double W, H;
    long data_ds_factor;
    MVTimeSeriesRenderManagerPrefs prefs;
    MultiScaleTimeSeries *ts;

    //output
    QImage image;

    void run();
};


#endif // MVTIMESERIESRENDERMANAGER_H
