#include "mvmainwindow.h"
#include "diskreadmda.h"
#include "sstimeseriesview.h"
#include "sstimeserieswidget.h"
#include "mvcrosscorrelogramswidget2.h"
#ifdef USE_LAPACK
#include "get_pca_features.h"
#else
#include "get_principal_components.h"
#endif
#include "get_sort_indices.h"
#include "mvclusterdetailwidget.h"
#include "mvclipsview.h"
#include "mvclusterwidget.h"
#include "mvspikesprayview.h"
#include "mvfiringeventview.h"
#include "mvfiringeventview2.h"
#include "extract_clips.h"
#include "tabber.h"
#include "computationthread.h"
#include "mountainprocessrunner.h"
#include "mvclipswidget.h"
#include "taskprogressview.h"
#include "mvcontrolpanel.h"
#include "taskprogress.h"
#include "clustermerge.h"
#include "mvviewagent.h"
#include "mvstatusbar.h"
//#include "mvtimeseriesview.h"
#include "mvtimeseriesview2.h"
#include "mlutils.h"
#include "mvfile.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QTime>
#include <QTimer>
#include <math.h>
#include <QProgressDialog>
#include "msmisc.h"
#include "mvutils.h"
#include <QColor>
#include <QStringList>
#include <QSet>
#include <QKeyEvent>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>
#include <QAbstractButton>
#include <QSettings>
#include <QScrollArea>
#include <toolbuttonmenu.h>
#include <QAction>
#include "textfile.h"

/// TODO put styles in central place?
#define MV_STATUS_BAR_HEIGHT 30

class MVMainWindowPrivate {
public:
    MVMainWindow* q;
    QList<Epoch> m_epochs; //not implemented yet -- put it in mvviewagent
    MVFile m_mv_file; //we need to keep this in case there is other data in the .json that we want to preserver
    MVViewAgent* m_view_agent; //gets passed to all the views and the control panel

    //these widgets go on the left
    QSplitter* m_left_splitter;
    MVControlPanel* m_control_panel;
    TaskProgressView* m_task_progress_view;

    QSplitter* m_hsplitter, *m_vsplitter;
    TabberTabWidget* m_tabs1, *m_tabs2;
    Tabber* m_tabber; //manages the views in the two tab widgets

    /// TODO put m_colors somewhere else (call it style colors?)
    QMap<QString, QColor> m_colors;

    void update_sizes(); //update sizes of all the widgets when the main window is resized
    void add_tab(QWidget* W, QString label);
    void set_tool_button_menu(QWidget* X);

    MVCrossCorrelogramsWidget2* open_auto_correlograms();
    MVCrossCorrelogramsWidget2* open_cross_correlograms(int k);
    MVCrossCorrelogramsWidget2* open_matrix_of_cross_correlograms();
    MVClusterDetailWidget* open_cluster_details();
    void open_timeseries();
    void open_clips();
    void open_pca_features();
    void open_channel_features();
    void open_spike_spray();
    void open_firing_events();
    /// TODO implement find_nearby_events
    //void find_nearby_events();

    void annotate_selected();
    void merge_selected();
    void unmerge_selected();

    /// TODO get rid of setting times and labels for views (do this internally based on the MVViewAgent)
    void set_times_labels_for_mvtimeseriesview(MVTimeSeriesView2* WW);
    void set_times_labels_for_timeseries_widget(SSTimeSeriesWidget* WW);

    TabberTabWidget* tab_widget_of(QWidget* W);

    void export_mountainview_document();
    void export_original_firings();
    void export_filtered_firings();
    void export_file(QString source_path, QString dest_path, bool use_float64);

    //not sure about these
    QVariant get_cluster_attribute(int k, QString attr);
    void set_cluster_attribute(int k, QString attr, QVariant val);
    void set_button_enabled(QString name, bool val);
};

