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

    QList<QColor> m_channel_colors;
    QMap<QString, QColor> m_colors;

    //shell_split_and_event_filter_calculator m_calculator;

    void create_cross_correlograms_data();
    //void create_templates_data();

    void update_sizes();
    //void update_templates();
    void update_cluster_details();
    void update_clips();
    void update_cluster_views();
    void update_firing_event_views();
    //void start_shell_split_and_event_filter();
    void add_tab(QWidget* W, QString label);
    void set_tool_button_menu(QWidget* X);

    MVCrossCorrelogramsWidget2* open_auto_correlograms();
    MVCrossCorrelogramsWidget2* open_cross_correlograms(int k);
    MVCrossCorrelogramsWidget2* open_matrix_of_cross_correlograms();
    //void open_templates();
    MVClusterDetailWidget* open_cluster_details();
    void open_timeseries();
    void open_clips();
    void open_pca_features();
    void open_channel_features();
    void open_spike_spray();
    void open_firing_events();
    //void find_nearby_events();

    void annotate_selected();
    void merge_selected();
    void unmerge_selected();

    void update_cross_correlograms();
    void update_timeseries_views();
    void move_to_timepoint(double tp);
    void update_widget(QWidget* W);

    void set_times_labels_for_mvtimeseriesview(MVTimeSeriesView2* WW);
    void set_times_labels_for_timeseries_widget(SSTimeSeriesWidget* WW);

    QList<QWidget*> get_all_widgets();
    TabberTabWidget* tab_widget_of(QWidget* W);

    void remove_widgets_of_type(QString widget_type);

    Mda compute_centroid(Mda& clips);
    Mda compute_geometric_median(Mda& clips, int num_iterations);
    void compute_geometric_median(int M, int N, double* output, double* input, int num_iterations);

    void set_progress(QString title, QString text, float frac);

    void export_mountainview_document();
    void export_original_firings();
    void export_filtered_firings();
    void export_file(QString source_path, QString dest_path, bool use_float64);

    QString current_timeseries_path();

    QVariant get_cluster_attribute(int k, QString attr);
    void set_cluster_attribute(int k, QString attr, QVariant val);
    void set_button_enabled(QString name, bool val);

    //void start_cross_correlograms_computer();
};

