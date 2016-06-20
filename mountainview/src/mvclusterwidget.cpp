#include "mvclusterwidget.h"
#include "mvclusterview.h"
#include "computationthread.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <taskprogress.h>
#include "mvclipsview.h"
#include "msmisc.h"
#include <math.h>
#include "extract_clips.h"
#include "mountainprocessrunner.h"

/// TODO legend as separate class
/// TODO highlight white on hover over label
/// TODO toggle on/off
/// TODO (MEDIUM) control brightness in 3D density view

class MVClusterWidgetComputer : public ComputationThread {
public:
    //input
    QString mlproxy_url;
    MVEventFilter filter;
    DiskReadMda timeseries;
    DiskReadMda firings;
    int clip_size;
    QList<int> labels_to_use;
    QString features_mode; //"pca" or "channels"
    QList<int> channels; //in case of feature_mode=="channels"

    //output
    Mda data;
    QList<double> times;
    QList<int> labels;
    QList<double> amplitudes;
    QList<double> detectability_scores;
    QList<double> outlier_scores;

    void compute();
};

class MVClusterWidgetPrivate {
public:
    MVClusterWidget* q;
    QList<MVClusterView*> m_views;
    MVClipsView* m_clips_view;
    QLabel* m_info_bar;
    Mda m_data;
    QList<int> m_labels_to_use;
    QList<double> m_outlier_scores;
    MVClusterWidgetComputer m_computer;
    QString m_feature_mode;
    QList<int> m_channels;

    void connect_view(MVClusterView* V);
    void update_clips_view();
    int current_event_index();
    void set_data_on_visible_views();
};

MVClusterWidget::MVClusterWidget(MVViewAgent* view_agent)
    : MVAbstractView(view_agent)
{
    d = new MVClusterWidgetPrivate;
    d->q = this;

    d->m_clips_view = new MVClipsView(view_agent);

    {
        MVClusterView* X = new MVClusterView(view_agent);
        X->setMode(MVCV_MODE_HEAT_DENSITY);
        d->m_views << X;
    }
    {
        MVClusterView* X = new MVClusterView(view_agent);
        X->setMode(MVCV_MODE_LABEL_COLORS);
        d->m_views << X;
    }
    {
        MVClusterView* X = new MVClusterView(view_agent);
        X->setMode(MVCV_MODE_TIME_COLORS);
        X->setVisible(false);
        d->m_views << X;
    }
    {
        MVClusterView* X = new MVClusterView(view_agent);
        X->setMode(MVCV_MODE_AMPLITUDE_COLORS);
        X->setVisible(false);
        d->m_views << X;
    }

    QVBoxLayout* mainlayout = new QVBoxLayout;
    this->setLayout(mainlayout);

    QHBoxLayout* bottom_panel = new QHBoxLayout;
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Clip View");
        connect(CB, SIGNAL(toggled(bool)), this, SLOT(slot_show_clip_view_toggled(bool)));
        CB->setChecked(true);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Density Plot");
        CB->setProperty("view_index", 0);
        connect(CB, SIGNAL(toggled(bool)), this, SLOT(slot_show_view_toggled(bool)));
        CB->setChecked(true);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Label Colors");
        CB->setProperty("view_index", 1);
        connect(CB, SIGNAL(toggled(bool)), this, SLOT(slot_show_view_toggled(bool)));
        CB->setChecked(true);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Time Colors");
        CB->setProperty("view_index", 2);
        connect(CB, SIGNAL(toggled(bool)), this, SLOT(slot_show_view_toggled(bool)));
        CB->setChecked(false);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Amplitude Colors");
        CB->setProperty("view_index", 3);
        connect(CB, SIGNAL(toggled(bool)), this, SLOT(slot_show_view_toggled(bool)));
        CB->setChecked(false);
        bottom_panel->addWidget(CB);
    }
    bottom_panel->addStretch();

    QHBoxLayout* hlayout = new QHBoxLayout;
    mainlayout->addLayout(hlayout);
    mainlayout->addLayout(bottom_panel);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_clips_view);
    d->m_info_bar = new QLabel;
    d->m_info_bar->setFixedHeight(20);
    vlayout->addWidget(d->m_info_bar);

    QSizePolicy clips_size_policy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    clips_size_policy.setHorizontalStretch(1);

    QWidget* clips_widget = new QWidget;
    clips_widget->setLayout(vlayout);
    clips_widget->setSizePolicy(clips_size_policy);
    hlayout->addWidget(clips_widget);

    QSizePolicy view_size_policy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    view_size_policy.setHorizontalStretch(1);

    foreach (MVClusterView* V, d->m_views) {
        V->setSizePolicy(view_size_policy);
        hlayout->addWidget(V);
    }

    foreach (MVClusterView* V, d->m_views) {
        d->connect_view(V);
    }

    this->recalculateOn(view_agent, SIGNAL(currentTimeseriesChanged()));
    this->recalculateOn(view_agent, SIGNAL(filteredFiringsChanged()));
    this->recalculateOnOptionChanged("clip_size");

    connect(view_agent, SIGNAL(currentEventChanged()), this, SLOT(slot_current_event_changed()));
}