MVMainWindow::MVMainWindow(MVViewAgent* view_agent, QWidget* parent)
    : QWidget(parent)
{
    d = new MVMainWindowPrivate;
    d->q = this;

    d->m_view_agent = view_agent;

    d->m_control_panel = new MVControlPanel(view_agent);
    //probably get rid of the following line
    connect(d->m_control_panel, SIGNAL(userAction(QString)), this, SLOT(slot_control_panel_user_action(QString)));

    QSplitter* hsplitter = new QSplitter;
    hsplitter->setOrientation(Qt::Horizontal);
    d->m_hsplitter = hsplitter;

    QSplitter* vsplitter = new QSplitter;
    vsplitter->setOrientation(Qt::Vertical);
    d->m_vsplitter = vsplitter;

    //scroll area for control panel
    QScrollArea* CP = new QScrollArea;
    CP->setWidget(d->m_control_panel);
    CP->setWidgetResizable(true);

    d->m_task_progress_view = new TaskProgressView;

    QSplitter* left_splitter = new QSplitter(Qt::Vertical);
    left_splitter->addWidget(CP);
    left_splitter->addWidget(d->m_task_progress_view);
    d->m_left_splitter = left_splitter;

    hsplitter->addWidget(left_splitter);
    hsplitter->addWidget(vsplitter);

    d->m_tabber = new Tabber;
    d->m_tabs2 = d->m_tabber->createTabWidget("south");
    d->m_tabs1 = d->m_tabber->createTabWidget("north");

    vsplitter->addWidget(d->m_tabs1);
    vsplitter->addWidget(d->m_tabs2);

    MVStatusBar* status_bar = new MVStatusBar();
    status_bar->setFixedHeight(MV_STATUS_BAR_HEIGHT);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addWidget(hsplitter);
    vlayout->addWidget(status_bar);
    this->setLayout(vlayout);

    //update which buttons are enabled/disabled
    slot_update_buttons();
    connect(view_agent, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(clusterAttributesChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(clusterMergeChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(currentEventChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(currentTimepointChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_buttons()));
}

MVMainWindow::~MVMainWindow()
{
    delete d;
}

void MVMainWindow::setDefaultInitialization()
{
    d->open_cluster_details();
    d->m_tabber->switchCurrentContainer();
    d->open_auto_correlograms();
}

void MVMainWindow::setEpochs(const QList<Epoch>& epochs)
{
    d->m_epochs = epochs; //not used for now
}

void MVMainWindow::setClusterMerge(ClusterMerge CM)
{
    d->m_view_agent->setClusterMerge(CM);
}

void MVMainWindow::setMVFile(MVFile ff)
{
    d->m_mv_file = ff; //we need to save the whole thing so we know what to save

    /// TODO need to clear out the view_agent here in case this gets called twice
    QStringList timeseries_names = d->m_mv_file.timeseriesNames();

    foreach (QString name, timeseries_names) {
        d->m_view_agent->addTimeseries(name, DiskReadMda(d->m_mv_file.timeseriesPathResolved(name)));
    }

    d->m_view_agent->setOption("clip_size", d->m_mv_file.viewOptions()["clip_size"].toInt());
    d->m_view_agent->setOption("cc_max_dt_msec", d->m_mv_file.viewOptions()["cc_max_dt_msec"].toDouble());

    /// TODO fix this
    d->m_control_panel->setEventFilter(MVEventFilter::fromJsonObject(d->m_mv_file.eventFilter()));
    if (!d->m_mv_file.currentTimeseriesName().isEmpty()) {
        d->m_view_agent->setCurrentTimeseriesName(d->m_mv_file.currentTimeseriesName());
    }
    else {
        d->m_view_agent->setCurrentTimeseriesName(timeseries_names.value(0));
    }

    {
        QJsonObject ann0 = d->m_mv_file.annotations();
        if (ann0.contains("cluster_attributes")) {
            QJsonObject obj2 = ann0["cluster_attributes"].toObject();
            QMap<int, QJsonObject> CA;
            QStringList keys = obj2.keys();
            foreach (QString key, keys) {
                bool ok;
                int num = key.toInt(&ok);
                if (ok) {
                    CA[num] = obj2[key].toObject();
                }
            }
            d->m_view_agent->setClusterAttributes(CA);
        }
        else
            d->m_view_agent->setClusterAttributes(QMap<int, QJsonObject>());
        if (ann0.contains("cluster_merge")) {
            QJsonArray CM = ann0["cluster_merge"].toArray();
            QString json = QJsonDocument(CM).toJson(QJsonDocument::Compact);
            d->m_view_agent->setClusterMerge(ClusterMerge::fromJson(json));
        }
        else {
            d->m_view_agent->setClusterMerge(ClusterMerge());
        }
    }

    d->m_view_agent->setFirings(DiskReadMda(ff.firingsPathResolved()));
    d->m_view_agent->setSampleRate(ff.sampleRate());
    d->m_view_agent->setMLProxyUrl(ff.mlproxyUrl());
}

void MVMainWindow::writeMVFile(const QString& mv_fname)
{
    TaskProgress task("saving .mv file: " + mv_fname);

    QJsonObject view_options;
    view_options["clip_size"] = d->m_view_agent->option("clip_size").toInt();
    view_options["cc_max_dt_msec"] = d->m_view_agent->option("cc_max_dt_msec").toDouble();
    d->m_mv_file.setEventFilter(d->m_control_panel->eventFilter().toJsonObject());
    d->m_mv_file.setCurrentTimeseriesName(d->m_view_agent->currentTimeseriesName());

    QJsonObject cluster_attributes;
    QMap<int, QJsonObject> CA = d->m_view_agent->clusterAttributes();
    {
        QList<int> keys = CA.keys();
        foreach (int key, keys) {
            if (!CA[key].isEmpty())
                cluster_attributes[QString("%1").arg(key)] = CA[key];
        }
    }

    QJsonObject annotations;
    annotations["cluster_attributes"] = cluster_attributes;
    annotations["cluster_merge"] = QJsonDocument::fromJson(d->m_view_agent->clusterMerge().toJson().toLatin1()).array();

    d->m_mv_file.setAnnotations(annotations);
    d->m_mv_file.setSampleRate(d->m_view_agent->sampleRate());

    if (!d->m_mv_file.write(mv_fname)) {
        task.error("Error writing .mv file: " + mv_fname);
    }
}

void MVMainWindow::resizeEvent(QResizeEvent* evt)
{
    Q_UNUSED(evt)
    d->update_sizes();
}

void MVMainWindow::keyPressEvent(QKeyEvent* evt)
{
    if ((evt->key() == Qt::Key_W) && (evt->modifiers() & Qt::ControlModifier)) {
        this->close();
    }
    else if (evt->key() == Qt::Key_A) {
        d->annotate_selected();
    }
    else if (evt->key() == Qt::Key_M) {
        d->merge_selected();
    }
    else if (evt->key() == Qt::Key_U) {
        d->unmerge_selected();
    }
    else
        evt->ignore();
}

void MVMainWindow::slot_control_panel_user_action(QString str)
{
    if (str == "open-cluster-details") {
        d->open_cluster_details();
    }
    else if (str == "open-auto-correlograms") {
        d->open_auto_correlograms();
    }
    else if (str == "open-matrix-of-cross-correlograms") {
        d->open_matrix_of_cross_correlograms();
    }
    else if (str == "open-timeseries-data") {
        d->open_timeseries();
    }
    else if (str == "open-clips") {
        d->open_clips();
    }
    else if (str == "open-pca-features") {
        d->open_pca_features();
    }
    else if (str == "open-channel-features") {
        d->open_channel_features();
    }
    else if (str == "open-spike-spray") {
        d->open_spike_spray();
    }
    else if (str == "open-firing-events") {
        d->open_firing_events();
    }
    else if (str == "annotate_selected") {
        d->annotate_selected();
    }
    else if (str == "merge_selected") {
        d->merge_selected();
    }
    else if (str == "unmerge_selected") {
        d->unmerge_selected();
    }
    else if (str == "export_mountainview_document") {
        d->export_mountainview_document();
    }
    else if (str == "export_original_firings") {
        d->export_original_firings();
    }
    else if (str == "export_filtered_firings") {
        d->export_filtered_firings();
    }
}

void MVMainWindow::slot_auto_correlogram_activated()
{
    TabberTabWidget* TW = d->tab_widget_of((QWidget*)sender());
    d->m_tabber->setCurrentContainer(TW);
    d->m_tabber->switchCurrentContainer();
    d->open_cross_correlograms(d->m_view_agent->currentCluster());
}

void MVMainWindow::slot_details_template_activated()
{
    int k = d->m_view_agent->currentCluster();
    if (k < 0)
        return;
    TabberTabWidget* TW = d->tab_widget_of((QWidget*)sender());
    d->m_tabber->setCurrentContainer(TW);
    d->m_tabber->switchCurrentContainer();
    d->open_clips();
}

void MVMainWindow::slot_update_buttons()
{
    //bool has_peaks = (d->m_firings.value(0, 3) != 0); //for now we just test the very first one (might be problematic)
    /// TODO (0.9.1) restore this has_peaks without accessing m_firings in gui thread
    bool has_peaks = true;
    bool something_selected = (!d->m_view_agent->selectedClusters().isEmpty());

    d->set_button_enabled("open-cluster-details", true);
    d->set_button_enabled("open-auto-correlograms", true);
    d->set_button_enabled("open-matrix-of-cross-correlograms", something_selected);
    d->set_button_enabled("open-timeseries-data", true);
    d->set_button_enabled("open-clips", something_selected);
    d->set_button_enabled("open-clusters", something_selected);
    d->set_button_enabled("open-spike-spray", something_selected);
    d->set_button_enabled("open-firing-events", (something_selected) && (has_peaks));
    d->set_button_enabled("find-nearby-events", d->m_view_agent->selectedClusters().count() >= 2);

    d->set_button_enabled("annotate_selected", something_selected);
    d->set_button_enabled("merge_selected", d->m_view_agent->selectedClusters().count() >= 2);
    d->set_button_enabled("unmerge_selected", something_selected);
    d->set_button_enabled("export_mountainview_document", true);
    d->set_button_enabled("export_original_firings", true);
    d->set_button_enabled("export_filtered_firings", true);
}

/*
void MVMainWindow::slot_calculator_finished()
{
    //d->update_cross_correlograms();
    //d->update_cluster_details();
    //d->update_timeseries_views();
    d->m_firings = d->m_calculator.m_firings;
    d->m_original_cluster_numbers = d->m_calculator.m_original_cluster_numbers;
    d->m_original_cluster_offsets = d->m_original_cluster_offsets;
    d->update_all_widgets();
    d->m_view_agent->setSelectedClusters(QList<int>());
    slot_update_buttons();
}
*/

void MVMainWindow::slot_action_move_to_other_tab_widget()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (!a)
        return;
    QWidget* W = a->parentWidget();
    if (!W)
        return;
    d->m_tabber->moveWidgetToOtherContainer(W);
}

void MVMainWindow::slot_pop_out_widget()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (!a)
        return;
    QWidget* W = a->parentWidget();
    if (!W)
        return;
    d->m_tabber->popOutWidget(W);
}

void MVMainWindowPrivate::update_sizes()
{
    float W0 = q->width();
    float H0 = q->height() - MV_STATUS_BAR_HEIGHT;

    int W1 = W0 / 3;
    if (W1 < 150)
        W1 = 150;
    if (W1 > 400)
        W1 = 400;
    int W2 = W0 - W1;

    int H1 = H0 / 2;
    int H2 = H0 / 2;
    //int H3=H0-H1-H2;

    {
        QList<int> sizes;
        sizes << W1 << W2;
        m_hsplitter->setSizes(sizes);
    }
    {
        QList<int> sizes;
        sizes << H1 << H2;
        m_vsplitter->setSizes(sizes);
    }
    {
        int tv_height;
        if (H0 > 1200) {
            tv_height = 300;
        }
        if (H0 > 900) {
            tv_height = 200;
        }
        else {
            tv_height = 100;
        }
        int cp_height = H0 - tv_height;
        QList<int> sizes;
        sizes << cp_height << tv_height;
        m_left_splitter->setSizes(sizes);
    }
}

void MVMainWindowPrivate::add_tab(QWidget* W, QString label)
{
    W->setFocusPolicy(Qt::StrongFocus);
    m_tabber->addWidget(m_tabber->currentContainerName(), label, W);
    W->setProperty("tab_label", label); //won't be needed in future, once Tabber is fully implemented
}

void MVMainWindowPrivate::set_tool_button_menu(QWidget* X)
{
    X->setContextMenuPolicy(Qt::ActionsContextMenu);
    ToolButtonMenu* MM = new ToolButtonMenu(X);
    MM->setOffset(QSize(4, 4));
    QToolButton* tb = MM->activateOn(X);
    tb->setIconSize(QSize(48, 48));
    QIcon icon(":images/gear.png");
    tb->setIcon(icon);

    /// TODO (LOW) add separator action
    {
        /// TODO (LOW) change text to "Pop in" after popped out (slot will be same)
        QAction* a = new QAction("Move to other tab widget", X);
        QObject::connect(a, SIGNAL(triggered(bool)), q, SLOT(slot_action_move_to_other_tab_widget()));
        /// Witold, I'd really like to insert this at index zero, but the qt api makes that a bit annoying because i need to find the first QAction*, which may not exist. Pls help.
        X->addAction(a);
    }
    {
        /// TODO (LOW) disable action when popped out
        QAction* a = new QAction("Pop out", X);
        QObject::connect(a, SIGNAL(triggered(bool)), q, SLOT(slot_pop_out_widget()));
        X->addAction(a);
    }
}

MVCrossCorrelogramsWidget2* MVMainWindowPrivate::open_auto_correlograms()
{
    MVCrossCorrelogramsWidget2* X = new MVCrossCorrelogramsWidget2(m_view_agent);
    CrossCorrelogramOptions opts;
    opts.mode = "all_auto_correlograms";
    X->setOptions(opts);
    X->setProperty("widget_type", "auto_correlograms"); //not really needed
    add_tab(X, "Auto-Correlograms");
    QObject::connect(X, SIGNAL(histogramActivated()), q, SLOT(slot_auto_correlogram_activated()));
    return X;
}

MVCrossCorrelogramsWidget2* MVMainWindowPrivate::open_cross_correlograms(int k)
{
    MVCrossCorrelogramsWidget2* X = new MVCrossCorrelogramsWidget2(m_view_agent);
    CrossCorrelogramOptions opts;
    opts.mode = "cross_correlograms";
    opts.ks << k;
    X->setOptions(opts);
    X->setProperty("widget_type", "cross_correlograms"); //not really needed
    X->setProperty("kk", k); //not really needed
    QString str = QString("CC for %1").arg(k);
    add_tab(X, str);
    return X;
}

MVCrossCorrelogramsWidget2* MVMainWindowPrivate::open_matrix_of_cross_correlograms()
{
    MVCrossCorrelogramsWidget2* X = new MVCrossCorrelogramsWidget2(m_view_agent);
    X->setProperty("widget_type", "matrix_of_cross_correlograms");
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    CrossCorrelogramOptions opts;
    opts.mode = "matrix_of_cross_correlograms";
    opts.ks = ks;
    X->setOptions(opts);
    add_tab(X, QString("CC Matrix"));
    return X;
}

MVClusterDetailWidget* MVMainWindowPrivate::open_cluster_details()
{
    /// TODO move sample rate into mvviewagent
    MVClusterDetailWidget* X = new MVClusterDetailWidget(m_view_agent);
    set_tool_button_menu(X);
    QObject::connect(X, SIGNAL(signalTemplateActivated()), q, SLOT(slot_details_template_activated()));
    X->setProperty("widget_type", "cluster_details");
    add_tab(X, QString("Details"));
    return X;
}

void MVMainWindowPrivate::open_timeseries()
{
    MVTimeSeriesView2* X = new MVTimeSeriesView2(m_view_agent);
    X->setProperty("widget_type", "mvtimeseries");
    add_tab(X, QString("Timeseries"));
}

void MVMainWindowPrivate::open_clips()
{
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.count() == 0) {
        QMessageBox::information(q, "Unable to open clips", "You must select at least one cluster.");
        return;
    }

    MVClipsWidget* X = new MVClipsWidget(m_view_agent);
    X->setProperty("widget_type", "clips");
    X->setLabelsToUse(ks);
    /// TODO, pass this in a method
    //X->setProperty("ks", int_list_to_string_list(ks));
    /// TODO (LOW) more descriptive tab title in case of more than one
    QString tab_title = "Clips";
    if (ks.count() == 1) {
        int kk = ks[0];
        tab_title = QString("Clips %1").arg(kk);
    }

    add_tab(X, tab_title);
}

