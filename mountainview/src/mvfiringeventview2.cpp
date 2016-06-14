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

class MVFiringEventView2Private {
public:
    MVFiringEventView2* q;
    //DiskReadMda m_timeseries;

    MVRange m_amplitude_range;
    QList<QColor> m_channel_colors;
    QVector<double> m_amplitudes;

    double val2ypix(double val);
    double ypix2val(double ypix);
};

MVFiringEventView2::MVFiringEventView2(MVViewAgent* view_agent)
    : MVTimeSeriesViewBase(view_agent)
{
    d = new MVFiringEventView2Private;
    d->q = this;
    d->m_amplitude_range = MVRange(0, 1);
    this->setMarkersVisible(false);
    this->setMargins(60, 60, 40, 40);
}

MVFiringEventView2::~MVFiringEventView2()
{
    delete d;
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

#include "msmisc.h"
void MVFiringEventView2::autoSetAmplitudeRange()
{
    double min0 = compute_min(d->m_amplitudes);
    double max0 = compute_max(d->m_amplitudes);
    setAmplitudeRange(MVRange(qMin(0.0, min0), qMax(0.0, max0)));
}

void MVFiringEventView2::paintContent(QPainter* painter)
{
    double alpha_pct = 0.3;
    QVector<double> times0 = this->times();
    QVector<int> labels0 = this->labels();
    for (long i = 0; i < times0.count(); i++) {
        double t0 = times0.value(i);
        int k0 = labels0.value(i);
        QColor col = viewAgent()->clusterColor(k0);
        col.setAlpha((int)(alpha_pct * 255));
        QPen pen = painter->pen();
        pen.setColor(col);
        painter->setPen(pen);
        double amp0 = d->m_amplitudes.value(i);
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
        QList<int> list;
        for (long i = 0; i < labels0.count(); i++) {
            const int value = labels0.at(i);
            QList<int>::iterator iter = qLowerBound(list.begin(), list.end(), value);
            if (iter == list.end() || *iter != value) {
                list.insert(iter, value);
            }
        }
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

void MVFiringEventView2::setAmplitudes(const QVector<double>& amplitudes)
{
    d->m_amplitudes = amplitudes;
    update();
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
