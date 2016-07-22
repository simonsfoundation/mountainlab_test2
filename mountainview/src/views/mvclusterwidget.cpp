#include "mvclusterwidget.h"
#include "mvclusterview.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <taskprogress.h>
#include "mvclipsview.h"
#include "mlcommon.h"
#include <QAction>
#include <QSettings>
#include <QThread>
#include <math.h>
#include "mountainprocessrunner.h"

/// TODO: (MEDIUM) control brightness in 3D density view

class MVClusterWidgetComputer {
public:
    //input
    QString mlproxy_url;
    MVEventFilter filter;
    DiskReadMda timeseries;
    DiskReadMda firings;
    int clip_size;
    QList<int> labels_to_use;
    QString features_mode; //"pca" or "channels"
    QVector<int> channels; //in case of feature_mode=="channels"

    //output
    Mda data;
    QVector<double> times;
    QVector<int> labels;
    QVector<double> amplitudes;
    QVector<double> detectability_scores;
    QVector<double> outlier_scores;

    void compute();
};

class ClipsViewThread : public QThread {
public:
    //input
    DiskReadMda timeseries;
    QVector<double> times;
    int clip_size;

    //output
    DiskReadMda clips;

    void run();
};

class MVClusterWidgetPrivate {
public:
    MVClusterWidget* q;
    QList<MVClusterView*> m_views;
    MVClipsView* m_clips_view;
    QLabel* m_info_bar;
    Mda m_data;
    QList<int> m_labels_to_use;
    QVector<double> m_outlier_scores;
    MVClusterWidgetComputer m_computer;
    QString m_feature_mode;
    QVector<int> m_channels;
    ClipsViewThread m_clips_view_thread;

    void connect_view(MVClusterView* V);
    void update_clips_view();
    int current_event_index();
    void set_data_on_visible_views();
};

MVClusterWidget::MVClusterWidget(MVContext* context)
    : MVAbstractView(context)
{
    d = new MVClusterWidgetPrivate;
    d->q = this;

    d->m_clips_view = new MVClipsView(context);

    {
        MVClusterView* X = new MVClusterView(context);
        X->setMode(MVCV_MODE_HEAT_DENSITY);
        d->m_views << X;
    }
    {
        MVClusterView* X = new MVClusterView(context);
        X->setMode(MVCV_MODE_LABEL_COLORS);
        d->m_views << X;
    }
    {
        MVClusterView* X = new MVClusterView(context);
        X->setMode(MVCV_MODE_TIME_COLORS);
        X->setVisible(false);
        d->m_views << X;
    }
    {
        MVClusterView* X = new MVClusterView(context);
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
        connect(CB, SIGNAL(clicked(bool)), this, SLOT(slot_show_clip_view_clicked()));
        CB->setChecked(true);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Density Plot");
        CB->setProperty("view_index", 0);
        connect(CB, SIGNAL(clicked(bool)), this, SLOT(slot_show_view_clicked()));
        CB->setChecked(true);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Label Colors");
        CB->setProperty("view_index", 1);
        connect(CB, SIGNAL(clicked(bool)), this, SLOT(slot_show_view_clicked()));
        CB->setChecked(true);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Time Colors");
        CB->setProperty("view_index", 2);
        connect(CB, SIGNAL(clicked(bool)), this, SLOT(slot_show_view_clicked()));
        CB->setChecked(false);
        bottom_panel->addWidget(CB);
    }
    {
        QCheckBox* CB = new QCheckBox;
        CB->setText("Amplitude Colors");
        CB->setProperty("view_index", 3);
        connect(CB, SIGNAL(clicked(bool)), this, SLOT(slot_show_view_clicked()));
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

    this->recalculateOn(context, SIGNAL(clusterMergeChanged()));
    this->recalculateOn(context, SIGNAL(currentTimeseriesChanged()));
    this->recalculateOn(context, SIGNAL(filteredFiringsChanged()));
    this->recalculateOnOptionChanged("clip_size");
    onOptionChanged("cluster_color_index_shift", this, SLOT(onCalculationFinished()));

    connect(context, SIGNAL(currentEventChanged()), this, SLOT(slot_current_event_changed()));

    connect(&d->m_clips_view_thread, SIGNAL(finished()), this, SLOT(slot_clips_view_thread_finished()));

    {
        QAction* A = new QAction("<-Colors", this);
        A->setProperty("action_type", "toolbar");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_shift_colors_left()));
        this->addAction(A);
    }
    {
        QAction* A = new QAction("Colors->", this);
        A->setProperty("action_type", "toolbar");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_shift_colors_right()));
        this->addAction(A);
    }
}

