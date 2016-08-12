#include "mvamphistview3.h"
#include "actionfactory.h"
#include "histogramlayer.h"
#include "mvpanelwidget.h"
#include "get_sort_indices.h"
/// TODO get_sort_indices should be put into MLCompute

#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <taskprogress.h>

struct AmpHistogram3 {
    int k;
    QVector<double> data;
};

class MVAmpHistView3Computer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings;

    //output
    QList<AmpHistogram3> histograms;

    void compute();
};

class MVAmpHistView3Private {
public:
    MVAmpHistView3* q;

    MVAmpHistView3Computer m_computer;
    QList<AmpHistogram3> m_histograms;
    MVPanelWidget* m_panel_widget;
    QList<HistogramLayer*> m_views;
    QSpinBox *m_num_bins_control;
    double m_zoom_factor = 1;

    void set_views();
};

MVAmpHistView3::MVAmpHistView3(MVContext *context) : MVAbstractView(context)
{
    d=new MVAmpHistView3Private;
    d->q=this;

    QVBoxLayout* layout = new QVBoxLayout;
    this->setLayout(layout);

    d->m_panel_widget = new MVPanelWidget;
    d->m_panel_widget->setScrollable(true, false);
    //d->m_panel_widget->setZoomOnWheel(false);
    layout->addWidget(d->m_panel_widget);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomIn, this, d->m_panel_widget, SLOT(zoomIn()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOut, this, d->m_panel_widget, SLOT(zoomOut()));

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomInHorizontal, this, SLOT(slot_zoom_in_horizontal()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOutHorizontal, this, SLOT(slot_zoom_out_horizontal()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::PanLeft, this, SLOT(slot_pan_left()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::PanRight, this, SLOT(slot_pan_right()));

    {
        d->m_num_bins_control=new QSpinBox();
        d->m_num_bins_control->setRange(1,2000);
        d->m_num_bins_control->setSingleStep(20);
        d->m_num_bins_control->setValue(100);
        this->addToolbarControl(new QLabel(" #bins:"));
        this->addToolbarControl(d->m_num_bins_control);
        QObject::connect(d->m_num_bins_control,SIGNAL(valueChanged(int)),this,SLOT(slot_update_bins()));
    }

    QObject::connect(context, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting_and_captions()));
    QObject::connect(context, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting_and_captions()));
    QObject::connect(context, SIGNAL(currentClusterChanged()), this, SLOT(slot_current_cluster_changed()));

    this->recalculateOn(context, SIGNAL(firingsChanged()), false);
    this->recalculateOn(context, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(context, SIGNAL(clusterVisibilityChanged()), false);
    this->recalculateOn(context, SIGNAL(viewMergedChanged()), false);
    this->recalculateOnOptionChanged("amp_thresh_display", false);

    connect(d->m_panel_widget, SIGNAL(signalPanelClicked(int, Qt::KeyboardModifiers)), this, SLOT(slot_panel_clicked(int, Qt::KeyboardModifiers)));

    this->recalculate();
}

MVAmpHistView3::~MVAmpHistView3() {
    this->stopCalculation();
    delete d;
}

void MVAmpHistView3::prepareCalculation()
{
    d->m_computer.mlproxy_url = mvContext()->mlProxyUrl();
    d->m_computer.firings = mvContext()->firings();
}

void MVAmpHistView3::runCalculation()
{
    d->m_computer.compute();
}

void MVAmpHistView3::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    d->set_views();
}

void MVAmpHistView3::wheelEvent(QWheelEvent *evt)
{
    int delta = evt->delta();
    if (delta > 0)
        slot_zoom_in_horizontal();
    else
        slot_zoom_out_horizontal();
}

void MVAmpHistView3::slot_zoom_in_horizontal(double factor)
{
    QList<HistogramLayer*> views = d->m_views;
    for (int i = 0; i < views.count(); i++) {
        views[i]->setXRange(views[i]->xRange() * (1.0 / factor));
        views[i]->autoCenterXRange();
    }
}

void MVAmpHistView3::slot_zoom_out_horizontal(double factor)
{
    slot_zoom_in_horizontal(1 / factor);
}

void MVAmpHistView3::slot_pan_left(double units)
{
    QList<HistogramLayer*> views = d->m_views;
    for (int i = 0; i < views.count(); i++) {
        MVRange range = views[i]->xRange();
        if (range.range()) {
            range = range + units * range.range();
        }
        views[i]->setXRange(range);
    }
}

void MVAmpHistView3::slot_pan_right(double units)
{
    slot_pan_left(-units);
}

void MVAmpHistView3::slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers)
{
    /// TODO this is redundant logic with mvtemplatesview2
    if (modifiers & Qt::ShiftModifier) {
        int i1 = d->m_panel_widget->currentPanelIndex();
        int i2 = index;
        int j1 = qMin(i1, i2);
        int j2 = qMax(i1, i2);
        if ((j1 >= 0) && (j2 >= 0)) {
            QSet<int> set = mvContext()->selectedClusters().toSet();
            for (int j = j1; j <= j2; j++) {
                if (d->m_views.value(j)) {
                    int k = d->m_views.value(j)->property("k").toInt();
                    if (k > 0)
                        set.insert(k);
                }
            }
            mvContext()->setSelectedClusters(set.toList());
        }
    }
    else {
        if (d->m_views.value(index)) {
            int k = d->m_views.value(index)->property("k").toInt();
            if (k>0) {
                mvContext()->clickCluster(k, modifiers);
            }
        }
    }
}