void MVMainWindowPrivate::open_pca_features()
{
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.count() == 0) {
        QMessageBox::information(q, "Unable to open clusters", "You must select at least one cluster.");
        return;
    }
    MVClusterWidget* X = new MVClusterWidget(m_view_agent);
    X->setFeatureMode("pca");
    X->setProperty("widget_type", "clusters");
    /// TODO pass this in a method
    //X->setProperty("ks", int_list_to_string_list(ks));
    add_tab(X, QString("PCA features"));
}

void MVMainWindowPrivate::open_channel_features()
{
    QSettings settings("SCDA", "MountainView");
    QString str = settings.value("open_channel_features_channels", "1,2,3").toString();
    str = QInputDialog::getText(0, "Open Channel Features", "Channels:", QLineEdit::Normal, str);
    if (str.isEmpty())
        return;
    QStringList strlist = str.split(",", QString::SkipEmptyParts);
    QList<int> channels;
    foreach (QString a, strlist) {
        bool ok;
        channels << a.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(0, "Open channel features", "Invalid channels string");
        }
    }
    settings.setValue("open_channel_features_channels", strlist.join(","));

    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.count() == 0) {
        QMessageBox::information(q, "Unable to open clusters", "You must select at least one cluster.");
        return;
    }
    MVClusterWidget* X = new MVClusterWidget(m_view_agent);
    X->setFeatureMode("channels");
    X->setProperty("widget_type", "clusters");
    /// TODO pass this in a method
    //X->setProperty("ks", int_list_to_string_list(ks));
    X->setChannels(channels);
    add_tab(X, QString("Ch. features"));
}

