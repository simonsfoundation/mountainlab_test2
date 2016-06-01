/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/1/2016
*******************************************************/

#ifndef MVTIMESERIESRENDERMANAGER_H
#define MVTIMESERIESRENDERMANAGER_H

#include "multiscaletimeseries.h"

#include <QImage>
#include <QRunnable>
#include <QThread>

class MVTimeSeriesRenderManagerPrivate;
class MVTimeSeriesRenderManager : public QObject {
    Q_OBJECT
public:
    friend class MVTimeSeriesRenderManagerPrivate;
    MVTimeSeriesRenderManager();
    virtual ~MVTimeSeriesRenderManager();

    void setMultiScaleTimeSeries(MultiScaleTimeSeries* ts);

    QImage getImage(double t1, double t2, double amp_factor, double W, double H);

signals:
    void updated();

private
slots:
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
    MultiScaleTimeSeries* ts;

    //output
    QImage image;

    void run();
};

class ThreadManager : public QObject {
    Q_OBJECT
public:
    ThreadManager();
    void start(QThread* thread);
private
slots:
    void slot_timer();
    void slot_thread_finished();

private:
    QList<QThread*> m_queued_threads;
    QSet<QThread*> m_running_threads;
};

#endif // MVTIMESERIESRENDERMANAGER_H