void MVAmpHistView3::slot_update_highlighting_and_captions()
{
    QList<int> selected_clusters = this->mvContext()->selectedClusters();
    QSet<ClusterPair> selected_cluster_pairs = this->mvContext()->selectedClusterPairs();
    for (int i = 0; i < d->m_views.count(); i++) {
        HistogramLayer* HV = d->m_views[i];
        QString title0, caption0;

        int k = HV->property("k").toInt();
        if (k == this->mvContext()->currentCluster()) {
            HV->setCurrent(true);
        }
        else {
            HV->setCurrent(false);
        }
        if (selected_clusters.contains(k)) {
            HV->setSelected(true);
        }
        else {
            HV->setSelected(false);
        }
        title0 = QString("%1").arg(k);
        caption0 = this->mvContext()->clusterTagsList(k).join(", ");

        HV->setTitle(title0);
        HV->setCaption(caption0);
    }
}

void MVAmpHistView3::slot_current_cluster_changed()
{
    int k=mvContext()->currentCluster();
    for (int i=0; i<d->m_views.count(); i++) {
        HistogramLayer *HV=d->m_views[i];
        if (HV->property("k").toInt()==k) {
            d->m_panel_widget->setCurrentPanelIndex(i); //for purpose of zooming
        }
    }
}

void MVAmpHistView3Computer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("AmplitudeHistograms3"));

    histograms.clear();

    QVector<int> labels;
    //QVector<double> amplitudes;
    long L = firings.N2();

    task.setProgress(0.2);
    for (long n = 0; n < L; n++) {
        labels << (int)firings.value(2, n);
    }

    int K = MLCompute::max<int>(labels);

    //assemble the histograms index 0 <--> k=1
    for (int k = 1; k <= K; k++) {
        AmpHistogram3 HH;
        HH.k = k;
        this->histograms << HH;
    }

    int row = 3; //for amplitudes
    for (long n = 0; n < L; n++) {
        int label0 = (int)firings.value(2, n);
        double amp0 = firings.value(row, n);
        if ((label0 >= 1) && (label0 <= K)) {
            this->histograms[label0 - 1].data << amp0;
        }
    }

    for (int i = 0; i < histograms.count(); i++) {
        if (histograms[i].data.count() == 0) {
            histograms.removeAt(i);
            i--;
        }
    }

    //sort by medium value
    QVector<double> abs_medians(histograms.count());
    for (int i=0; i<histograms.count(); i++) {
        abs_medians[i]=qAbs(MLCompute::median(histograms[i].data));
    }
    QList<long> inds=get_sort_indices(abs_medians);
    QList<AmpHistogram3> hist_new;
    for (int i=0; i<inds.count(); i++) {
        hist_new << histograms[inds[i]];
    }
    histograms=hist_new;
}

double compute_min3(const QList<AmpHistogram3>& data0)
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

double max3(const QList<AmpHistogram3>& data0)
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

void MVAmpHistView3::slot_update_bins()
{
    double bin_min = compute_min3(d->m_histograms);
    double bin_max = max3(d->m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));
    int num_bins = d->m_num_bins_control->value();
    for (int i=0; i<d->m_views.count(); i++) {
        d->m_views[i]->setBinInfo(bin_min,bin_max,num_bins);
        d->m_views[i]->setXRange(MVRange(-max00*0.8,max00*0.8));
        d->m_views[i]->autoCenterXRange();
    }
}

void MVAmpHistView3Private::set_views()
{
    m_panel_widget->clearPanels(true);
    m_views.clear();

    double amp_thresh = q->mvContext()->option("amp_thresh_display", 0).toDouble();

    QList<HistogramLayer*> views;
    for (int ii = 0; ii < m_histograms.count(); ii++) {
        int k0 = m_histograms[ii].k;
        if (q->mvContext()->clusterIsVisible(k0)) {
            HistogramLayer* HV = new HistogramLayer;
            HV->setData(m_histograms[ii].data);
            HV->setColors(q->mvContext()->colors());
            //HV->autoSetBins(50);
            HV->setDrawVerticalAxisAtZero(true);
            if (amp_thresh) {
                QList<double> vals;
                for (int i=1; i<=50; i++) {
                    vals << -amp_thresh*i << amp_thresh*i;
                }
                HV->setVerticalLines(vals);
            }
            HV->setProperty("k", k0);
            views << HV;
        }
    }

    int num_rows=qMax(1.0,sqrt(views.count()));
    int num_cols=ceil(views.count()*1.0/num_rows);
    m_views=views;
    for (int i=0; i<views.count(); i++) {
        m_panel_widget->addPanel(i/num_cols,i%num_cols,views[i]);
    }

    q->slot_update_highlighting_and_captions();
    q->slot_update_bins();
}

void MVAmpHistView3::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << mvContext()->selectedClusters();
    mimeData.setData("application/x-mv-clusters", ba); // selected cluster data
    MVAbstractView::prepareMimeData(mimeData, pos);
}

void MVAmpHistView3::keyPressEvent(QKeyEvent *evt)
{
    if (evt->matches(QKeySequence::SelectAll)) {
        QList<int> all_ks;
        for (int i = 0; i < d->m_views.count(); i++) {
            all_ks << d->m_views[i]->property("k").toInt();
        }
        mvContext()->setSelectedClusters(all_ks);
    }
}

MVAmplitudeHistograms3Factory::MVAmplitudeHistograms3Factory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
}

QString MVAmplitudeHistograms3Factory::id() const
{
    return QStringLiteral("open-amplitude-histograms-3");
}

QString MVAmplitudeHistograms3Factory::name() const
{
    return tr("Amplitude Histograms");
}

QString MVAmplitudeHistograms3Factory::title() const
{
    return tr("Amplitudes");
}

MVAbstractView* MVAmplitudeHistograms3Factory::createView(QWidget* parent)
{
    Q_UNUSED(parent)
    MVAmpHistView3* X = new MVAmpHistView3(mvContext());
    return X;
}