void MVMainWindowPrivate::open_spike_spray()
{
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.count() == 0) {
        QMessageBox::information(q, "Unable to open spike spray", "You must select at least one cluster.");
        return;
    }
    MVSpikeSprayView* X = new MVSpikeSprayView(m_view_agent);
    X->setProperty("widget_type", "spike_spray");
    X->setLabelsToUse(ks);
    add_tab(X, QString("Spike Spray"));
}

void MVMainWindowPrivate::open_firing_events()
{
    QList<int> ks = m_view_agent->selectedClusters();
    if (ks.count() == 0) {
        QMessageBox::information(q, "Unable to open firing events", "You must select at least one cluster.");
        return;
    }
    //MVFiringEventView* X = new MVFiringEventView;
    MVFiringEventView2* X = new MVFiringEventView2(m_view_agent);
    X->setLabelsToUse(ks.toSet());
    X->setNumTimepoints(m_view_agent->currentTimeseries().N2());
    X->setProperty("widget_type", "firing_events");
    /// TODO pass this in a method
    //X->setProperty("ks", int_list_to_string_list(ks));
    add_tab(X, QString("Firing Events"));
}

/*
void MVMainWindowPrivate::find_nearby_events()
{
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.count() < 2) {
        QMessageBox::information(q, "Problem finding nearby events", "You must select at least two clusters.");
        return;
    }

    MVClipsView* X = new MVClipsView(m_view_agent);
    X->setProperty("widget_type", "find_nearby_events");
    X->setProperty("ks", int_list_to_string_list(ks));
    QString tab_title = "Nearby Clips";
    if (ks.count() == 2) {
        int kk1 = ks[0];
        int kk2 = ks[1];
        tab_title = QString("Clips %1(%2) : Clips %3(%4)")
                        .arg(m_original_cluster_numbers.value(kk1))
                        .arg(m_original_cluster_offsets.value(kk1))
                        .arg(m_original_cluster_numbers.value(kk2))
                        .arg(m_original_cluster_offsets.value(kk2));
    }
    add_tab(X, tab_title);
    update_widget(X);
    X->setXRange(vec2(0, m_timeseries.N1() - 1));
}
*/

