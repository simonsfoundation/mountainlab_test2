#include "mvamphistview.h"

#include <QGridLayout>
#include <taskprogress.h>
#include "msmisc.h"
#include "histogramview.h"
#include <QWheelEvent>

/// TODO zoom in/out icons
/// TODO (HIGH) figure out panning? Should it be per view?

struct Histogram {
    int k;
    QVector<double> data;
};

class MVAmpHistViewComputer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings;
    MVEventFilter event_filter;

    //output
    QList<Histogram> histograms;

    void compute();
};

class MVAmpHistViewPrivate {
public:
    MVAmpHistView* q;
    int m_num_columns = -1;
    QGridLayout* m_grid_layout;

    MVAmpHistViewComputer m_computer;
    QList<Histogram> m_histograms;
    QList<HistogramView*> m_histogram_views;
    QList<QWidget*> m_child_widgets;

    void do_highlighting();
};

MVAmpHistView::MVAmpHistView(MVContext* context)
    : MVAbstractView(context)
{
    d = new MVAmpHistViewPrivate;
    d->q = this;

    QObject::connect(context, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(context, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting()));

    this->recalculateOn(context, SIGNAL(filteredFiringsChanged()));
    this->recalculateOn(context, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(context, SIGNAL(clusterVisibilityChanged()), false);

    QGridLayout* GL = new QGridLayout;
    GL->setHorizontalSpacing(20);
    GL->setVerticalSpacing(0);
    GL->setMargin(0);
    this->setLayout(GL);
    d->m_grid_layout = GL;

    this->recalculate();
}

MVAmpHistView::~MVAmpHistView()
{
    delete d;
}

void MVAmpHistView::prepareCalculation()
{
    d->m_computer.mlproxy_url = mvContext()->mlProxyUrl();
    d->m_computer.firings = mvContext()->firings();
    d->m_computer.event_filter = mvContext()->eventFilter();
}

void MVAmpHistView::runCalculation()
{
    d->m_computer.compute();
}

double compute_min2(const QList<Histogram>& data0)
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

double compute_max2(const QList<Histogram>& data0)
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

void MVAmpHistView::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    qDeleteAll(d->m_histogram_views);
    d->m_histogram_views.clear();

    qDeleteAll(d->m_child_widgets);
    d->m_child_widgets.clear();

    QGridLayout* GL = d->m_grid_layout;
    double bin_min = compute_min2(d->m_histograms);
    double bin_max = compute_max2(d->m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));

    int num_bins = 200; //how to choose this?

    QVector<int> inds_to_use;
    for (int ii = 0; ii < d->m_histograms.count(); ii++) {
        int k0 = d->m_histograms[ii].k;
        if (mvContext()->clusterIsVisible(k0)) {
            inds_to_use << ii;
        }
    }

    int NUM = inds_to_use.count();
    int num_rows = (int)sqrt(NUM);
    if (num_rows < 1)
        num_rows = 1;
    int num_cols = (NUM + num_rows - 1) / num_rows;
    d->m_num_columns = num_cols;
    for (int jj = 0; jj < inds_to_use.count(); jj++) {
        int ii = inds_to_use[jj];
        HistogramView* HV = new HistogramView;
        HV->setData(d->m_histograms[ii].data);
        HV->setColors(mvContext()->colors());
        //HV->autoSetBins(50);
        HV->setBins(bin_min, bin_max, num_bins);
        QString title0 = QString("%1").arg(d->m_histograms[ii].k);
        HV->setTitle(title0);
        HV->setDrawVerticalAxisAtZero(true);
        HV->setXRange(MVRange(-max00, max00));
        int row0 = (jj) / num_cols;
        int col0 = (jj) % num_cols;
        GL->addWidget(HV, row0, col0);
        HV->setProperty("row", row0);
        HV->setProperty("col", col0);
        HV->setProperty("index", ii);
        connect(HV, SIGNAL(clicked(Qt::KeyboardModifiers)), this, SLOT(slot_histogram_view_clicked(Qt::KeyboardModifiers)));
        connect(HV, SIGNAL(activated()), this, SLOT(slot_histogram_view_activated()));
        connect(HV, SIGNAL(signalExportHistogramMatrixImage()), this, SLOT(slot_export_image()));
        d->m_histogram_views << HV;
    }

    //TimeScaleWidget2* TSW = new TimeScaleWidget2;
    //TSW->m_time_width = time_width;
    //GL->addWidget(TSW, num_rows, 0);
    //d->m_child_widgets << TSW;
}

