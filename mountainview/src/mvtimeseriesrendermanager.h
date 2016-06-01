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

struct MVTimeSeriesRenderManagerInfo {
    double W, H;
    double t1, t2;
    QString make_code();
};

class MVTimeSeriesRenderManagerPrivate;
class MVTimeSeriesRenderManager : public QObject {
    Q_OBJECT
public:
    friend class MVTimeSeriesRenderManagerPrivate;
    MVTimeSeriesRenderManager();
    virtual ~MVTimeSeriesRenderManager();

    void setMultiScaleTimeSeries(MultiScaleTimeSeries* ts);
    void setMargins(double left, double right, double top, double bottom);
    void setSpaceBetweenChannels(double pix);

    void setInfo(MVTimeSeriesRenderManagerInfo info);
    void getImage(QImage& img0);

signals:
    void updated();

private slots:
    void slot_update_thread_finished();

private:
    MVTimeSeriesRenderManagerPrivate* d;
};

class MVTimeSeriesRenderManagerUpdateThread : public QThread {
    Q_OBJECT
public:
    //input
    MVTimeSeriesRenderManagerInfo info;
    int ds_x;
    int ds_y;

    //output
    QImage image;

    void run();
};

void MVTimeSeriesRenderManagerUpdateThread::run()
{

}

#endif // MVTIMESERIESRENDERMANAGER_H