void MVMainWindowPrivate::annotate_selected()
{
    if (m_view_agent->selectedClusters().isEmpty())
        return;
    QList<int> ks = m_view_agent->selectedClusters();
    QString common_assessment;
    for (int i = 0; i < ks.count(); i++) {
        QString aa = get_cluster_attribute(ks[i], "assessment").toString();
        if (i == 0)
            common_assessment = aa;
        else {
            if (common_assessment != aa)
                common_assessment = "";
        }
    }
    bool ok;
    QString new_assessment = QInputDialog::getText(0, "Set cluster assessment", "Cluster assessment:", QLineEdit::Normal, common_assessment, &ok);
    if (ok) {
        for (int i = 0; i < ks.count(); i++) {
            set_cluster_attribute(ks[i], "assessment", new_assessment);
        }
    }
}

void MVMainWindowPrivate::merge_selected()
{
    ClusterMerge CM = m_view_agent->clusterMerge();
    CM.merge(m_view_agent->selectedClusters());
    m_view_agent->setClusterMerge(CM);
}

void MVMainWindowPrivate::unmerge_selected()
{
    ClusterMerge CM = m_view_agent->clusterMerge();
    CM.unmerge(m_view_agent->selectedClusters());
    m_view_agent->setClusterMerge(CM);
}