MVClusterWidget::~MVClusterWidget()
{
    this->stopCalculation();
    delete d;
}

void MVClusterWidget::prepareCalculation()
{
    d->m_computer.mlproxy_url = mvContext()->mlProxyUrl();
    d->m_computer.filter = mvContext()->eventFilter();
    d->m_computer.timeseries = mvContext()->currentTimeseries();
    d->m_computer.firings = mvContext()->firings();
    d->m_computer.clip_size = mvContext()->option("clip_size").toInt();
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
    QVector<int> merged_labels = d->m_computer.labels;
    if (mvContext()->viewMerged()) {
        merged_labels = this->mvContext()->clusterMerge().mapLabels(merged_labels);
    }

    this->setTimes(d->m_computer.times);
    this->setLabels(merged_labels);
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

void MVClusterWidget::setTimes(const QVector<double>& times)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setTimes(times);
    }
}

void MVClusterWidget::setLabels(const QVector<int>& labels)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setLabels(labels);
    }
}

void MVClusterWidget::setAmplitudes(const QVector<double>& amps)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setAmplitudes(amps);
    }
}

void MVClusterWidget::setScores(const QVector<double>& detectability_scores, const QVector<double>& outlier_scores)
{
    foreach (MVClusterView* V, d->m_views) {
        V->setScores(detectability_scores, outlier_scores);
    }
}