MVMainWindow::MVMainWindow(MVViewAgent* view_agent, QWidget* parent)
    : QWidget(parent)
{
    d = new MVMainWindowPrivate;
    d->q = this;

    d->m_view_agent = view_agent;

    d->m_progress_dialog = 0;

    //connect(&d->m_calculator, SIGNAL(computationFinished()), this, SLOT(slot_calculator_finished()));

    d->m_control_panel = new MVControlPanel(view_agent);
    connect(d->m_control_panel, SIGNAL(userAction(QString)), this, SLOT(slot_control_panel_user_action(QString)));

    QSplitter* splitter1 = new QSplitter;
    splitter1->setOrientation(Qt::Horizontal);
    d->m_splitter1 = splitter1;

    QSplitter* splitter2 = new QSplitter;
    splitter2->setOrientation(Qt::Vertical);
    d->m_splitter2 = splitter2;

    QScrollArea* CP = new QScrollArea;
    CP->setWidget(d->m_control_panel);
    CP->setWidgetResizable(true);

    d->m_task_progress_view = new TaskProgressView;

    QSplitter* left_splitter = new QSplitter(Qt::Vertical);
    left_splitter->addWidget(CP);
    left_splitter->addWidget(d->m_task_progress_view);
    d->m_left_splitter = left_splitter;

    splitter1->addWidget(left_splitter);
    splitter1->addWidget(splitter2);

    d->m_tabber = new Tabber;
    d->m_tabs2 = d->m_tabber->createTabWidget("south");
    d->m_tabs1 = d->m_tabber->createTabWidget("north");

    splitter2->addWidget(d->m_tabs1);
    splitter2->addWidget(d->m_tabs2);

    MVStatusBar* status_bar = new MVStatusBar();
    status_bar->setFixedHeight(MV_STATUS_BAR_HEIGHT);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addWidget(splitter1);
    vlayout->addWidget(status_bar);
    this->setLayout(vlayout);

    d->m_colors["background"] = QColor(240, 240, 240);
    d->m_colors["frame1"] = QColor(245, 245, 245);
    d->m_colors["info_text"] = QColor(80, 80, 80);
    d->m_colors["view_background"] = QColor(245, 245, 245);
    d->m_colors["view_background_highlighted"] = QColor(210, 230, 250);
    d->m_colors["view_background_selected"] = QColor(220, 240, 250);
    d->m_colors["view_background_hovered"] = QColor(240, 245, 240);

    d->m_colors["view_frame_selected"] = QColor(50, 20, 20);
    d->m_colors["divider_line"] = QColor(255, 100, 150);

    slot_update_buttons();

    connect(view_agent, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(clusterAttributesChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(clusterMergeChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(currentEventChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(currentTimepointChanged()), this, SLOT(slot_update_buttons()));
    connect(view_agent, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_buttons()));

    //connect(&d->m_cross_correlogram_computer,SIGNAL(computationFinished()),this,SLOT(slot_cross_correlogram_computer_finished()));
}

MVMainWindow::~MVMainWindow()
{
    delete d;
}

void MVMainWindow::setCurrentTimeseriesName(const QString& name)
{
    {
        MVViewOptions opts = d->m_control_panel->viewOptions();
        opts.timeseries = name;
        d->m_control_panel->setViewOptions(opts);
    }

    d->update_timeseries_views();
    d->update_cluster_details();
    d->update_clips();
    d->update_cluster_views();
    d->update_firing_event_views();
}

void MVMainWindow::setDefaultInitialization()
{
    //d->open_templates();
    d->open_cluster_details();
    d->m_tabber->switchCurrentContainer();
    d->open_auto_correlograms();
}

void MVMainWindow::setEpochs(const QList<Epoch>& epochs)
{
    d->m_epochs = epochs;
}

/*
QImage MVMainWindow::generateImage(const QMap<QString, QVariant>& params)
{
    QString type0 = params.value("type").toString();
    if (type0 == "templates") {
        MVClusterDetailWidget* X = d->open_cluster_details();
        return X->renderImage();
    }
    else if (type0 == "auto_correlograms") {
        MVCrossCorrelogramsWidget2* X = d->open_auto_correlograms();
        return X->renderImage();
    }
    else if (type0 == "cross_correlograms") {
        int k = params.value("k", 0).toInt();
        MVCrossCorrelogramsWidget2* X = d->open_cross_correlograms(k);
        return X->renderImage();
    }
    else {
        qWarning() << "Unknown type in generateImage: " << type0;
        return QImage();
    }
}
*/

void MVMainWindow::setClusterMerge(ClusterMerge CM)
{
    d->m_view_agent->setClusterMerge(CM);
}

void MVMainWindow::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
}

void MVMainWindow::setClusterColors(const QList<QColor>& colors)
{
    d->m_view_agent->setClusterColors(colors);
}

/*
void MVMainWindow::setMscmdServerUrl(const QString& url)
{
    d->m_mscmdserver_url = url;
}
*/

void MVMainWindow::setMVFile(MVFile ff)
{
    d->m_mv_file = ff;

    QStringList timeseries_names = d->m_mv_file.timeseriesNames();

    d->m_control_panel->setTimeseriesChoices(timeseries_names);
    //if (!timeseries_names.isEmpty()) {
    //    d->m_view_agent->setCurrentTimeseriesName(timeseries_names[0]);
    //}

    d->m_control_panel->setViewOptions(MVViewOptions::fromJsonObject(d->m_mv_file.viewOptions()));
    d->m_control_panel->setEventFilter(MVEventFilter::fromJsonObject(d->m_mv_file.eventFilter()));
    if (!d->m_mv_file.currentTimeseriesName().isEmpty()) {
        MVViewOptions opts = d->m_control_panel->viewOptions();
        opts.timeseries = d->m_mv_file.currentTimeseriesName();
        d->m_control_panel->setViewOptions(opts);
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
    d->m_view_agent->setTimeseries(DiskReadMda(d->current_timeseries_path()));
}

void MVMainWindow::writeMVFile(const QString& mv_fname)
{
    TaskProgress task("saving .mv file: " + mv_fname);

    d->m_mv_file.setViewOptions(d->m_control_panel->viewOptions().toJsonObject());
    d->m_mv_file.setEventFilter(d->m_control_panel->eventFilter().toJsonObject());
    d->m_mv_file.setCurrentTimeseriesName(d->m_control_panel->viewOptions().timeseries);

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

/*
void MVMainWindow::slot_control_panel_button_clicked(QString str)
{
    if (str == "update_cross_correlograms") {
        //d->start_cross_correlograms_computer();
        d->update_cross_correlograms();
    }
    else if (str == "update_templates") {
        //	d->update_templates();
        d->update_cluster_details();
        d->update_clips();
    }
    else if (str == "update_cluster_details") {
        d->update_cluster_details();
    }
    else if ((str == "update_shell_split") || (str == "use_shell_split")) {
        d->do_shell_split_and_event_filter();
        d->remove_widgets_of_type("cross_correlograms");
        d->remove_widgets_of_type("matrix_of_cross_correlograms");
        d->remove_widgets_of_type("clips");
        d->remove_widgets_of_type("find_nearby_events");
        d->remove_widgets_of_type("clusters");
        d->remove_widgets_of_type("firing_events");
        d->update_cluster_details();
        d->update_cross_correlograms();
    }
    else if ((str == "update_event_filter") || (str == "use_event_filter")) {
        d->do_shell_split_and_event_filter();
        //d->do_event_filter();
        //d->start_cross_correlograms_computer();
        d->update_all_widgets();
    }
    else if (str == "open_auto_correlograms") {
        d->open_auto_correlograms();
    }
    else if (str == "open_matrix_of_cross_correlograms") {
        d->open_matrix_of_cross_correlograms();
    }
    //else if (str=="open_templates") {
    //    d->open_templates();
    //}
    else if (str == "open_cluster_details") {
        d->open_cluster_details();
    }
    else if (str == "open_timeseries") {
        d->open_timeseries();
    }
    else if (str == "open_clips") {
        d->open_clips();
    }
    else if (str == "open_clusters") {
        d->open_clusters();
    }
    else if (str == "open_firing_events") {
        d->open_firing_events();
    }
    else if (str == "find_nearby_events") {
        d->find_nearby_events();
    }
    else if (str == "template_method") {
        d->update_cluster_details();
    }
}
*/

void MVMainWindow::slot_control_panel_user_action(QString str)
{
    //if ((str == "apply_shell_splitting") || (str == "apply_filter")) {
    //d->start_shell_split_and_event_filter();
    //}
    if (str == "update_viewing_options") {
        d->m_view_agent->setTimeseries(DiskReadMda(d->current_timeseries_path()));
        d->update_cross_correlograms();
    }
    else if (str == "open-cluster-details") {
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
    //else if (str == "find-nearby-events") {
    //d->find_nearby_events();
    //}
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
    else {
        TaskProgress task(str);
        task.error("user action not yet implemented.");
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
void MVMainWindowPrivate::start_shell_split_and_event_filter()
{
    m_calculator.stopComputation();
    m_calculator.m_evt_filter = m_control_panel->eventFilter();
    m_calculator.m_firings_original = m_firings_original;
    m_calculator.m_mlproxy_url = m_mv_file.mlproxyUrl();
    m_calculator.startComputation();
}
*/

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
        m_splitter1->setSizes(sizes);
    }
    {
        QList<int> sizes;
        sizes << H1 << H2;
        m_splitter2->setSizes(sizes);
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

/*
void MVMainWindowPrivate::update_all_widgets()
{
    QList<QWidget*> list = get_all_widgets();
    foreach (QWidget* W, list) {
        update_widget(W);
    }
}
*/

void MVMainWindowPrivate::update_cluster_details()
{
    QList<QWidget*> list = get_all_widgets();
    foreach (QWidget* W, list) {
        if (W->property("widget_type") == "cluster_details") {
            update_widget(W);
        }
    }
}

void MVMainWindowPrivate::update_clips()
{
    QList<QWidget*> list = get_all_widgets();
    foreach (QWidget* W, list) {
        if (W->property("widget_type") == "clips") {
            update_widget(W);
        }
        if (W->property("widget_type") == "find_nearby_events") {
            update_widget(W);
        }
    }
}

void MVMainWindowPrivate::update_cluster_views()
{
    QList<QWidget*> list = get_all_widgets();
    foreach (QWidget* W, list) {
        if (W->property("widget_type") == "clusters") {
            update_widget(W);
        }
        if (W->property("widget_type") == "spike_spray") {
            update_widget(W);
        }
    }
}

void MVMainWindowPrivate::update_firing_event_views()
{
    QList<QWidget*> list = get_all_widgets();
    foreach (QWidget* W, list) {
        if (W->property("widget_type") == "firing_events") {
            update_widget(W);
        }
    }
}

double get_max(QList<double>& list)
{
    double ret = list.value(0);
    for (int i = 0; i < list.count(); i++) {
        if (list[i] > ret)
            ret = list[i];
    }
    return ret;
}

double get_min(QList<double>& list)
{
    double ret = list.value(0);
    for (int i = 0; i < list.count(); i++) {
        if (list[i] < ret)
            ret = list[i];
    }
    return ret;
}

void MVMainWindowPrivate::add_tab(QWidget* W, QString label)
{
    W->setFocusPolicy(Qt::StrongFocus);
    //current_tab_widget()->addTab(W,label);
    //current_tab_widget()->setCurrentIndex(current_tab_widget()->count()-1);
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
    X->setProperty("widget_type", "auto_correlograms");
    add_tab(X, "Auto-Correlograms");
    QObject::connect(X, SIGNAL(histogramActivated()), q, SLOT(slot_auto_correlogram_activated()));
    update_widget(X);
    return X;
}

MVCrossCorrelogramsWidget2* MVMainWindowPrivate::open_cross_correlograms(int k)
{
    MVCrossCorrelogramsWidget2* X = new MVCrossCorrelogramsWidget2(m_view_agent);
    X->setProperty("widget_type", "cross_correlograms");
    X->setProperty("kk", k);
    QString str = QString("CC for %1").arg(k);
    add_tab(X, str);
    update_widget(X);
    return X;
}

QStringList int_list_to_string_list(const QList<int>& list)
{
    QStringList list2;
    for (int i = 0; i < list.count(); i++)
        list2 << QString("%1").arg(list[i]);
    return list2;
}

QList<int> string_list_to_int_list(const QList<QString>& list)
{
    QList<int> list2;
    for (int i = 0; i < list.count(); i++)
        list2 << list[i].toInt();
    return list2;
}

MVCrossCorrelogramsWidget2* MVMainWindowPrivate::open_matrix_of_cross_correlograms()
{
    MVCrossCorrelogramsWidget2* X = new MVCrossCorrelogramsWidget2(m_view_agent);
    X->setProperty("widget_type", "matrix_of_cross_correlograms");
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    X->setProperty("ks", int_list_to_string_list(ks));
    add_tab(X, QString("CC Matrix"));
    update_widget(X);
    return X;
}

//void MVMainWindowPrivate::open_templates()
//{
//	SSTimeSeriesView *X=new SSTimeSeriesView;
//	X->initialize();
//	X->setProperty("widget_type","templates");
//	add_tab(X,QString("Templates"));
//    QObject::connect(X,SIGNAL(currentXChanged()),q,SLOT(slot_templates_clicked()));
//	update_widget(X);
//}

MVClusterDetailWidget* MVMainWindowPrivate::open_cluster_details()
{
    /// TODO move sample rate into mvviewagent
    MVClusterDetailWidget* X = new MVClusterDetailWidget(m_view_agent);
    set_tool_button_menu(X);
    X->setMLProxyUrl(m_mv_file.mlproxyUrl());
    X->setChannelColors(m_channel_colors);
    //DiskReadMda TT(current_timeseries_path());
    //X->setTimeseries(TT);
    //X->setFirings(DiskReadMda(m_firings)); //done in update_widget
    X->setSampleRate(m_mv_file.sampleRate());
    QObject::connect(X, SIGNAL(signalTemplateActivated()), q, SLOT(slot_details_template_activated()));
    X->setProperty("widget_type", "cluster_details");
    add_tab(X, QString("Details"));
    update_widget(X);
    return X;
}

void MVMainWindowPrivate::open_timeseries()
{
    MVTimeSeriesView2* X = new MVTimeSeriesView2(m_view_agent);
    X->setSampleRate(m_mv_file.sampleRate());
    X->setChannelColors(m_channel_colors);
    X->setProperty("widget_type", "mvtimeseries");
    X->setMLProxyUrl(m_mv_file.mlproxyUrl());
    add_tab(X, QString("Timeseries"));
    update_widget(X);
    /*
    SSTimeSeriesWidget* X = new SSTimeSeriesWidget;
    SSTimeSeriesView* V = new SSTimeSeriesView;
    V->initialize();
    V->setSampleRate(m_samplerate);
    X->addView(V);
    X->setProperty("widget_type", "timeseries");
    add_tab(X, QString("Timeseries"));
    update_widget(X);
    */
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
    //X->setMscmdServerUrl(m_mscmdserver_url);
    X->setMLProxyUrl(m_mv_file.mlproxyUrl());
    X->setProperty("widget_type", "clips");
    X->setProperty("ks", int_list_to_string_list(ks));
    /// TODO (LOW) more descriptive tab title in case of more than one
    QString tab_title = "Clips";
    if (ks.count() == 1) {
        int kk = ks[0];
        tab_title = QString("Clips %1").arg(kk);
    }

    add_tab(X, tab_title);
    update_widget(X);

    /*
    MVClipsView* X = MVClipsView::newInstance();
    X->setProperty("widget_type", "clips");
    X->setProperty("ks", int_list_to_string_list(ks));
    QString tab_title = "Clips";
    if (ks.count() == 1) {
        int kk = ks[0];
        tab_title = QString("Clips %1(%2)").arg(m_original_cluster_numbers.value(kk)).arg(m_original_cluster_offsets.value(kk));
    }
    add_tab(X, tab_title);
    update_widget(X);
    X->setXRange(vec2(0, 5000));
    */
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
    X->setMLProxyUrl(m_mv_file.mlproxyUrl());
    X->setProperty("widget_type", "clusters");
    X->setProperty("ks", int_list_to_string_list(ks));
    add_tab(X, QString("PCA features"));
    update_widget(X);
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
    X->setMLProxyUrl(m_mv_file.mlproxyUrl());
    X->setProperty("widget_type", "clusters");
    X->setProperty("ks", int_list_to_string_list(ks));
    X->setChannels(channels);
    add_tab(X, QString("Ch. features"));
    update_widget(X);
}

void MVMainWindowPrivate::open_spike_spray()
{
    QList<int> ks = m_view_agent->selectedClusters();
    qSort(ks);
    if (ks.count() == 0) {
        QMessageBox::information(q, "Unable to open spike spray", "You must select at least one cluster.");
        return;
    }
    MVSpikeSprayView* X = new MVSpikeSprayView;
    X->setMLProxyUrl(m_mv_file.mlproxyUrl());
    X->setProperty("widget_type", "spike_spray");
    X->setProperty("ks", int_list_to_string_list(ks));
    add_tab(X, QString("Spike Spray"));
    update_widget(X);
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
    X->setNumTimepoints(m_view_agent->timeseries().N2());
    X->setSampleRate(m_mv_file.sampleRate());
    X->setProperty("widget_type", "firing_events");
    X->setProperty("ks", int_list_to_string_list(ks));
    add_tab(X, QString("Firing Events"));
    update_widget(X);
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

void MVMainWindowPrivate::update_cross_correlograms()
{
    QList<QWidget*> widgets = get_all_widgets();
    foreach (QWidget* W, widgets) {
        QString widget_type = W->property("widget_type").toString();
        if ((widget_type == "auto_correlograms") || (widget_type == "cross_correlograms")) {
            MVCrossCorrelogramsWidget2* WW = qobject_cast<MVCrossCorrelogramsWidget2*>(W);
            if (WW) {
            }
        }
    }
}

void MVMainWindowPrivate::update_timeseries_views()
{
    QList<QWidget*> widgets = get_all_widgets();
    foreach (QWidget* W, widgets) {
        QString widget_type = W->property("widget_type").toString();
        if ((widget_type == "timeseries") || (widget_type == "mvtimeseries")) {
            update_widget(W);
        }
    }
}

void MVMainWindowPrivate::move_to_timepoint(double tp)
{
    QList<QWidget*> widgets = get_all_widgets();
    foreach (QWidget* W, widgets) {
        QString widget_type = W->property("widget_type").toString();
        if (widget_type == "timeseries") {
            SSTimeSeriesWidget* V = (SSTimeSeriesWidget*)W;
            V->view(0)->setCurrentTimepoint(tp);
        }
    }
}

void subtract_features_mean(Mda& F)
{
    if (F.N1() == 0)
        return;
    if (F.N2() == 0)
        return;
    double mean[F.N1()];
    for (int i = 0; i < F.N1(); i++)
        mean[i] = 0;
    for (int i = 0; i < F.N2(); i++) {
        for (int j = 0; j < F.N1(); j++) {
            mean[j] += F.value(j, i);
        }
    }
    for (int i = 0; i < F.N1(); i++)
        mean[i] /= F.N2();
    for (int i = 0; i < F.N2(); i++) {
        for (int j = 0; j < F.N1(); j++) {
            F.setValue(F.value(j, i) - mean[j], j, i);
        }
    }
}

void normalize_features(Mda& F, bool individual_channels)
{
    if (individual_channels) {
        for (int m = 0; m < F.N1(); m++) {
            double sumsqr = 0;
            double sum = 0;
            int NN = F.N2();
            for (int i = 0; i < NN; i++) {
                sumsqr += F.value(m, i) * F.value(m, i);
                sum += F.value(m, i);
            }
            double norm = 1;
            if (NN >= 2)
                norm = sqrt((sumsqr - sum * sum / NN) / (NN - 1));
            for (int i = 0; i < NN; i++)
                F.setValue(F.value(m, i) / norm, m, i);
        }
    }
    else {
        double sumsqr = 0;
        double sum = 0;
        int NN = F.totalSize();
        for (int i = 0; i < NN; i++) {
            sumsqr += F.get(i) * F.get(i);
            sum += F.get(i);
        }
        double norm = 1;
        if (NN >= 2)
            norm = sqrt((sumsqr - sum * sum / NN) / (NN - 1));
        for (int i = 0; i < NN; i++)
            F.set(F.get(i) / norm, i);
    }
}

void MVMainWindowPrivate::update_widget(QWidget* W)
{
    QString widget_type = W->property("widget_type").toString();
    if (widget_type == "auto_correlograms") {
        TaskProgress task("update auto correlograms");
        MVCrossCorrelogramsWidget2* WW = (MVCrossCorrelogramsWidget2*)W;
        set_tool_button_menu(WW);
        WW->setSampleRate(m_mv_file.sampleRate());
        WW->setColors(m_colors);
        task.log(QString("m_samplerate = %1").arg(m_mv_file.sampleRate()));
        //WW->setCrossCorrelogramsData(DiskReadMda(m_cross_correlograms_data));
        /*
        QStringList text_labels;
        QList<int> labels1, labels2;
        for (int i = 1; i < m_original_cluster_numbers.count(); i++) {
            labels1 << i;
            labels2 << i;
            if ((i == 1) || (m_original_cluster_numbers.value(i) != m_original_cluster_numbers.value(i - 1))) {
                text_labels << QString("Auto %1").arg(m_original_cluster_numbers.value(i));
            }
            else
                text_labels << "";
        }
        CrossCorrelogramOptions opts;
        opts.mode="all_auto_correlograms";
        WW->setOptions(opts);
        qDebug() << "AAAAAAAAAAAAAAAAAAAAAA" << labels1 << labels2 << text_labels;
        //WW->setTextLabels(labels);
        WW->setLabelPairs(labels1, labels2, text_labels);
        //WW->updateWidget();
        */
    }
    else if (widget_type == "cross_correlograms") {
        /*
        MVCrossCorrelogramsWidget2* WW = (MVCrossCorrelogramsWidget2*)W;
        set_tool_button_menu(WW);
        int k = W->property("kk").toInt();
        WW->setColors(m_colors);
        WW->setSampleRate(m_mv_file.sampleRate());
        WW->setMaxDtTimepoints(cc_max_dt_timepoints());
        //WW->setCrossCorrelogramsData(DiskReadMda(m_cross_correlograms_data));
        //WW->setBaseLabel(k);
        QStringList text_labels;
        QList<int> labels1, labels2;
        for (int i = 1; i < m_original_cluster_numbers.count(); i++) {
            labels1 << k;
            labels2 << i;
            if ((i == 1) || (m_original_cluster_numbers.value(i) != m_original_cluster_numbers.value(i - 1))) {
                text_labels << QString("Cross %1/%3").arg(m_original_cluster_numbers.value(k)).arg(m_original_cluster_numbers.value(i));
            }
            else
                text_labels << "";
        }
        //WW->setTextLabels(labels);
        WW->setLabelPairs(labels1, labels2, text_labels);
        //WW->updateWidget();
        */
    }
    else if (widget_type == "matrix_of_cross_correlograms") {
        /*
        MVCrossCorrelogramsWidget2* WW = (MVCrossCorrelogramsWidget2*)W;
        set_tool_button_menu(WW);
        QList<int> ks = string_list_to_int_list(W->property("ks").toStringList());
        WW->setColors(m_colors);
        WW->setSampleRate(m_mv_file.sampleRate());
        WW->setMaxDtTimepoints(cc_max_dt_timepoints());
        //WW->setCrossCorrelogramsData(DiskReadMda(m_cross_correlograms_data));
        //WW->setLabelNumbers(ks);
        QStringList text_labels;
        QList<int> labels1, labels2;
        //text_labels << "";
        for (int a1 = 0; a1 < ks.count(); a1++) {
            QString str1 = QString("%1").arg(m_original_cluster_numbers.value(ks[a1]));
            for (int a2 = 0; a2 < ks.count(); a2++) {
                QString str2 = QString("%1").arg(m_original_cluster_numbers.value(ks[a2]));
                text_labels << QString("%1/%2").arg(str1).arg(str2);
                labels1 << ks.value(a1);
                labels2 << ks.value(a2);
            }
        }
        //WW->setTextLabels(labels);
        WW->setLabelPairs(labels1, labels2, text_labels);
        //WW->updateWidget();
        */
    }
    /*else if (widget_type=="templates") {
        printf("Setting templates data...\n");
        SSTimeSeriesView *WW=(SSTimeSeriesView *)W;
        Mda TD=m_templates_data;
        DiskArrayModel_New *MM=new DiskArrayModel_New;
        MM->setFromMda(TD);
        int KK=TD.N3();
        QList<long> times,labels;
        int last_k=-1;
        for (int kk=1; kk<=KK; kk++) {
            int k=m_original_cluster_numbers.value(kk);
            if (k!=last_k) {
                times << (long)(TD.N2()*(kk-1+0.5));
                labels << k;
            }
            last_k=k;
        }
        WW->setData(MM,true);
        WW->setTimesLabels(times,labels);
        WW->setMarkerLinesVisible(false);
        printf(".\n");
    }*/
    else if (widget_type == "cluster_details") {
        MVClusterDetailWidget* WW = (MVClusterDetailWidget*)W;
        //int clip_size = m_control_panel->getParameterValue("clip_size").toInt();
        TaskProgress task("Update cluster details");
        WW->setColors(m_colors);
        DiskReadMda TT(current_timeseries_path());
        WW->zoomAllTheWayOut();
    }
    else if (widget_type == "clips") {
        MVClipsWidget* WW = (MVClipsWidget*)W;
        int clip_size = m_control_panel->viewOptions().clip_size;
        QList<int> ks = string_list_to_int_list(WW->property("ks").toStringList());
        DiskReadMda TT(current_timeseries_path());
        WW->setTimeseries(TT);
        WW->setClipSize(clip_size);
        WW->setFirings(m_view_agent->firings());
        WW->setLabelsToUse(ks);
    }
    /*
    else if (widget_type == "find_nearby_events") {
        printf("Extracting clips...\n");
        MVClipsView* WW = (MVClipsView*)W;
        QList<int> ks = string_list_to_int_list(WW->property("ks").toStringList());

        QList<int> labels;
        QList<double> times;

        for (int n = 0; n < m_firings.N2(); n++) {
            times << m_firings.value(1, n);
            labels << (int)m_firings.value(2, n);
        }

        int clip_size = m_control_panel->viewOptions().clip_size;

        QList<long> inds;
        if (widget_type == "clips") {
            for (int ik = 0; ik < ks.count(); ik++) {
                int kk = ks[ik];
                for (int n = 0; n < labels.count(); n++) {
                    if (labels[n] == kk) {
                        inds << n;
                    }
                }
            }
        }
        else if (widget_type == "find_nearby_events") {
            long max_time = (long)(compute_max(times) + 1);
            Mda occupied(ks.count(), max_time + 1);
            for (int ik = 0; ik < ks.count(); ik++) {
                int kk = ks[ik];
                for (int n = 0; n < labels.count(); n++) {
                    if (labels[n] == kk) {
                        occupied.setValue(1, ik, (long)times[n]);
                    }
                }
            }
            int radius = clip_size / 2 - 1;
            int kk1 = ks.value(0);
            for (long n = 0; n < labels.count(); n++) {
                if (labels[n] == kk1) {
                    bool okay = true;
                    for (int ik = 1; (ik < ks.count()) && (okay); ik++) {
                        bool found = false;
                        for (int dt = -radius; dt <= radius; dt++) {
                            if (occupied.value(ik, (long)times[n] + dt)) {
                                found = true;
                            }
                        }
                        if (!found)
                            okay = false;
                    }
                    if (okay)
                        inds << n;
                }
            }
        }

        QList<double> times_kk;
        QList<int> labels_kk;
        for (long i = 0; i < inds.count(); i++) {
            long n = inds[i];
            times_kk << times.value(n);
            labels_kk << labels.value(n);
        }

        DiskReadMda TT(current_timeseries_path());
        Mda clips = extract_clips(TT, times_kk, clip_size);
        printf("Setting data...\n");
        //DiskArrayModel_New *DAM=new DiskArrayModel_New;
        //DAM->setFromMda(clips);
        //WW->setData(DAM,true);
        WW->setClips(clips);
        WW->setTimes(times_kk);
        WW->setLabels(labels_kk);
        printf(".\n");
    }
    */
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
    /*
    else if (widget_type == "firing_events") {
        //MVFiringEventView* WW = (MVFiringEventView*)W;
        MVFiringEventView2* WW = (MVFiringEventView2*)W;
        QList<int> ks = string_list_to_int_list(WW->property("ks").toStringList());
        QSet<int> ks_set = ks.toSet();

        WW->setSampleRate(m_mv_file.sampleRate());
        DiskReadMda TT(current_timeseries_path());
        WW->setNumTimepoints(TT.N2());
        WW->setTimeRange(MVRange(0, TT.N2() - 1));
        //WW->setEpochs(m_epochs);
        QVector<double> times, amplitudes;
        QVector<int> labels;
        for (int i = 0; i < m_firings.N2(); i++) {
            int label = (int)m_firings.value(2, i);
            if (ks_set.contains(label)) {
                times << m_firings.value(1, i);
                amplitudes << m_firings.value(3, i);
                labels << label;
            }
        }
        WW->setTimesLabels(times, labels);
        WW->setAmplitudes(amplitudes);
        WW->autoSetAmplitudeRange();
    }
    */
    else if (widget_type == "timeseries") {
        SSTimeSeriesWidget* WW = (SSTimeSeriesWidget*)W;
        DiskArrayModel_New* X = new DiskArrayModel_New;
        X->setPath(current_timeseries_path());
        ((SSTimeSeriesView*)(WW->view()))->setData(X, true);
        set_times_labels_for_timeseries_widget(WW);
    }
    else if (widget_type == "mvtimeseries") {
        MVTimeSeriesView2* WW = (MVTimeSeriesView2*)W;
        WW->setTimeseries(DiskReadMda(current_timeseries_path()));
        set_times_labels_for_mvtimeseriesview(WW);
    }
}

void MVMainWindowPrivate::set_times_labels_for_mvtimeseriesview(MVTimeSeriesView2* WW)
{
    /*
    QVector<double> times;
    QVector<int> labels;
    for (int n = 0; n < m_firings_original.N2(); n++) {
        long label0 = (long)m_firings_original.value(2, n);
        if ((m_view_agent->selectedClusters().isEmpty()) || (m_view_agent->selectedClusters().contains(label0))) {
            times << (long)m_firings_original.value(1, n);
            labels << label0;
        }
    }
    WW->setTimesLabels(times, labels);
    */
}

void MVMainWindowPrivate::set_times_labels_for_timeseries_widget(SSTimeSeriesWidget* WW)
{
    /*
    QList<long> times, labels;
    for (int n = 0; n < m_firings_original.N2(); n++) {
        long label0 = (long)m_firings_original.value(2, n);
        if ((m_view_agent->selectedClusters().isEmpty()) || (m_view_agent->selectedClusters().contains(label0))) {
            times << (long)m_firings_original.value(1, n);
            labels << label0;
        }
    }
    SSTimeSeriesView* V = (SSTimeSeriesView*)WW->view();
    V->setTimesLabels(times, labels);
    */

    /*
    QList<QWidget*> widgets = get_all_widgets();
    foreach (QWidget* W, widgets) {
        QString widget_type = W->property("widget_type").toString();
        if (widget_type == "timeseries") {
            SSTimeSeriesWidget* WW = (SSTimeSeriesWidget*)W;
            SSTimeSeriesView* V = (SSTimeSeriesView*)WW->view();
            V->setTimesLabels(times, labels);
        }
    }
    */
}

QList<QWidget*> MVMainWindowPrivate::get_all_widgets()
{
    return m_tabber->allWidgets();
    /*
    QList<QWidget *> ret;
    for (int i=0; i<m_tabs1->count(); i++) {
        ret << m_tabs1->widget(i);
    }
    for (int i=0; i<m_tabs2->count(); i++) {
        ret << m_tabs2->widget(i);
    }
    return ret;
    */
}

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

void MVMainWindowPrivate::remove_widgets_of_type(QString widget_type)
{
    QList<QWidget*> list = get_all_widgets();
    foreach (QWidget* W, list) {
        if (W->property("widget_type") == widget_type) {
            delete W;
        }
    }
}

Mda MVMainWindowPrivate::compute_centroid(Mda& clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int NC = clips.N3();
    Mda ret;
    ret.allocate(M, T);
    double* retptr = ret.dataPtr();
    double* clipsptr = clips.dataPtr();
    for (int i = 0; i < NC; i++) {
        int iii = i * M * T;
        int jjj = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                retptr[jjj] += clipsptr[iii];
                iii++;
                jjj++;
            }
        }
    }
    if (NC) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                double val = ret.value(m, t) / NC;
                ret.setValue(val, m, t);
            }
        }
    }
    return ret;
}

Mda MVMainWindowPrivate::compute_geometric_median(Mda& clips, int num_iterations)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    double* clipsptr = clips.dataPtr();
    Mda ret;
    ret.allocate(M, T);
    if (L == 0)
        return ret;

    int num_features = 6;
    Mda features;
    features.allocate(num_features, L);
    double* featuresptr = features.dataPtr();
#ifdef USE_LAPACK
    get_pca_features(M * T, L, num_features, featuresptr, clipsptr);
#else
    get_pca_features_2(M * T, L, num_features, featuresptr, clipsptr);
#endif

    Mda geomedian;
    geomedian.allocate(num_features, 1);
    double* geomedianptr = geomedian.dataPtr();
    compute_geometric_median(num_features, L, geomedianptr, featuresptr, num_iterations);

    QList<double> dists;
    {
        int kkk = 0;
        for (int j = 0; j < L; j++) {
            double sumsqr = 0;
            for (int a = 0; a < num_features; a++) {
                double diff = featuresptr[kkk] - geomedianptr[a];
                sumsqr += diff * diff;
                kkk++;
            }
            dists << sqrt(sumsqr);
        }
    }

    QList<double> dists_sorted = dists;
    qSort(dists_sorted);
    double cutoff = dists_sorted[(int)(L * 0.3)];
    QList<int> inds_to_use;
    for (int j = 0; j < L; j++) {
        if (dists[j] <= cutoff)
            inds_to_use << j;
    }

    Mda clips2;
    clips2.allocate(M, T, inds_to_use.count());
    for (int j = 0; j < inds_to_use.count(); j++) {
        int jj = inds_to_use[j];
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                clips2.setValue(clips.value(m, t, jj), m, t, j);
            }
        }
    }

    ret = compute_centroid(clips2);
    return ret;
}