/*
void MVMainWindowPrivate::update_widget(QWidget* W)
{
    QString widget_type = W->property("widget_type").toString();

    if (widget_type == "clips") {
        MVClipsWidget* WW = (MVClipsWidget*)W;
        int clip_size = m_control_panel->viewOptions().clip_size;
        QList<int> ks = string_list_to_int_list(WW->property("ks").toStringList());
        DiskReadMda TT(current_timeseries_path());
        WW->setTimeseries(TT);
        WW->setClipSize(clip_size);
        WW->setFirings(m_view_agent->firings());
        WW->setLabelsToUse(ks);
    }
    else if (widget_type == "clusters") {
        MVClusterWidget* WW = (MVClusterWidget*)W;
        int clip_size = m_control_panel->viewOptions().clip_size;
        QList<int> ks = string_list_to_int_list(WW->property("ks").toStringList());
        DiskReadMda TT(current_timeseries_path());
        WW->setTimeseries(TT);
        WW->setClipSize(clip_size);
        //WW->setFirings(m_firings);
        WW->setFirings(m_view_agent->firings()); //now that we are doing the event filter, we should show everyone
        WW->setLabelsToUse(ks);
        FilterInfo FF;
        FF.use_filter = m_control_panel->eventFilter().use_event_filter;
        FF.min_detectability_score = m_control_panel->eventFilter().min_detectability_score;
        FF.max_outlier_score = m_control_panel->eventFilter().max_outlier_score;
        WW->setEventFilter(FF);
    }
    else if (widget_type == "spike_spray") {
        MVSpikeSprayView* WW = (MVSpikeSprayView*)W;
        int clip_size = m_control_panel->viewOptions().clip_size;
        QList<int> ks = string_list_to_int_list(WW->property("ks").toStringList());
        DiskReadMda TT(current_timeseries_path());
        WW->setTimeseries(TT);
        WW->setClipSize(clip_size);
        WW->setFirings(m_view_agent->firings());
        WW->setLabelsToUse(ks);
    }
    else if (widget_type == "mvtimeseries") {
        MVTimeSeriesView2* WW = (MVTimeSeriesView2*)W;
        WW->setTimeseries(DiskReadMda(current_timeseries_path()));
        set_times_labels_for_mvtimeseriesview(WW);
    }
}
*/

