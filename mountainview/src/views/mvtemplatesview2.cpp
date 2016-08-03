/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "mvtemplatesview2.h"
#include "mvpanelwidget.h"
#include "mvtemplatesview2panel.h"

#include <QSpinBox>
#include <QVBoxLayout>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"

struct ClusterData2 {
    int k = 0;
    int channel = 0;
    Mda template0;
    Mda stdev0;
    int num_events = 0;

    QJsonObject toJsonObject();
    void fromJsonObject(const QJsonObject& X);
};

class MVTemplatesView2Calculator {
public:
    //input
    QString mlproxy_url;
    MVEventFilter filter;
    DiskReadMda timeseries;
    DiskReadMda firings;
    int clip_size;

    //output
    QList<ClusterData2> cluster_data;

    virtual void compute();

    bool loaded_from_static_output = false;
    QJsonObject exportStaticOutput();
    void loadStaticOutput(const QJsonObject& X);
};

class MVTemplatesView2Private {
public:
    MVTemplatesView2* q;

    MVTemplatesView2Calculator m_calculator;
    QList<ClusterData2> m_cluster_data;
    double m_max_absval = 1;
    QList<MVTemplatesView2Panel*> m_panels;
    int m_num_rows=1;

    double m_total_time_sec = 0;
    bool m_zoomed_out_once = false;
    MVPanelWidget* m_panel_widget;
    double m_vscale_factor = 4;
    double m_hscale_factor = 2;

    void compute_total_time();
    void update_panels();
    void update_scale_factors();
};

MVTemplatesView2::MVTemplatesView2(MVContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new MVTemplatesView2Private;
    d->q = this;

    QVBoxLayout* layout = new QVBoxLayout;
    this->setLayout(layout);

    d->m_panel_widget = new MVPanelWidget;
    d->m_panel_widget->setScrollable(true, false);
    layout->addWidget(d->m_panel_widget);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomIn, this, d->m_panel_widget, SLOT(zoomIn()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOut, this, d->m_panel_widget, SLOT(zoomOut()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomInVertical, this, SLOT(slot_vertical_zoom_in()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOutVertical, this, SLOT(slot_vertical_zoom_out()));

    this->recalculateOn(mvcontext, SIGNAL(firingsChanged()), false);
    this->recalculateOn(mvcontext, SIGNAL(currentTimeseriesChanged()));
    this->recalculateOnOptionChanged("clip_size");

    QObject::connect(mvcontext, SIGNAL(clusterMergeChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(mvcontext, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(mvcontext, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting()));
    connect(mvcontext, SIGNAL(clusterVisibilityChanged()), this, SLOT(slot_update_panels()));
    connect(mvcontext, SIGNAL(viewMergedChanged()), this, SLOT(slot_update_panels()));

    connect(d->m_panel_widget, SIGNAL(signalPanelClicked(int, Qt::KeyboardModifiers)), this, SLOT(slot_panel_clicked(int, Qt::KeyboardModifiers)));

    {
        QSpinBox *SB=new QSpinBox;
        SB->setRange(1,10);
        SB->setValue(1);
        QObject::connect(SB,SIGNAL(valueChanged(int)),this,SLOT(slot_set_num_rows(int)));
        this->addToolbarControl(SB);
    }

    this->recalculate();
}

MVTemplatesView2::~MVTemplatesView2()
{
    this->stopCalculation();
    delete d;
}

void MVTemplatesView2::prepareCalculation()
{
    if (!d->m_calculator.loaded_from_static_output) {
        d->compute_total_time();
    }
    d->m_calculator.mlproxy_url = mvContext()->mlProxyUrl();
    d->m_calculator.filter = mvContext()->eventFilter();
    d->m_calculator.timeseries = mvContext()->currentTimeseries();
    d->m_calculator.firings = mvContext()->firings();
    d->m_calculator.clip_size = mvContext()->option("clip_size", 100).toInt();
    update();
}

void MVTemplatesView2::runCalculation()
{
    d->m_calculator.compute();
}

void MVTemplatesView2::onCalculationFinished()
{
    d->m_cluster_data = d->m_calculator.cluster_data;
    if (!d->m_zoomed_out_once) {
        this->zoomAllTheWayOut();
        d->m_zoomed_out_once = true;
    }
    d->update_panels();
}

void MVTemplatesView2::zoomAllTheWayOut()
{
    d->m_panel_widget->setViewportGeometry(QRectF(0, 0, 1, 1));
}

void MVTemplatesView2::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Up) {
        slot_vertical_zoom_in();
    }
    if (evt->key() == Qt::Key_Down) {
        slot_vertical_zoom_out();
    }
}

void MVTemplatesView2::slot_update_panels()
{
    d->update_panels();
}

void MVTemplatesView2::slot_update_highlighting()
{
    int current_k = mvContext()->currentCluster();
    QSet<int> selected_ks = mvContext()->selectedClusters().toSet();
    for (int i = 0; i < d->m_panels.count(); i++) {
        int k0 = d->m_cluster_data.value(i).k;
        d->m_panels[i]->setCurrent(current_k == k0);
        d->m_panels[i]->setSelected(selected_ks.contains(k0));
        if (current_k == k0) {
            d->m_panel_widget->setCurrentPanelIndex(i); //for purposes of zooming
        }
    }
    update();
}

void MVTemplatesView2::slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ShiftModifier) {
        int i1 = d->m_panel_widget->currentPanelIndex();
        int i2 = index;
        int j1 = qMin(i1, i2);
        int j2 = qMax(i1, i2);
        if ((j1 >= 0) && (j2 >= 0)) {
            QSet<int> set = mvContext()->selectedClusters().toSet();
            for (int j = j1; j <= j2; j++) {
                int k = d->m_cluster_data.value(j).k;
                if (k > 0)
                    set.insert(k);
            }
            mvContext()->setSelectedClusters(set.toList());
        }
    }
    else {
        ClusterData2 CD = d->m_cluster_data.value(index);
        if (CD.k > 0) {
            mvContext()->clickCluster(CD.k, modifiers);
        }
    }
}