MVClusterWidget::~MVClusterWidget()
{
    delete d;
}

void MVClusterWidget::prepareCalculation()
{
    d->m_computer.mlproxy_url = viewAgent()->mlProxyUrl();
    d->m_computer.filter = viewAgent()->eventFilter();
    d->m_computer.timeseries = viewAgent()->currentTimeseries();
    d->m_computer.firings = viewAgent()->firings();
    d->m_computer.clip_size = viewAgent()->option("clip_size").toInt();
    d->m_computer.labels_to_use = d->m_labels_to_use;
    d->m_computer.features_mode = d->m_feature_mode;
    d->m_computer.channels = d->m_channels;
}

void MVClusterWidget::runCalculation()
{
    d->m_computer.compute();
}

void MVClusterWidget::onCalculationFinished()
{
    this->setTimes(d->m_computer.times);
    this->setLabels(d->m_computer.labels);
    this->setAmplitudes(d->m_computer.amplitudes);
    this->setScores(d->m_computer.detectability_scores, d->m_computer.outlier_scores);
    this->setData(d->m_computer.data);
}

void MVClusterWidget::setData(const Mda& X)
{
    d->m_data = X;
    foreach (MVClusterView* V, d->m_views) {
        V->setData(Mda());
    }
    double max_abs_val = 0;
    int NN = X.totalSize();
    for (int i = 0; i < NN; i++) {
        if (fabs(X.get(i)) > max_abs_val)
            max_abs_val = fabs(X.get(i));
    }
    if (max_abs_val) {
        AffineTransformation T;
        T.setIdentity();
        double factor = 1 / 1.2;
        T.scale(1.0 / max_abs_val * factor, 1.0 / max_abs_val * factor, 1.0 / max_abs_val * factor);
        this->setTransformation(T);
    }
    d->set_data_on_visible_views();
}

void MVClusterWidget::setTimes(const QList<double>& times)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setTimes(times);
    }
}

void MVClusterWidget::setLabels(const QList<int>& labels)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setLabels(labels);
    }
}

void MVClusterWidget::setAmplitudes(const QList<double>& amps)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setAmplitudes(amps);
    }
}

void MVClusterWidget::setScores(const QList<double>& detectability_scores, const QList<double>& outlier_scores)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setScores(detectability_scores, outlier_scores);
    }
}

void MVClusterWidget::slot_current_event_changed()
{
    foreach (MVClusterView* V, d->m_views) {
        V->setCurrentEvent(viewAgent()->currentEvent());
    }
    d->update_clips_view();
}

void MVClusterWidget::setLabelsToUse(const QList<int>& labels)
{
    d->m_labels_to_use = labels;
    this->recalculate();
}

void MVClusterWidget::setFeatureMode(QString mode)
{
    d->m_feature_mode = mode;
    this->recalculate();
}

void MVClusterWidget::setChannels(QList<int> channels)
{
    d->m_channels = channels;
    this->recalculate();
}

void MVClusterWidget::setTransformation(const AffineTransformation& T)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setTransformation(T);
    }
}

void MVClusterWidget::slot_view_current_event_changed()
{
    MVClusterView* V0 = (MVClusterView*)sender();
    viewAgent()->setCurrentEvent(V0->currentEvent());
}

void MVClusterWidget::slot_view_transformation_changed()
{
    MVClusterView* V0 = (MVClusterView*)sender();
    AffineTransformation T = V0->transformation();
    foreach (MVClusterView* V, d->m_views) {
        V->setTransformation(T);
    }
}

void MVClusterWidget::slot_view_active_cluster_number_toggled()
{
    /// TODO (LOW) replace all these casts with qobject_cast's
    MVClusterView* V0 = (MVClusterView*)sender();
    QSet<int> active_cluster_numbers=V0->activeClusterNumbers();
    foreach (MVClusterView* V, d->m_views) {
        V->setActiveClusterNumbers(active_cluster_numbers);
    }
}

void MVClusterWidget::slot_show_clip_view_toggled(bool val)
{
    d->m_clips_view->setVisible(val);
}

void MVClusterWidget::slot_show_view_toggled(bool val)
{
    int index = sender()->property("view_index").toInt();
    if ((index >= 0) && (index < d->m_views.count())) {
        d->m_views[index]->setVisible(val);
    }
    d->set_data_on_visible_views();
}

void MVClusterWidgetPrivate::connect_view(MVClusterView* V)
{
    QObject::connect(V, SIGNAL(currentEventChanged()), q, SLOT(slot_view_current_event_changed()));
    QObject::connect(V, SIGNAL(transformationChanged()), q, SLOT(slot_view_transformation_changed()));
    QObject::connect(V, SIGNAL(activeClusterNumberToggled()),q,SLOT(slot_view_active_cluster_number_toggled()));
}

