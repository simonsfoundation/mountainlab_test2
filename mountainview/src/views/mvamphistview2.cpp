#include "mvamphistview2.h"

#include <QGridLayout>
#include <taskprogress.h>
#include "msmisc.h"
#include "histogramview.h"
#include <QWheelEvent>

/// TODO zoom in/out icons

struct AmpHistogram {
    int k;
    QVector<double> data;
};

class MVAmpHistView2Computer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings;
    MVEventFilter event_filter;

    //output
    QList<AmpHistogram> histograms;

    void compute();
};

class MVAmpHistView2Private {
public:
    MVAmpHistView2* q;

    MVAmpHistView2Computer m_computer;
    QList<AmpHistogram> m_histograms;
    double m_zoom_factor = 1;

    void set_views();
};

MVAmpHistView2::MVAmpHistView2(MVContext* context)
    : MVHistogramGrid(context)
{
    d = new MVAmpHistView2Private;
    d->q = this;

    this->recalculateOn(context, SIGNAL(filteredFiringsChanged()));
    this->recalculateOn(context, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(context, SIGNAL(clusterVisibilityChanged()), false);

    this->recalculate();
}

MVAmpHistView2::~MVAmpHistView2()
{
    delete d;
}

void MVAmpHistView2::prepareCalculation()
{
    d->m_computer.mlproxy_url = mvContext()->mlProxyUrl();
    d->m_computer.firings = mvContext()->firings();
    d->m_computer.event_filter = mvContext()->eventFilter();
}

void MVAmpHistView2::runCalculation()
{
    d->m_computer.compute();
}

double compute_min2(const QList<AmpHistogram>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QVector<double> tmp = data0[i].data;
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] < ret)
                ret = tmp[j];
        }
    }
    return ret;
}

double compute_max2(const QList<AmpHistogram>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QVector<double> tmp = data0[i].data;
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] > ret)
                ret = tmp[j];
        }
    }
    return ret;
}

void MVAmpHistView2::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    d->set_views();
}

void MVAmpHistView2Computer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("Amplitude AmpHistograms"));

    histograms.clear();

    firings = compute_filtered_firings_locally(firings, event_filter);

    QVector<int> labels;
    QVector<double> amplitudes;
    long L = firings.N2();

    task.setProgress(0.2);
    for (long n = 0; n < L; n++) {
        labels << (int)firings.value(2, n);
    }

    int K = compute_max(labels);

    //assembe the histograms index 0 <--> k=1
    for (int k = 1; k <= K; k++) {
        AmpHistogram HH;
        HH.k = k;
        this->histograms << HH;
    }

    for (long n = 0; n < L; n++) {
        int label0 = (int)firings.value(2, n);
        double amp0 = firings.value(3, n);
        if ((label0 >= 1) && (label0 <= K)) {
            this->histograms[label0 - 1].data << amp0;
        }
    }
}

void MVAmpHistView2::wheelEvent(QWheelEvent* evt)
{
    double zoom_factor = 1;
    if (evt->delta() > 0) {
        zoom_factor *= 1.2;
    }
    else if (evt->delta() < 0) {
        zoom_factor /= 1.2;
    }
    QList<HistogramView*> views = this->histogramViews(); //inherited
    for (int i = 0; i < views.count(); i++) {
        views[i]->setXRange(views[i]->xRange() * (1.0 / zoom_factor));
    }
}

void MVAmpHistView2Private::set_views()
{
    double bin_min = compute_min2(m_histograms);
    double bin_max = compute_max2(m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));

    int num_bins = 200; //how to choose this?

    QList<HistogramView*> views;
    for (int ii = 0; ii < m_histograms.count(); ii++) {
        int k0 = m_histograms[ii].k;
        if (q->mvContext()->clusterIsVisible(k0)) {
            HistogramView* HV = new HistogramView;
            HV->setData(m_histograms[ii].data);
            HV->setColors(q->mvContext()->colors());
            //HV->autoSetBins(50);
            HV->setBins(bin_min, bin_max, num_bins);
            QString title0 = QString("%1").arg(m_histograms[ii].k);
            HV->setTitle(title0);
            HV->setDrawVerticalAxisAtZero(true);
            HV->setXRange(MVRange(-max00, max00));
            HV->autoCenterXRange();
            HV->setProperty("k", k0);
            views << HV;
        }
    }

    q->setHistogramViews(views); //inherited
}

MVAmplitudeHistogramsFactory::MVAmplitudeHistogramsFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
}

QString MVAmplitudeHistogramsFactory::id() const
{
    return QStringLiteral("open-amplitude-histograms");
}

QString MVAmplitudeHistogramsFactory::name() const
{
    return tr("Amplitude Histograms");
}

QString MVAmplitudeHistogramsFactory::title() const
{
    return tr("Amplitudes");
}

MVAbstractView* MVAmplitudeHistogramsFactory::createView(QWidget* parent)
{
    MVAmpHistView2* X = new MVAmpHistView2(mvContext());
    QObject::connect(X, SIGNAL(histogramActivated()), this, SLOT(slot_amplitude_histogram_activated()));
    return X;
}

void MVAmplitudeHistogramsFactory::slot_amplitude_histogram_activated()
{
    MVAbstractView* view = qobject_cast<MVAbstractView*>(sender());
    if (!view)
        return;
    //not sure what to do here
}