void MVTemplatesView2::slot_vertical_zoom_in()
{
    d->m_vscale_factor *= 1.1;
    d->update_scale_factors();
}

void MVTemplatesView2::slot_vertical_zoom_out()
{
    d->m_vscale_factor /= 1.1;
    d->update_scale_factors();
}

void MVTemplatesView2::slot_set_num_rows(int num_rows)
{
    d->m_num_rows=num_rows;
    d->update_panels();
}

void mv_compute_templates_stdevs(DiskReadMda& templates_out, DiskReadMda& stdevs_out, const QString& mlproxy_url, const QString& timeseries, const QString& firings, int clip_size)
{
    TaskProgress task(TaskProgress::Calculate, "mv_compute_templates_stdevs");
    task.log("mlproxy_url: " + mlproxy_url);
    MountainProcessRunner X;
    QString processor_name = "mv_compute_templates";
    X.setProcessorName(processor_name);

    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries;
    params["firings"] = firings;
    params["clip_size"] = clip_size;
    X.setInputParameters(params);
    X.setMLProxyUrl(mlproxy_url);

    QString templates_fname = X.makeOutputFilePath("templates");
    QString stdevs_fname = X.makeOutputFilePath("stdevs");

    task.log("X.compute()");
    X.runProcess();
    task.log("Returning DiskReadMda: " + templates_fname + " " + stdevs_fname);
    templates_out.setPath(templates_fname);
    stdevs_out.setPath(stdevs_fname);

    templates_out.setRemoteDataType("float32_q8");
    stdevs_out.setRemoteDataType("float32_q8");
}