void MVClusterWidgetPrivate::update_clips_view()
{
    /// TODO: (HIGH) -- restore functionality in MVClusterWidget, click on current point
    QMessageBox::information(q, "Feature disabled", "This feature has been temporarily disabled. Normally you would see the current clip on the left.");
    /*
    MVEvent evt = q->currentEvent();
    QString info_txt;
    if (evt.time >= 0) {
        QList<double> times;
        times << evt.time;
        long hold_size = m_timeseries.downloadChunkSize();
        m_timeseries.setDownloadChunkSize(m_clip_size * m_timeseries.N1() * 5); //don't create chunks that are too big -- no need to download too much since we only want one clip
        Mda clip0 = extract_clips(m_timeseries, times, m_clip_size);
        m_timeseries.setDownloadChunkSize(hold_size); //now put it back
        double ppp = m_outlier_scores.value(current_event_index());
        if (ppp) {
            info_txt = QString("Outlier score: %1").arg(ppp);
        }
        m_clips_view->setClips(clip0);
    } else {
        m_clips_view->setClips(Mda());
    }
    m_info_bar->setText(info_txt);
    */
}

int MVClusterWidgetPrivate::current_event_index()
{
    if (m_views.isEmpty())
        return 0;
    return m_views[0]->currentEventIndex();
}

void MVClusterWidgetPrivate::set_data_on_visible_views()
{
    foreach (MVClusterView* V, m_views) {
        if (V->isVisible()) {
            V->setData(m_data);
        }
    }
}

void MVClusterWidgetComputer::compute()
{
    TaskProgress task(TaskProgress::Calculate, "MVClusterWidgetComputer");

    //firings = compute_filtered_firings_remotely(mlproxy_url, firings, filter);
    firings = compute_filtered_firings_remotely(mlproxy_url, firings, filter);

    QString firings_out_path;
    {
        QString labels_str;
        foreach (int x, labels_to_use) {
            if (!labels_str.isEmpty())
                labels_str += ",";
            labels_str += QString("%1").arg(x);
        }

        MountainProcessRunner MT;
        QString processor_name = "mv_subfirings";
        MT.setProcessorName(processor_name);

        task.log(QString("firings = %1").arg(firings.makePath()));

        QMap<QString, QVariant> params;
        params["firings"] = firings.makePath();
        params["labels"] = labels_str;
        MT.setInputParameters(params);
        //MT.setMscmdServerUrl(mscmdserver_url);
        MT.setMLProxyUrl(mlproxy_url);

        firings_out_path = MT.makeOutputFilePath("firings_out");

        MT.runProcess();
    }

    if (thread_interrupt_requested()) {
        return;
    }

    QString features_path;
    if (features_mode == "pca") {
        MountainProcessRunner MT;
        QString processor_name = "extract_clips_features";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["timeseries"] = timeseries.makePath();
        params["firings"] = firings_out_path;
        params["clip_size"] = clip_size;
        params["num_features"] = 3;
        MT.setInputParameters(params);
        //MT.setMscmdServerUrl(mscmdserver_url);
        MT.setMLProxyUrl(mlproxy_url);

        features_path = MT.makeOutputFilePath("features");

        MT.runProcess();

        if (thread_interrupt_requested()) {
            return;
        }
    }
    else if (features_mode == "channels") {
        MountainProcessRunner MT;
        QString processor_name = "extract_channel_values";
        MT.setProcessorName(processor_name);

        QStringList channels_strlist;
        foreach (int ch, channels) {
            channels_strlist << QString("%1").arg(ch);
        }

        QMap<QString, QVariant> params;
        params["timeseries"] = timeseries.makePath();
        params["firings"] = firings_out_path;
        params["channels"] = channels_strlist.join(",");
        MT.setInputParameters(params);
        MT.setMLProxyUrl(mlproxy_url);

        features_path = MT.makeOutputFilePath("values");

        MT.runProcess();

        if (thread_interrupt_requested()) {
            return;
        }
    }
    else {
        TaskProgress err("Computing features");
        err.error("Unrecognized features mode: " + features_mode);
        return;
    }
    DiskReadMda F(firings_out_path);

    times.clear();
    labels.clear();
    amplitudes.clear();
    outlier_scores.clear();
    detectability_scores.clear();
    for (long j = 0; j < F.N2(); j++) {
        times << F.value(1, j);
        labels << (int)F.value(2, j);
        amplitudes << F.value(3, j);
        outlier_scores << F.value(4, j);
        detectability_scores << F.value(5, j);
    }

    if (thread_interrupt_requested()) {
        return;
    }

    DiskReadMda features(features_path);
    features.setRemoteDataType("float32");
    features.readChunk(data, 0, 0, features.N1(), features.N2());
}