TabberTabWidget* MVMainWindowPrivate::tab_widget_of(QWidget* W)
{
    for (int i = 0; i < m_tabs1->count(); i++) {
        if (m_tabs1->widget(i) == W)
            return m_tabs1;
    }
    for (int i = 0; i < m_tabs2->count(); i++) {
        if (m_tabs2->widget(i) == W)
            return m_tabs2;
    }
    return m_tabs1;
}

class DownloadComputer : public ComputationThread {
public:
    //inputs
    QString source_path;
    QString dest_path;
    bool use_float64;

    void compute();
};
void DownloadComputer::compute()
{
    TaskProgress task("Downlading");
    task.setDescription(QString("Downloading %1 to %2").arg(source_path).arg(dest_path));
    DiskReadMda X(source_path);
    Mda Y;
    task.setProgress(0.2);
    task.log(QString("Reading/Downloading %1x%2x%3").arg(X.N1()).arg(X.N2()).arg(X.N3()));
    if (!X.readChunk(Y, 0, 0, 0, X.N1(), X.N2(), X.N3())) {
        if (thread_interrupt_requested()) {
            task.error("Halted download: " + source_path);
        }
        else {
            task.error("Failed to readChunk from: " + source_path);
        }
        return;
    }
    task.setProgress(0.8);
    if (use_float64) {
        task.log("Writing 64-bit to " + dest_path);
        Y.write64(dest_path);
    }
    else {
        task.log("Writing 32-bit to " + dest_path);
        Y.write32(dest_path);
    }
}