void MVAmpHistView::slot_histogram_view_clicked(Qt::KeyboardModifiers modifiers)
{
    int index = sender()->property("index").toInt();
    int k = d->m_histograms.value(index).k;

    if (modifiers & Qt::ControlModifier) {
        mvContext()->clickCluster(k, Qt::ControlModifier);
    }
    else {
        mvContext()->clickCluster(k, Qt::NoModifier);
    }
}

void MVAmpHistView::slot_histogram_view_activated()
{
    emit histogramActivated();
}

void MVAmpHistView::slot_export_image()
{
    QImage img = this->renderImage();
    user_save_image(img);
}

void MVAmpHistView::slot_update_highlighting()
{
    d->do_highlighting();
}

void MVAmpHistViewComputer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("Amplitude Histograms"));

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
        Histogram HH;
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

void MVAmpHistViewPrivate::do_highlighting()
{
    QVector<int> selected_clusters = q->mvContext()->selectedClusters();
    for (int i = 0; i < m_histogram_views.count(); i++) {
        HistogramView* HV = m_histogram_views[i];
        int index = HV->property("index").toInt();
        if (m_histograms.value(index).k == q->mvContext()->currentCluster()) {
            HV->setCurrent(true);
        }
        else {
            HV->setCurrent(false);
        }
        if (selected_clusters.contains(m_histograms.value(index).k)) {
            HV->setSelected(true);
        }
        else {
            HV->setSelected(false);
        }
    }
}

QImage MVAmpHistView::renderImage(int W, int H)
{
    if (!W)
        W = 1800;
    if (!H)
        H = 900;
    int max_row = 0, max_col = 0;
    for (int i = 0; i < d->m_histogram_views.count(); i++) {
        HistogramView* HV = d->m_histogram_views[i];
        int row = HV->property("row").toInt();
        int col = HV->property("col").toInt();
        if (row > max_row)
            max_row = row;
        if (col > max_col)
            max_col = col;
    }
    int NR = max_row + 1, NC = max_col + 1;
    int spacingx = 10;
    int spacingy = 10;
    int W0 = (W - spacingx * (NC + 1)) / NC;
    int H0 = (H - spacingy * (NR + 1)) / NR;

    QImage ret = QImage(W, H, QImage::Format_RGB32);
    QPainter painter(&ret);
    painter.fillRect(0, 0, ret.width(), ret.height(), Qt::white);

    for (int i = 0; i < d->m_histogram_views.count(); i++) {
        HistogramView* HV = d->m_histogram_views[i];
        int row = HV->property("row").toInt();
        int col = HV->property("col").toInt();
        QImage img = HV->renderImage(W0, H0);
        int x0 = spacingx + (W0 + spacingx) * col;
        int y0 = spacingy + (H0 + spacingy) * row;
        painter.drawImage(x0, y0, img);
    }

    return ret;
}

void MVAmpHistView::wheelEvent(QWheelEvent* evt)
{
    double zoom_factor = 1;
    if (evt->delta() > 0) {
        zoom_factor *= 1.2;
    }
    else if (evt->delta() < 0) {
        zoom_factor /= 1.2;
    }
    for (int i = 0; i < d->m_histogram_views.count(); i++) {
        d->m_histogram_views[i]->setXRange(d->m_histogram_views[i]->xRange() * (1.0 / zoom_factor));
    }
}