void MVMainWindowPrivate::compute_geometric_median(int M, int N, double* output, double* input, int num_iterations)
{
    double* weights = (double*)malloc(sizeof(double) * N);
    double* dists = (double*)malloc(sizeof(double) * N);
    for (int j = 0; j < N; j++)
        weights[j] = 1;
    for (int it = 1; it <= num_iterations; it++) {
        float sumweights = 0;
        for (int j = 0; j < N; j++)
            sumweights += weights[j];
        if (sumweights)
            for (int j = 0; j < N; j++)
                weights[j] /= sumweights;
        for (int i = 0; i < M; i++)
            output[i] = 0;
        {
            //compute output
            int kkk = 0;
            for (int j = 0; j < N; j++) {
                int i = 0;
                for (int m = 0; m < M; m++) {
                    output[i] += weights[j] * input[kkk];
                    i++;
                    kkk++;
                }
            }
        }
        {
            //compute dists
            int kkk = 0;
            for (int j = 0; j < N; j++) {
                int i = 0;
                double sumsqr = 0;
                for (int m = 0; m < M; m++) {
                    double diff = output[i] - input[kkk];
                    i++;
                    kkk++;
                    sumsqr += diff * diff;
                }
                dists[j] = sqrt(sumsqr);
            }
        }
        {
            //compute weights
            for (int j = 0; j < N; j++) {
                if (dists[j])
                    weights[j] = 1 / dists[j];
                else
                    weights[j] = 0;
            }
        }
    }
    free(dists);
    free(weights);
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

QString MVMainWindowPrivate::current_timeseries_path()
{
    return m_mv_file.timeseriesPathResolved(m_control_panel->viewOptions().timeseries);
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

/*
void shell_split_and_event_filter_calculator::compute()
{
    TaskProgress task("shell_split_and_event_filter()");

    MountainProcessRunner MT;
    QString processor_name = "mv_firings_filter";

    MT.setProcessorName(processor_name);

    QMap<QString, QVariant> params;

    params["use_shell_split"] = m_evt_filter.use_shell_split;
    params["shell_width"] = m_evt_filter.shell_increment;
    params["min_per_shell"] = m_evt_filter.min_per_shell;
    params["use_event_filter"] = m_evt_filter.use_event_filter;
    //if (m_evt_filter.min_detectability_score) {
    params["min_detectability_score"] = m_evt_filter.min_detectability_score;
    //}
    //if (evt_filter.max_outlier_score) {
    params["max_outlier_score"] = m_evt_filter.max_outlier_score;
    //}
    params["min_amplitude"] = 0;

    params["firings"] = m_firings_original.makePath();

    QStringList debug_keys = params.keys();
    foreach (QString key, debug_keys) {
        task.log(QString("%1 = %2").arg(key).arg(params[key].toString()));
    }

    MT.setInputParameters(params);
    //MT.setMscmdServerUrl(m_mscmdserver_url);
    MT.setMLProxyUrl(m_mlproxy_url);

    QString firings_out = MT.makeOutputFilePath("firings_out");
    QString original_cluster_numbers_out = MT.makeOutputFilePath("original_cluster_numbers");
    MT.runProcess();

    m_firings.setPath(firings_out);
    task.log("m_firings path = " + firings_out);
    task.log(QString("m_firings.N1()=%1 m_firings.N2()=%2").arg(m_firings.N1()).arg(m_firings.N2()));
    m_original_cluster_numbers.clear();
    m_original_cluster_offsets.clear();

    DiskReadMda AA(original_cluster_numbers_out);
    int offset = 0;
    for (int i = 0; i < AA.totalSize(); i++) {
        if (AA.value(i) != AA.value(i - 1)) {
            offset = 0;
        }
        offset++;
        m_original_cluster_numbers << AA.value(i);
        m_original_cluster_offsets << offset;
    }
}
*/