void MVClusterWidget::slot_current_event_changed()
{
    foreach (MVClusterView* V, d->m_views) {
        V->setCurrentEvent(mvContext()->currentEvent());
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

void MVClusterWidget::setChannels(QVector<int> channels)
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
    mvContext()->setCurrentEvent(V0->currentEvent());
}

void MVClusterWidget::slot_view_transformation_changed()
{
    MVClusterView* V0 = (MVClusterView*)sender();
    AffineTransformation T = V0->transformation();
    foreach (MVClusterView* V, d->m_views) {
        V->setTransformation(T);
    }
}

void MVClusterWidget::slot_view_active_cluster_numbers_changed()
{
    /// TODO (LOW) replace all these casts with qobject_cast's
    MVClusterView* V0 = (MVClusterView*)sender();
    QSet<int> active_cluster_numbers = V0->activeClusterNumbers();
    foreach (MVClusterView* V, d->m_views) {
        V->setActiveClusterNumbers(active_cluster_numbers);
    }
}

void MVClusterWidget::slot_show_clip_view_clicked()
{
    bool val = ((QCheckBox*)sender())->isChecked();
    d->m_clips_view->setVisible(val);
}

void MVClusterWidget::slot_show_view_clicked()
{
    int index = sender()->property("view_index").toInt();
    bool val = ((QCheckBox*)sender())->isChecked();
    if ((index >= 0) && (index < d->m_views.count())) {
        d->m_views[index]->setVisible(val);
    }
    d->set_data_on_visible_views();
}

void MVClusterWidget::slot_clips_view_thread_finished()
{
    if (d->m_clips_view_thread.isRunning()) {
        //we want to be careful
        qWarning() << "m_clips_view_thread is running even though we are in its finished slot!!!!!!!!!";
        return;
    }
    d->m_clips_view->setClips(d->m_clips_view_thread.clips);
}

void MVClusterWidget::slot_shift_colors_left(int step)
{
    int shift = this->mvContext()->option("cluster_color_index_shift", 0).toInt();
    shift += step;
    this->mvContext()->setOption("cluster_color_index_shift", shift);
}

void MVClusterWidget::slot_shift_colors_right()
{
    slot_shift_colors_left(-1);
}

void MVClusterWidgetPrivate::connect_view(MVClusterView* V)
{
    QObject::connect(V, SIGNAL(currentEventChanged()), q, SLOT(slot_view_current_event_changed()));
    QObject::connect(V, SIGNAL(transformationChanged()), q, SLOT(slot_view_transformation_changed()));
    QObject::connect(V, SIGNAL(activeClusterNumbersChanged()), q, SLOT(slot_view_active_cluster_numbers_changed()));
}

void MVClusterWidgetPrivate::update_clips_view()
{
    MVEvent evt = q->mvContext()->currentEvent();
    QString info_txt;
    if (evt.time >= 0) {
        QVector<double> times;
        times << evt.time;

        m_clips_view->setClips(Mda());
        if (m_clips_view_thread.isRunning()) {
            m_clips_view_thread.requestInterruption();
            m_clips_view_thread.wait();
        }

        m_clips_view_thread.timeseries = q->mvContext()->currentTimeseries();
        m_clips_view_thread.clip_size = q->mvContext()->option("clip_size").toInt();
        m_clips_view_thread.times = times;

        m_clips_view_thread.start();
    }
    else {
        m_clips_view->setClips(Mda());
    }
    //m_info_bar->setText(info_txt);
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
        //if (V->isVisible()) { // not sure why this condition is not working on initial load
        V->setData(m_data);
        //}
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

    if (MLUtil::threadInterruptRequested()) {
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

        if (MLUtil::threadInterruptRequested()) {
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

        if (MLUtil::threadInterruptRequested()) {
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

    if (MLUtil::threadInterruptRequested()) {
        return;
    }

    DiskReadMda features(features_path);
    features.setRemoteDataType("float32");
    features.readChunk(data, 0, 0, features.N1(), features.N2());
}

MVPCAFeaturesFactory::MVPCAFeaturesFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
    connect(context, SIGNAL(selectedClustersChanged()),
        this, SLOT(updateEnabled()));
    updateEnabled();
}

QString MVPCAFeaturesFactory::id() const
{
    return QStringLiteral("open-pca-features");
}

QString MVPCAFeaturesFactory::name() const
{
    return tr("PCA Features");
}

QString MVPCAFeaturesFactory::title() const
{
    return tr("PCA features");
}

MVAbstractView* MVPCAFeaturesFactory::createView(QWidget* parent)
{
    Q_UNUSED(parent)
    QList<int> ks = mvContext()->selectedClusters();
    if (ks.isEmpty())
        ks = mvContext()->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.count() == 0) {
        QMessageBox::information(0, "Unable to open clusters", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    MVClusterWidget* X = new MVClusterWidget(mvContext());
    X->setLabelsToUse(ks);
    X->setFeatureMode("pca");
    return X;
}

void MVPCAFeaturesFactory::updateEnabled()
{
    setEnabled(!mvContext()->selectedClusters().isEmpty());
}

MVChannelFeaturesFactory::MVChannelFeaturesFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
    connect(context, SIGNAL(selectedClustersChanged()),
        this, SLOT(updateEnabled()));
    updateEnabled();
}

QString MVChannelFeaturesFactory::id() const
{
    return QStringLiteral("open-channel-features");
}

QString MVChannelFeaturesFactory::name() const
{
    return tr("Channel Features");
}

QString MVChannelFeaturesFactory::title() const
{
    return tr("Ch. features");
}

MVAbstractView* MVChannelFeaturesFactory::createView(QWidget* parent)
{
    Q_UNUSED(parent)
    QSettings settings("SCDA", "MountainView");
    QString str = settings.value("open_channel_features_channels", "1,2,3").toString();
    str = QInputDialog::getText(0, "Open Channel Features", "Channels:", QLineEdit::Normal, str);
    if (str.isEmpty())
        return Q_NULLPTR;
    QStringList strlist = str.split(",", QString::SkipEmptyParts);
    QVector<int> channels;
    foreach (QString a, strlist) {
        bool ok;
        channels << a.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(0, "Open channel features", "Invalid channels string");
        }
    }
    settings.setValue("open_channel_features_channels", strlist.join(","));

    QList<int> ks = mvContext()->selectedClusters();
    if (ks.isEmpty())
        ks = mvContext()->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.isEmpty()) {
        QMessageBox::warning(0, "Unable to open clusters", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    MVClusterWidget* X = new MVClusterWidget(mvContext());
    X->setLabelsToUse(ks);
    X->setFeatureMode("channels");
    X->setChannels(channels);
    return X;
}

void MVChannelFeaturesFactory::updateEnabled()
{
    setEnabled(!mvContext()->selectedClusters().isEmpty());
}

#include "extract_clips.h"
void ClipsViewThread::run()
{
    timeseries.setRemoteDataType("float32");
    //use a small chunk size so we don't end up downloading too much extra
    timeseries.setDownloadChunkSize(clip_size * timeseries.N1() * 4 * 3);
    clips = extract_clips(timeseries, times, clip_size);
}
