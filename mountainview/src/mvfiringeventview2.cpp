/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "mvfiringeventview2.h"
#include <math.h>

#include <QImageWriter>
#include <QMouseEvent>
#include <QPainter>
#include <taskprogress.h>
#include "computationthread.h"

class MVFiringEventViewCalculator : public ComputationThread {
public:
    //input
    DiskReadMda firings;
    QSet<int> labels_to_use;

    //output
    QVector<double> times;
    QVector<int> labels;
    QVector<double> amplitudes;

    void compute();
};

class MVFiringEventView2Private {
public:
    MVFiringEventView2* q;
    //DiskReadMda m_timeseries;

    MVViewAgent *m_view_agent;
    MVRange m_amplitude_range;
    QList<QColor> m_channel_colors;
    QSet<int> m_labels_to_use;
    QVector<double> m_times0;
    QVector<int> m_labels0;
    QVector<double> m_amplitudes0;

    MVFiringEventViewCalculator m_calculator;

    double val2ypix(double val);
    double ypix2val(double ypix);

    void start_computation();
};

MVFiringEventView2::MVFiringEventView2(MVViewAgent* view_agent)
    : MVTimeSeriesViewBase(view_agent)
{
    d = new MVFiringEventView2Private;
    d->q = this;

    d->m_view_agent=view_agent;

    d->m_amplitude_range = MVRange(0, 1);
    this->setMarkersVisible(false);
    this->setMargins(60, 60, 40, 40);

    QObject::connect(d->m_view_agent,SIGNAL(firingsChanged()),this,SLOT(slot_restart_calculation()));
    QObject::connect(&d->m_calculator,SIGNAL(computationFinished()),this,SLOT(slot_computation_finished()));

    d->start_computation();
}

MVFiringEventView2::~MVFiringEventView2()
{
    delete d;
}

void MVFiringEventView2::setLabelsToUse(const QSet<int> &labels_to_use)
{
    d->m_labels_to_use=labels_to_use;
    d->start_computation();
}

void MVFiringEventView2::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
    update();
}

void MVFiringEventView2::setAmplitudeRange(MVRange range)
{
    d->m_amplitude_range = range;
    update();
}

#include "computationthread.h"
#include "msmisc.h"
void MVFiringEventView2::autoSetAmplitudeRange()
{
    double min0 = compute_min(d->m_amplitudes0);
    double max0 = compute_max(d->m_amplitudes0);
    setAmplitudeRange(MVRange(qMin(0.0, min0), qMax(0.0, max0)));
}

void MVFiringEventView2::slot_restart_calculation()
{
    d->start_computation();
}

void MVFiringEventView2::slot_computation_finished()
{
    d->m_calculator.stopComputation(); //because i'm paranoid
    d->m_labels0=d->m_calculator.labels;
    d->m_times0=d->m_calculator.times;
    d->m_amplitudes0=d->m_calculator.amplitudes;
    /// TODO only do this if user has specified that it should be auto calculated (should be default)
    this->autoSetAmplitudeRange();
    update();
}

void MVFiringEventView2::paintContent(QPainter* painter)
{
    double alpha_pct = 0.3;
    for (long i = 0; i < d->m_times0.count(); i++) {
        double t0 = d->m_times0.value(i);
        int k0 = d->m_labels0.value(i);
        QColor col = viewAgent()->clusterColor(k0);
        col.setAlpha((int)(alpha_pct * 255));
        QPen pen = painter->pen();
        pen.setColor(col);
        painter->setPen(pen);
        double amp0 = d->m_amplitudes0.value(i);
        double xpix = this->time2xpix(t0);
        double ypix = d->val2ypix(amp0);
        painter->drawEllipse(xpix, ypix, 3, 3);
    }

    //axis
    {
        draw_axis_opts opts;
        opts.minval = d->m_amplitude_range.min;
        opts.maxval = d->m_amplitude_range.max;
        opts.orientation = Qt::Vertical;
        opts.pt1 = contentGeometry().bottomLeft() + QPointF(-3, 0);
        opts.pt2 = contentGeometry().topLeft() + QPointF(-3, 0);
        opts.tick_length = 5;
        draw_axis(painter, opts);
    }

    //legend
    /// TODO make general legend "widget"
    {
        double spacing = 6;
        double margin = 10;
        // it would still be better if m_labels.was presorted right from the start
        QList<int> list=d->m_labels_to_use.toList();
        qSort(list);
        double text_height = qBound(12.0, width() * 1.0 / 10, 25.0);
        double y0 = margin;
        QFont font = painter->font();
        font.setPixelSize(text_height - 1);
        painter->setFont(font);
        for (int i = 0; i < list.count(); i++) {
            QRectF rect(0, y0, width() - margin, text_height);
            QString str = QString("%1").arg(list[i]);
            QPen pen = painter->pen();
            pen.setColor(viewAgent()->clusterColor(list[i]));
            painter->setPen(pen);
            painter->drawText(rect, Qt::AlignRight, str);
            y0 += text_height + spacing;
        }
    }
}

double MVFiringEventView2Private::val2ypix(double val)
{
    if (m_amplitude_range.min >= m_amplitude_range.max)
        return 0;
    double pcty = (val - m_amplitude_range.min) / (m_amplitude_range.max - m_amplitude_range.min);

    return q->contentGeometry().top() + (1 - pcty) * q->contentGeometry().height();
}

double MVFiringEventView2Private::ypix2val(double ypix)
{
    if (m_amplitude_range.min >= m_amplitude_range.max)
        return 0;

    double pcty = 1 - (ypix - q->contentGeometry().top()) / q->contentGeometry().height();
    return m_amplitude_range.min + pcty * (m_amplitude_range.max - m_amplitude_range.min);
}

void MVFiringEventView2Private::start_computation()
{
    m_calculator.stopComputation();
    m_calculator.labels_to_use=m_labels_to_use;
    m_calculator.firings=m_view_agent->firings();
    m_calculator.startComputation();
}

void MVFiringEventViewCalculator::compute()
{
    TaskProgress task("Computing firing events");
    long L=firings.N2();
    times.clear();
    labels.clear();
    amplitudes.clear();
    for (long i=0; i<L; i++) {
        if (this->isInterruptionRequested()) return;
        int label0=(int)firings.value(2,i);
        if (labels_to_use.contains(label0)) {
            task.setProgress(i*1.0/L);
            times << firings.value(1,i);
            labels << label0;
            amplitudes << firings.value(3,i);
        }
    }
    task.log(QString("Found %1 events, using %2 clusters").arg(times.count()).arg(labels_to_use.count()));
}
