/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "mvfiringeventview2.h"
#include <math.h>

#include <QImageWriter>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <taskprogress.h>

/// TODO (MEDIUM) control brightness in firing event view

class MVFiringEventViewCalculator {
public:
    //input
    DiskReadMda firings;
    MVEventFilter filter;
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

    MVRange m_amplitude_range;
    QSet<int> m_labels_to_use;
    QVector<double> m_times0;
    QVector<int> m_labels0;
    QVector<double> m_amplitudes0;

    MVFiringEventViewCalculator m_calculator;

    double val2ypix(double val);
    double ypix2val(double ypix);
};

MVFiringEventView2::MVFiringEventView2(MVContext* view_agent)
    : MVTimeSeriesViewBase(view_agent)
{
    d = new MVFiringEventView2Private;
    d->q = this;

    d->m_amplitude_range = MVRange(0, 1);
    this->setMarkersVisible(false);
    this->setMargins(60, 60, 40, 40);

    mvtsv_prefs p = this->prefs();
    p.colors.axis_color = Qt::white;
    p.colors.text_color = Qt::white;
    p.colors.background_color = Qt::black;
    this->setPrefs(p);

    this->recalculateOn(view_agent, SIGNAL(filteredFiringsChanged()));

    this->recalculate();
}

MVFiringEventView2::~MVFiringEventView2()
{
    delete d;
}

void MVFiringEventView2::prepareCalculation()
{
    d->m_calculator.labels_to_use = d->m_labels_to_use;
    d->m_calculator.firings = viewAgent()->firings();
    d->m_calculator.filter = viewAgent()->eventFilter();
}

void MVFiringEventView2::runCalculation()
{
    d->m_calculator.compute();
}

void MVFiringEventView2::onCalculationFinished()
{
    d->m_labels0 = d->m_calculator.labels;
    d->m_times0 = d->m_calculator.times;
    d->m_amplitudes0 = d->m_calculator.amplitudes;
    /// TODO (MEDIUM) only do this if user has specified that it should be auto calculated (should be default)
    this->autoSetAmplitudeRange();
}

void MVFiringEventView2::setLabelsToUse(const QSet<int>& labels_to_use)
{
    d->m_labels_to_use = labels_to_use;
    this->recalculate();
}

void MVFiringEventView2::setAmplitudeRange(MVRange range)
{
    d->m_amplitude_range = range;
    update();
}

#include "msmisc.h"
#include "mvmainwindow.h"
void MVFiringEventView2::autoSetAmplitudeRange()
{
    double min0 = compute_min(d->m_amplitudes0);
    double max0 = compute_max(d->m_amplitudes0);
    setAmplitudeRange(MVRange(qMin(0.0, min0), qMax(0.0, max0)));
}

void MVFiringEventView2::paintContent(QPainter* painter)
{
    double alpha_pct = 0.7;
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
        opts.color = Qt::white;
        draw_axis(painter, opts);
    }

    //legend
    /// TODO (LOW) make general legend "widget"
    {
        double spacing = 6;
        double margin = 10;
        // it would still be better if m_labels.was presorted right from the start
        QList<int> list = d->m_labels_to_use.toList();
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

void MVFiringEventViewCalculator::compute()
{
    TaskProgress task("Computing firing events");

    firings = compute_filtered_firings_locally(firings, filter);

    long L = firings.N2();
    times.clear();
    labels.clear();
    amplitudes.clear();
    for (long i = 0; i < L; i++) {
        if (i % 100 == 0) {
            if (thread_interrupt_requested()) {
                task.error("Halted");
                return;
            }
        }
        int label0 = (int)firings.value(2, i);
        if (labels_to_use.contains(label0)) {

            task.setProgress(i * 1.0 / L);
            times << firings.value(1, i);
            labels << label0;
            amplitudes << firings.value(3, i);
        }
    }
    task.log(QString("Found %1 events, using %2 clusters").arg(times.count()).arg(labels_to_use.count()));
}

MVFiringEventsFactory::MVFiringEventsFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
    connect(context, SIGNAL(selectedClustersChanged()),
        this, SLOT(updateEnabled()));
    updateEnabled();
}

QString MVFiringEventsFactory::id() const
{
    return QStringLiteral("open-firing-events");
}

QString MVFiringEventsFactory::name() const
{
    return tr("Firing Events");
}

QString MVFiringEventsFactory::title() const
{
    return tr("Firing Events");
}

MVAbstractView* MVFiringEventsFactory::createView(QWidget* parent)
{
    QList<int> ks = mvContext()->selectedClusters();
    if (ks.isEmpty()) {
        QMessageBox::information(MVMainWindow::instance(), "Unable to open firing events", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    MVFiringEventView2* X = new MVFiringEventView2(mvContext());
    X->setLabelsToUse(ks.toSet());
    X->setNumTimepoints(mvContext()->currentTimeseries().N2());
    return X;
}

void MVFiringEventsFactory::updateEnabled()
{
    //bool has_peaks = (d->m_firings.value(0, 3) != 0); //for now we just test the very first one (might be problematic)
    /// TODO: (0.9.1) restore this has_peaks without accessing m_firings in gui thread
    bool has_peaks = true;
    setEnabled(!MVMainWindow::instance()->viewAgent()->selectedClusters().isEmpty() && has_peaks);
}
