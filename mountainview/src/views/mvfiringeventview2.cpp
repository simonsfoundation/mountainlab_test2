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
#include "mvclusterlegend.h"
#include "paintlayerstack.h"

/// TODO: (MEDIUM) control brightness in firing event view

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

class FiringEventAxisLayer : public PaintLayer {
public:
    void paint(QPainter* painter) Q_DECL_OVERRIDE;

    QRectF content_geometry;
    MVRange amplitude_range;
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

    MVClusterLegend* m_legend;

    MVFiringEventViewCalculator m_calculator;

    PaintLayerStack m_paint_layer_stack;
    FiringEventAxisLayer* m_axis_layer;

    double val2ypix(double val);
    double ypix2val(double ypix);
};

MVFiringEventView2::MVFiringEventView2(MVContext* context)
    : MVTimeSeriesViewBase(context)
{
    d = new MVFiringEventView2Private;
    d->q = this;

    this->setMouseTracking(true);

    d->m_legend = new MVClusterLegend;
    d->m_legend->setClusterColors(context->clusterColors());

    d->m_axis_layer = new FiringEventAxisLayer;
    d->m_paint_layer_stack.addLayer(d->m_axis_layer);
    d->m_paint_layer_stack.addLayer(d->m_legend);
    connect(&d->m_paint_layer_stack, SIGNAL(repaintNeeded()), this, SLOT(update()));

    d->m_amplitude_range = MVRange(0, 1);
    this->setMarkersVisible(false);
    this->setMargins(60, 60, 40, 40);

    mvtsv_prefs p = this->prefs();
    p.colors.axis_color = Qt::white;
    p.colors.text_color = Qt::white;
    p.colors.background_color = Qt::black;
    this->setPrefs(p);

    this->recalculateOn(context, SIGNAL(filteredFiringsChanged()));

    this->recalculate();
}

MVFiringEventView2::~MVFiringEventView2()
{
    delete d;
}

void MVFiringEventView2::prepareCalculation()
{
    d->m_calculator.labels_to_use = d->m_labels_to_use;
    d->m_calculator.firings = mvContext()->firings();
    d->m_calculator.filter = mvContext()->eventFilter();
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
    /// TODO: (MEDIUM) only do this if user has specified that it should be auto calculated (should be default)
    this->autoSetAmplitudeRange();
}

void MVFiringEventView2::setLabelsToUse(const QSet<int>& labels_to_use)
{
    d->m_labels_to_use = labels_to_use;
    d->m_legend->setClusterNumbers(d->m_labels_to_use.toList());
    this->recalculate();
}

void MVFiringEventView2::setAmplitudeRange(MVRange range)
{
    d->m_amplitude_range = range;
    d->m_axis_layer->amplitude_range = range;
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

void MVFiringEventView2::mouseMoveEvent(QMouseEvent* evt)
{
    d->m_legend->mouseMoveEvent(evt);
    /*
    int k=d->m_legend.clusterNumberAt(evt->pos());
    if (d->m_legend.hoveredClusterNumber()!=k) {
        d->m_legend.setHoveredClusterNumber(k);
        update();
    }
    */
}

void MVFiringEventView2::mouseReleaseEvent(QMouseEvent* evt)
{
    d->m_legend->mouseReleaseEvent(evt);
    /*
    int k=d->m_legend.clusterNumberAt(evt->pos());
    if (k>0) {
        /// TODO (LOW) make the legend more like a widget, responding to mouse clicks and movements on its own, and emitting signals
        d->m_legend.toggleActiveClusterNumber(k);
        evt->ignore();
        update();
    }
    */
    MVTimeSeriesViewBase::mouseReleaseEvent(evt);
}

void MVFiringEventView2::resizeEvent(QResizeEvent* evt)
{
    d->m_axis_layer->content_geometry = this->contentGeometry();
    d->m_paint_layer_stack.setWindowSize(this->size());
    MVTimeSeriesViewBase::resizeEvent(evt);
}

void MVFiringEventView2::paintContent(QPainter* painter)
{
    double alpha_pct = 0.7;
    for (long i = 0; i < d->m_times0.count(); i++) {
        double t0 = d->m_times0.value(i);
        int k0 = d->m_labels0.value(i);
        QColor col = mvContext()->clusterColor(k0);
        col.setAlpha((int)(alpha_pct * 255));
        QPen pen = painter->pen();
        pen.setColor(col);
        painter->setPen(pen);
        double amp0 = d->m_amplitudes0.value(i);
        double xpix = this->time2xpix(t0);
        double ypix = d->val2ypix(amp0);
        painter->drawEllipse(xpix, ypix, 3, 3);
    }

    d->m_paint_layer_stack.paint(painter);

    //legend
    //d->m_legend.setParentWindowSize(this->size());
    //d->m_legend.draw(painter);
    /*
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
            pen.setColor(mvContext()->clusterColor(list[i]));
            painter->setPen(pen);
            painter->drawText(rect, Qt::AlignRight, str);
            y0 += text_height + spacing;
        }
    }
    */
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
        QMessageBox::warning(0, "Unable to open firing events", "You must select at least one cluster.");
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
    setEnabled(!mvContext()->selectedClusters().isEmpty() && has_peaks);
}

void FiringEventAxisLayer::paint(QPainter* painter)
{
    draw_axis_opts opts;
    opts.minval = amplitude_range.min;
    opts.maxval = amplitude_range.max;
    opts.orientation = Qt::Vertical;
    opts.pt1 = content_geometry.bottomLeft() + QPointF(-3, 0);
    opts.pt2 = content_geometry.topLeft() + QPointF(-3, 0);
    opts.tick_length = 5;
    opts.color = Qt::white;
    draw_axis(painter, opts);
}