void MVMainWindowPrivate::export_mountainview_document()
{
    QString default_dir = "";
    QString fname = QFileDialog::getSaveFileName(q, "Export mountainview document", default_dir, "*.mv");
    if (QFileInfo(fname).suffix() != "mv")
        fname = fname + ".mv";
    q->writeMVFile(fname);
}

void MVMainWindowPrivate::export_original_firings()
{
    /// TODO reimplement export_original_firings
    /*
    QString default_dir = "";
    QString fname = QFileDialog::getSaveFileName(q, "Export original firings", default_dir, "*.mda");
    if (QFileInfo(fname).suffix() != "mda")
        fname = fname + ".mda";
    if (!fname.isEmpty()) {
        export_file(m_firings_original.path(), fname, true);
    }
    */
}

void MVMainWindowPrivate::export_filtered_firings()
{
    /// TODO reimplement export_filtered_firings
    /*
    QString default_dir = "";
    QString fname = QFileDialog::getSaveFileName(q, "Export filtered firings", default_dir, "*.mda");
    if (QFileInfo(fname).suffix() != "mda")
        fname = fname + ".mda";
    if (!fname.isEmpty()) {
        export_file(m_firings.path(), fname, true);
    }
    */
}

void MVMainWindowPrivate::export_file(QString source_path, QString dest_path, bool use_float64)
{
    DownloadComputer* C = new DownloadComputer;
    C->source_path = source_path;
    C->dest_path = dest_path;
    C->use_float64 = use_float64;
    C->setDeleteOnComplete(true);
    C->startComputation();
}

QVariant MVMainWindowPrivate::get_cluster_attribute(int k, QString attr)
{
    return m_view_agent->clusterAttributes().value(k).value(attr).toVariant();
}

void MVMainWindowPrivate::set_cluster_attribute(int k, QString attr, QVariant val)
{
    QMap<int, QJsonObject> CA = m_view_agent->clusterAttributes();
    CA[k][attr] = QJsonValue::fromVariant(val);
    m_view_agent->setClusterAttributes(CA);
}

void MVMainWindowPrivate::set_button_enabled(QString name, bool val)
{
    QAbstractButton* B = m_control_panel->findButton(name);
    if (B)
        B->setEnabled(val);
}
