#include "mountainprocessrunner.h"
#include "mvdiscrimhistview_sherpa.h"

#include <QGridLayout>
#include <taskprogress.h>
#include "msmisc.h"
#include "histogramview.h"
#include <QWheelEvent>
#include "mvmainwindow.h"

struct DiscrimHistogram {
    int k1, k2;
    QVector<double> data1, data2;
};

class MVDiscrimHistViewSherpaComputer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda timeseries;
    DiskReadMda firings;
    MVEventFilter event_filter;
    int num_histograms;

    //output
    QList<DiscrimHistogram> histograms;

    void compute();
};

class MVDiscrimHistViewSherpaPrivate {
public:
    MVDiscrimHistViewSherpa* q;

    int m_num_histograms = 20;

    MVDiscrimHistViewSherpaComputer m_computer;
    QList<DiscrimHistogram> m_histograms;

    double m_zoom_factor = 1;

    void set_views();
};

MVDiscrimHistViewSherpa::MVDiscrimHistViewSherpa(MVContext* context)
    : MVHistogramGrid(context)
{
    d = new MVDiscrimHistViewSherpaPrivate;
    d->q = this;

    this->recalculateOn(context, SIGNAL(currentTimeseriesChanged()));
    this->recalculateOn(context, SIGNAL(filteredFiringsChanged()));
    this->recalculateOn(context, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(context, SIGNAL(clusterVisibilityChanged()), false);

    this->recalculate();
}

MVDiscrimHistViewSherpa::~MVDiscrimHistViewSherpa()
{
    this->stopCalculation();
    delete d;
}

void MVDiscrimHistViewSherpa::setNumHistograms(int num)
{
    d->m_num_histograms = num;
}

void MVDiscrimHistViewSherpa::prepareCalculation()
{
    d->m_computer.mlproxy_url = mvContext()->mlProxyUrl();
    d->m_computer.timeseries = mvContext()->currentTimeseries();
    d->m_computer.firings = mvContext()->firings();
    d->m_computer.event_filter = mvContext()->eventFilter();
    d->m_computer.num_histograms = d->m_num_histograms;
}

void MVDiscrimHistViewSherpa::runCalculation()
{
    d->m_computer.compute();
}

double compute_min3(const QList<DiscrimHistogram>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QVector<double> tmp = data0[i].data1;
        tmp.append(data0[i].data2);
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] < ret)
                ret = tmp[j];
        }
    }
    return ret;
}

double compute_max3(const QList<DiscrimHistogram>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QVector<double> tmp = data0[i].data1;
        tmp.append(data0[i].data2);
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] > ret)
                ret = tmp[j];
        }
    }
    return ret;
}

void MVDiscrimHistViewSherpa::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    d->set_views();
}

void MVDiscrimHistViewSherpaComputer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("Discrim Histograms"));

    histograms.clear();

    firings = compute_filtered_firings_remotely(mlproxy_url, firings, event_filter);

    MountainProcessRunner MPR;
    MPR.setMLProxyUrl(mlproxy_url);
    MPR.setProcessorName("mv_discrimhist_sherpa");

    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries.makePath();
    params["firings"] = firings.makePath();
    params["num_histograms"] = num_histograms;
    MPR.setInputParameters(params);

    QString output_path = MPR.makeOutputFilePath("output");

    MPR.runProcess();

    DiskReadMda output(output_path);
    output.setRemoteDataType("float32");

    QMap<QString, DiscrimHistogram*> hist_lookup;

    long LL = output.N2();
    for (long i = 0; i < LL; i++) {
        int k1 = output.value(0, i);
        int k2 = output.value(1, i);
        int k0 = output.value(2, i);
        double val = output.value(3, i);
        QString code = QString("%1:%2").arg(k1).arg(k2);
        DiscrimHistogram* HH = hist_lookup.value(code);
        if (!HH) {
            DiscrimHistogram H;
            this->histograms << H;
            DiscrimHistogram* ptr = &this->histograms[this->histograms.count() - 1];
            ptr->k1 = k1;
            ptr->k2 = k2;
            hist_lookup[code] = ptr;
            HH = hist_lookup.value(code);
        }
        {
            if (k0 == k1)
                HH->data1 << val;
            else
                HH->data2 << val;
        }
    }
}

void MVDiscrimHistViewSherpa::wheelEvent(QWheelEvent* evt)
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

void MVDiscrimHistViewSherpaPrivate::set_views()
{
    double bin_min = compute_min3(m_histograms);
    double bin_max = compute_max3(m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));

    int num_bins = 200; //how to choose this?

    QList<HistogramView*> views;
    for (int ii = 0; ii < m_histograms.count(); ii++) {
        int k1 = m_histograms[ii].k1;
        int k2 = m_histograms[ii].k2;
        //if (q->mvContext()->clusterIsVisible(k1)) {
        {
            HistogramView* HV = new HistogramView;
            QVector<double> tmp = m_histograms[ii].data1;
            tmp.append(m_histograms[ii].data2);
            HV->setData(tmp);
            HV->setSecondData(m_histograms[ii].data2);
            HV->setColors(q->mvContext()->colors());
            //HV->autoSetBins(50);
            HV->setBins(bin_min, bin_max, num_bins);
            QString title0 = QString("%1/%2").arg(k1).arg(k2);
            HV->setTitle(title0);
            HV->setDrawVerticalAxisAtZero(true);
            HV->setXRange(MVRange(-max00, max00));
            HV->autoCenterXRange();
            HV->setProperty("k1", k1);
            HV->setProperty("k2", k2);
            views << HV;
        }
    }

    q->setHistogramViews(views); //inherited
}

MVDiscrimHistSherpaFactory::MVDiscrimHistSherpaFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
    connect(mvContext(), SIGNAL(selectedClustersChanged()),
        this, SLOT(updateEnabled()));
    updateEnabled();
}

QString MVDiscrimHistSherpaFactory::id() const
{
    return QStringLiteral("open-discrim-histograms-sherpa");
}

QString MVDiscrimHistSherpaFactory::name() const
{
    return tr("Discrim Histograms Sherpa");
}

QString MVDiscrimHistSherpaFactory::title() const
{
    return tr("Discrim");
}

MVAbstractView* MVDiscrimHistSherpaFactory::createView(QWidget* parent)
{
    MVDiscrimHistViewSherpa* X = new MVDiscrimHistViewSherpa(mvContext());
    X->setNumHistograms(50);
    return X;
}

void MVDiscrimHistSherpaFactory::updateEnabled()
{
    setEnabled(true);
    //setEnabled(mvContext()->selectedClusters().count() >= 2);
}
