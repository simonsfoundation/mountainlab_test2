/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "mvtimeseriesview2.h"
#include "multiscaletimeseries.h"
#include "mvtimeseriesrendermanager.h"
#include <math.h>

#include <QImageWriter>
#include <QMouseEvent>
#include <QPainter>

struct mvtsv_channel {
    long channel;
    QString label;
    QRectF geometry;
};

class MVTimeSeriesView2Private {
public:
    MVTimeSeriesView2* q;
    MultiScaleTimeSeries m_msts;

    MVViewAgent* m_view_agent;

    double m_amplitude_factor;
    QList<mvtsv_channel> m_channels;
    bool m_layout_needed;
    bool m_time_range_has_been_initialized;

    MVTimeSeriesRenderManager m_render_manager;

    QList<mvtsv_channel> make_channel_layout(int M);
    void paint_channel_labels(QPainter* painter, double W, double H);

    double val2ypix(int m, double val);
    double ypix2val(int m, double ypix);
};

MVTimeSeriesView2::MVTimeSeriesView2(MVViewAgent* view_agent)
    : MVTimeSeriesViewBase(view_agent)
{
    d = new MVTimeSeriesView2Private;
    d->q = this;
    d->m_view_agent = view_agent;

    d->m_amplitude_factor = 1.0;
    d->m_render_manager.setMultiScaleTimeSeries(&d->m_msts);
    d->m_render_manager.setChannelColors(view_agent->channelColors());
    d->m_layout_needed = true;
    d->m_time_range_has_been_initialized = false;

    QObject::connect(&d->m_render_manager, SIGNAL(updated()), this, SLOT(update()));
    QObject::connect(d->m_view_agent, SIGNAL(currentTimeseriesChanged()), this, SLOT(slot_current_timeseries_changed()));

    slot_current_timeseries_changed();
}

MVTimeSeriesView2::~MVTimeSeriesView2()
{
    delete d;
}

double MVTimeSeriesView2::amplitudeFactor() const
{
    return d->m_amplitude_factor;
}

void MVTimeSeriesView2::resizeEvent(QResizeEvent* evt)
{
    d->m_layout_needed = true;
    MVTimeSeriesViewBase::resizeEvent(evt);
}

void MVTimeSeriesView2::setAmplitudeFactor(double factor)
{
    d->m_amplitude_factor = factor;
    update();
}

class AutoSetAmplitudeFactorThread : public QThread {
public:
    //input
    MultiScaleTimeSeries* msts;

    //output
    double min, max;
    void run()
    {
        min = msts->minimum();
        max = msts->maximum();
    }
};

void MVTimeSeriesView2::autoSetAmplitudeFactor()
{
    if (!in_gui_thread()) {
        qWarning() << "Can only call autoSetAmplitudeFactor in gui thread";
        return;
    }
    //we can't actually do this in the gui thread, which is where it will be called

    /// Witold we should be sure this thread is stopped in the rare case that the object is deleted while it is still running
    AutoSetAmplitudeFactorThread* thread = new AutoSetAmplitudeFactorThread;
    thread->msts = &d->m_msts;
    QObject::connect(thread, &AutoSetAmplitudeFactorThread::finished, [this, thread]() {
        double max_range = qMax(qAbs(d->m_msts.minimum()), qAbs(d->m_msts.maximum()));
        if (max_range) {
            this->setAmplitudeFactor(1.5 / max_range);
        }
        else {
            qWarning() << "Problem in autoSetAmplitudeFactor: range is null";
        }
    });
    thread->start();
}

void MVTimeSeriesView2::autoSetAmplitudeFactorWithinTimeRange()
{
    double min0 = d->m_render_manager.visibleMinimum();
    double max0 = d->m_render_manager.visibleMaximum();
    double factor = qMax(qAbs(min0), qAbs(max0));
    if (factor)
        this->setAmplitudeFactor(1.5 / factor);
}

void MVTimeSeriesView2::paintContent(QPainter* painter)
{
    // Geometry of channels
    DiskReadMda TS = d->m_view_agent->currentTimeseries();
    if (d->m_layout_needed) {
        int M = TS.N1();
        d->m_layout_needed = false;
        d->m_channels = d->make_channel_layout(M);
        for (long m = 0; m < M; m++) {
            mvtsv_channel* CH = &d->m_channels[m];
            CH->channel = m;
            CH->label = QString("%1").arg(m + 1);
        }
    }

    double WW = this->contentGeometry().width();
    double HH = this->contentGeometry().height();
    QImage img = d->m_render_manager.getImage(timeRange().min, timeRange().max, d->m_amplitude_factor, WW, HH);
    painter->drawImage(this->contentGeometry().left(), this->contentGeometry().top(), img);

    // Channel labels
    d->paint_channel_labels(painter, this->width(), this->height());
}

void MVTimeSeriesView2::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Up) {
        d->m_amplitude_factor *= 1.2;
        update();
    }
    else if (evt->key() == Qt::Key_Down) {
        d->m_amplitude_factor /= 1.2;
        update();
    }
    else {
        MVTimeSeriesViewBase::keyPressEvent(evt);
    }
}

void MVTimeSeriesView2::slot_current_timeseries_changed()
{
    DiskReadMda TS = d->m_view_agent->currentTimeseries();
    this->setNumTimepoints(TS.N2());
    if (!d->m_time_range_has_been_initialized) {
        this->setTimeRange(MVRange(0, TS.N2() - 1));
        d->m_time_range_has_been_initialized = true;
    }
    d->m_msts.setData(TS);
    d->m_layout_needed = true;

    this->autoSetAmplitudeFactor();
    update();
}

QList<mvtsv_channel> MVTimeSeriesView2Private::make_channel_layout(int M)
{
    QList<mvtsv_channel> channels;
    if (!M)
        return channels;
    QRectF RR = q->contentGeometry();
    double space = 0;
    double channel_height = (RR.height() - (M - 1) * space) / M;
    double y0 = RR.top();
    for (int m = 0; m < M; m++) {
        mvtsv_channel X;
        X.geometry = QRectF(RR.left(), y0, RR.width(), channel_height);
        channels << X;
        y0 += channel_height + space;
    }
    return channels;
}

void MVTimeSeriesView2Private::paint_channel_labels(QPainter* painter, double W, double H)
{
    Q_UNUSED(W)
    Q_UNUSED(H)
    QPen pen = painter->pen();
    pen.setColor(Qt::black);
    painter->setPen(pen);

    QFont font = painter->font();
    font.setPixelSize(13);
    painter->setFont(font);

    long M = m_view_agent->currentTimeseries().N1();
    for (int m = 0; m < M; m++) {
        double ypix = val2ypix(m, 0);
        QRectF rect(0, ypix - 30, q->contentGeometry().left() - 5, 60);
        QString str = QString("%1").arg(m + 1);
        painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, str);
    }
}

double MVTimeSeriesView2Private::val2ypix(int m, double val)
{
    if (m >= m_channels.count())
        return 0;

    mvtsv_channel* CH = &m_channels[m];

    double py = CH->geometry.y() + CH->geometry.height() / 2 + CH->geometry.height() / 2 * (val * m_amplitude_factor);
    return py;
}

double MVTimeSeriesView2Private::ypix2val(int m, double ypix)
{
    if (m >= m_channels.count())
        return 0;

    mvtsv_channel* CH = &m_channels[m];

    if (m_amplitude_factor) {
        double val = (ypix - (CH->geometry.y() + CH->geometry.height() / 2)) / m_amplitude_factor / (CH->geometry.height() / 2);
        return val;
    }
    else
        return 0;
}
