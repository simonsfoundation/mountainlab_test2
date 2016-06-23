#include "mountainprocessrunner.h"
#include "mvdiscrimhistview.h"

#include <QGridLayout>
#include <taskprogress.h>
#include "msmisc.h"
#include "histogramview.h"
#include <QWheelEvent>

/// TODO zoom in/out icons

struct DiscrimHistogram {
    int k1, k2;
    QList<double> data1, data2;
};

class MVDiscrimHistViewComputer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda timeseries;
    DiskReadMda firings;
    MVEventFilter event_filter;
    QList<int> cluster_numbers;

    //output
    QList<DiscrimHistogram> histograms;

    void compute();
};

class MVDiscrimHistViewPrivate {
public:
    MVDiscrimHistView* q;

    QList<int> m_cluster_numbers;

    MVDiscrimHistViewComputer m_computer;
    QList<DiscrimHistogram> m_histograms;

    double m_zoom_factor = 1;

    void set_views();
};

MVDiscrimHistView::MVDiscrimHistView(MVViewAgent* view_agent)
    : MVHistogramGrid(view_agent)
{
    d = new MVDiscrimHistViewPrivate;
    d->q = this;

    this->recalculateOn(view_agent, SIGNAL(currentTimeseriesChanged()));
    this->recalculateOn(view_agent, SIGNAL(filteredFiringsChanged()));
    this->recalculateOn(view_agent, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(view_agent, SIGNAL(clusterVisibilityChanged()), false);

    this->recalculate();
}

MVDiscrimHistView::~MVDiscrimHistView()
{
    delete d;
}

void MVDiscrimHistView::setClusterNumbers(const QList<int>& cluster_numbers)
{
    d->m_cluster_numbers = cluster_numbers;
}

void MVDiscrimHistView::prepareCalculation()
{
    d->m_computer.mlproxy_url = viewAgent()->mlProxyUrl();
    d->m_computer.timeseries = viewAgent()->currentTimeseries();
    d->m_computer.firings = viewAgent()->firings();
    d->m_computer.event_filter = viewAgent()->eventFilter();
    d->m_computer.cluster_numbers = d->m_cluster_numbers;
}

void MVDiscrimHistView::runCalculation()
{
    d->m_computer.compute();
}

double compute_min2(const QList<DiscrimHistogram>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QList<double> tmp = data0[i].data1;
        tmp.append(data0[i].data2);
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] < ret)
                ret = tmp[j];
        }
    }
    return ret;
}

double compute_max2(const QList<DiscrimHistogram>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QList<double> tmp = data0[i].data1;
        tmp.append(data0[i].data2);
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] > ret)
                ret = tmp[j];
        }
    }
    return ret;
}

void MVDiscrimHistView::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    d->set_views();
}

QList<double> negative(const QList<double>& X)
{
    QList<double> ret;
    for (int i = 0; i < X.count(); i++) {
        ret << -X[i];
    }
    return ret;
}

void MVDiscrimHistViewComputer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("Amplitude AmpHistograms"));

    histograms.clear();

    firings = compute_filtered_firings_remotely(mlproxy_url, firings, event_filter);

    MountainProcessRunner MPR;
    MPR.setMLProxyUrl(mlproxy_url);
    MPR.setProcessorName("mv_discrimhist");

    QStringList clusters_strlist;
    foreach(int cluster, cluster_numbers)
    {
        clusters_strlist << QString("%1").arg(cluster);
    }

    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries.makePath();
    params["firings"] = firings.makePath();
    params["clusters"] = clusters_strlist.join(",");
    MPR.setInputParameters(params);

    QString output_path = MPR.makeOutputFilePath("output");

    MPR.runProcess();

    DiskReadMda output(output_path);

    QMap<QString, DiscrimHistogram*> hist_lookup;
    for (int i2 = 0; i2 < cluster_numbers.count(); i2++) {
        for (int i1 = 0; i1 < cluster_numbers.count(); i1++) {
            DiscrimHistogram H;
            H.k1 = cluster_numbers[i1];
            H.k2 = cluster_numbers[i2];
            this->histograms << H;
            hist_lookup[QString("%1:%2").arg(H.k1).arg(H.k2)] = &this->histograms[this->histograms.count() - 1];
        }
    }

    long LL = output.N2();
    for (long i = 0; i < LL; i++) {
        int k1 = output.value(0, i);
        int k2 = output.value(1, i);
        int k0 = output.value(2, i);
        double val = output.value(3, i);
        DiscrimHistogram* HH = hist_lookup[QString("%1:%2").arg(k1).arg(k2)];
        if (HH) {
            if (k0 == k1)
                HH->data1 << val;
            else
                HH->data2 << val;
        }
    }

    //copy by symmetry to missing histograms
    for (int i2 = 0; i2 < cluster_numbers.count(); i2++) {
        for (int i1 = 0; i1 < cluster_numbers.count(); i1++) {
            int k1 = cluster_numbers[i1];
            int k2 = cluster_numbers[i2];
            DiscrimHistogram* HH1 = hist_lookup[QString("%1:%2").arg(k1).arg(k2)];
            DiscrimHistogram* HH2 = hist_lookup[QString("%2:%1").arg(k1).arg(k2)];
            if (HH1->data1.isEmpty())
                HH1->data1 = negative(HH2->data2);
            if (HH1->data2.isEmpty())
                HH1->data2 = negative(HH2->data1);
        }
    }
}

void MVDiscrimHistView::wheelEvent(QWheelEvent* evt)
{
    double zoom_factor = 1;
    if (evt->delta() > 0) {
        zoom_factor *= 1.2;
    } else if (evt->delta() < 0) {
        zoom_factor /= 1.2;
    }
    QList<HistogramView*> views = this->histogramViews(); //inherited
    for (int i = 0; i < views.count(); i++) {
        views[i]->setXRange(views[i]->xRange() * (1.0 / zoom_factor));
    }
}

void MVDiscrimHistViewPrivate::set_views()
{
    double bin_min = compute_min2(m_histograms);
    double bin_max = compute_max2(m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));

    int num_bins = 200; //how to choose this?

    QList<HistogramView*> views;
    for (int ii = 0; ii < m_histograms.count(); ii++) {
        int k1 = m_histograms[ii].k1;
        int k2 = m_histograms[ii].k2;
        //if (q->viewAgent()->clusterIsVisible(k1)) {
        {
            HistogramView* HV = new HistogramView;
            QList<double> tmp = m_histograms[ii].data1;
            tmp.append(m_histograms[ii].data2);
            HV->setData(tmp);
            HV->setSecondData(m_histograms[ii].data2);
            HV->setColors(q->viewAgent()->colors());
            //HV->autoSetBins(50);
            HV->setBins(bin_min, bin_max, num_bins);
            QString title0 = QString("%1/%2").arg(k1).arg(k2);
            HV->setTitle(title0);
            HV->setDrawVerticalAxisAtZero(true);
            HV->setXRange(MVRange(-max00, max00));
            HV->autoCenterXRange();
            HV->setProperty("k", k2);
            views << HV;
        }
    }

    q->setHistogramViews(views); //inherited
}