void MVTemplatesView2Calculator::compute()
{
    TaskProgress task(TaskProgress::Calculate, "Cluster Details");
    if (this->loaded_from_static_output) {
        task.log("Loaded from static output");
        return;
    }

    QTime timer;
    timer.start();
    task.setProgress(0.1);

    firings = compute_filtered_firings_remotely(mlproxy_url, firings, filter);

    int M = timeseries.N1();
    //int N = timeseries.N2();
    int L = firings.N2();
    int T = clip_size;

    QVector<int> labels;

    task.log("Setting up labels");
    task.setProgress(0.2);
    for (int i = 0; i < L; i++) {
        labels << (int)firings.value(2, i);
    }

    if (MLUtil::threadInterruptRequested()) {
        task.error("Halted *");
        return;
    }
    task.log("Clearing data");
    cluster_data.clear();

    task.setLabel("Computing templates");
    task.setProgress(0.4);
    int K = MLCompute::max(labels);

    QString timeseries_path = timeseries.makePath();
    QString firings_path = firings.makePath();

    task.log("mp_compute_templates_stdevs: " + mlproxy_url + " timeseries_path=" + timeseries_path + " firings_path=" + firings_path);
    task.setProgress(0.6);
    DiskReadMda templates0, stdevs0;
    mv_compute_templates_stdevs(templates0, stdevs0, mlproxy_url, timeseries_path, firings_path, T);
    if (MLUtil::threadInterruptRequested()) {
        task.error("Halted **");
        return;
    }

    task.setLabel("Setting cluster data");
    task.setProgress(0.75);
    for (int k = 1; k <= K; k++) {
        if (MLUtil::threadInterruptRequested()) {
            task.error("Halted ***");
            return;
        }
        ClusterData2 CD;
        CD.k = k;
        CD.channel = 0;
        for (int i = 0; i < L; i++) {
            if (labels[i] == k) {
                CD.num_events++;
            }
        }
        if (MLUtil::threadInterruptRequested()) {
            task.error("Halted ****");
            return;
        }
        templates0.readChunk(CD.template0, 0, 0, k - 1, M, T, 1);
        stdevs0.readChunk(CD.stdev0, 0, 0, k - 1, M, T, 1);
        if (!MLUtil::threadInterruptRequested()) {
            if (CD.num_events > 0) {
                cluster_data << CD;
            }
        }
    }
}

void MVTemplatesView2Private::compute_total_time()
{
    m_total_time_sec = q->mvContext()->currentTimeseries().N2() / q->mvContext()->sampleRate();
}

double get_disksize_for_firing_rate(double firing_rate)
{
    return qMin(1.0, sqrt(firing_rate / 5));
}

void MVTemplatesView2Private::update_panels()
{
    m_panel_widget->clearPanels(true);
    m_panels.clear();
    QList<QColor> channel_colors;
    {
        int M = m_cluster_data.value(0).template0.N1();
        for (int m = 0; m < M; m++) {
            channel_colors << q->mvContext()->channelColor(m);
        }
    }
    m_max_absval = 0;
    for (int i = 0; i < m_cluster_data.count(); i++) {
        m_max_absval = qMax(qMax(m_max_absval, qAbs(m_cluster_data[i].template0.minimum())), qAbs(m_cluster_data[i].template0.maximum()));
    }
    int num_rows = m_num_rows;
    int num_cols = ceil(m_cluster_data.count() * 1.0 / num_rows);
    for (int i = 0; i < m_cluster_data.count(); i++) {
        ClusterData2 CD = m_cluster_data[i];
        MVTemplatesView2Panel* panel = new MVTemplatesView2Panel;
        panel->setElectrodeGeometry(q->mvContext()->electrodeGeometry());
        panel->setTemplate(CD.template0);
        panel->setChannelColors(channel_colors);
        panel->setColors(q->mvContext()->colors());
        panel->setTitle(QString::number(CD.k));
        if (q->mvContext()->sampleRate()) {
            double total_time_sec = q->mvContext()->currentTimeseries().N2() / q->mvContext()->sampleRate();
            if (total_time_sec) {
                double firing_rate = CD.num_events / total_time_sec;
                panel->setFiringRateDiskDiameter(get_disksize_for_firing_rate(firing_rate));
            }
        }
        m_panel_widget->addPanel(i / num_cols, i % num_cols, panel);
        m_panels << panel;
    }
    update_scale_factors();
    q->slot_update_highlighting();
}

void MVTemplatesView2Private::update_scale_factors()
{
    double vfactor = m_vscale_factor;
    if (m_max_absval)
        vfactor /= m_max_absval;
    for (int i = 0; i < m_panels.count(); i++) {
        m_panels[i]->setVerticalScaleFactor(vfactor);
    }
    q->update();
}

MVTemplatesView2Factory::MVTemplatesView2Factory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
}

QString MVTemplatesView2Factory::id() const
{
    return QStringLiteral("open-templates");
}

QString MVTemplatesView2Factory::name() const
{
    return tr("Templates");
}

QString MVTemplatesView2Factory::title() const
{
    return tr("Templates");
}

MVAbstractView* MVTemplatesView2Factory::createView(QWidget* parent)
{
    Q_UNUSED(parent)
    MVTemplatesView2* X = new MVTemplatesView2(mvContext());
    return X;
}
