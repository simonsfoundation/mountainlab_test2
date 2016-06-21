#include "mvmainwindow.h"
#include "diskreadmda.h"
#include "mvcrosscorrelogramswidget2.h"
#include "mvclusterdetailwidget.h"
#include "mvclipswidget.h"
#include "mvclusterwidget.h"
#include "mvspikesprayview.h"
#include "mvfiringeventview2.h"
#include "tabber.h"
#include "computationthread.h"
#include "mvclipswidget.h"
#include "taskprogressview.h"
#include "mvcontrolpanel.h"
#include "taskprogress.h"
#include "mvviewagent.h"
#include "mvstatusbar.h"
#include "mvtimeseriesview2.h"
#include "mlutils.h"
#include "mvfile.h"
#include "mvabstractviewfactory.h"
#include "mvamphistview.h"

/// TODO, get rid of computationthread

#include <QHBoxLayout>
#include <QMessageBox>
#include <QSignalMapper>
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
#include <QToolButton>
#include <QAction>
#include "textfile.h"

/// TODO (LOW) put styles in central place?
#define MV_STATUS_BAR_HEIGHT 30

MVMainWindow* MVMainWindow::window_instance = 0;

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
    QList<MVAbstractViewFactory*> m_viewFactories;
    QSignalMapper* m_viewMapper;


    void registerAllViews();

    MVAbstractViewFactory* viewFactoryById(const QString &id) const;
    MVAbstractView* openView(MVAbstractViewFactory *factory);

    void update_sizes(); //update sizes of all the widgets when the main window is resized
    void add_tab(MVAbstractView* W, QString label);

    MVCrossCorrelogramsWidget2* open_auto_correlograms();
    MVCrossCorrelogramsWidget2* open_cross_correlograms(int k);
    MVCrossCorrelogramsWidget2* open_matrix_of_cross_correlograms();

    /// TODO: (MEDIUM) implement find_nearby_events
    //void find_nearby_events();

    void annotate_selected();
    void merge_selected();
    void unmerge_selected();

    TabberTabWidget* tab_widget_of(QWidget* W);

    void export_mountainview_document();
    void export_original_firings();
    void export_filtered_firings();
    void export_file(QString source_path, QString dest_path, bool use_float64);

    void recalculate_views(QString str);

    //not sure about these
    QVariant get_cluster_attribute(int k, QString attr);
    void set_cluster_attribute(int k, QString attr, QVariant val);
    void set_button_enabled(QString name, bool val);
};

MVMainWindow::MVMainWindow(MVViewAgent* view_agent, QWidget* parent)
    : QWidget(parent)
{
    window_instance = this;
    d = new MVMainWindowPrivate;
    d->q = this;
    d->m_viewMapper = new QSignalMapper(this);
    connect(d->m_viewMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(slot_open_view(QObject*)));

    d->m_view_agent = view_agent;

    registerViewFactory(new MVClusterDetailsFactory(this));
    registerViewFactory(new MVAutoCorrelogramsFactory(this));
    registerViewFactory(new MVMatrixOfCrossCorrelogramsFactory(this));
    registerViewFactory(new MVTimeSeriesDataFactory(this));
    registerViewFactory(new MVClipsFactory(this));
    registerViewFactory(new MVPCAFeaturesFactory(this));
    registerViewFactory(new MVChannelFeaturesFactory(this));
    registerViewFactory(new MVSpikeSprayFactory(this));
    registerViewFactory(new MVFiringEventsFactory(this));
    registerViewFactory(new MVAmplitudeHistogramsFactory(this));

    d->m_control_panel = new MVControlPanel(view_agent);
    //probably get rid of the following line
    connect(d->m_control_panel, SIGNAL(userAction(QString)), this, SLOT(slot_control_panel_user_action(QString)));

    d->registerAllViews();


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
    window_instance = 0;
}

void MVMainWindow::setDefaultInitialization()
{
    openView("open-cluster-details");
    d->m_tabber->switchCurrentContainer();
    openView("open-auto-correlograms");
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

    d->m_view_agent->clear();
    QStringList timeseries_names = d->m_mv_file.timeseriesNames();

    foreach (QString name, timeseries_names) {
        DiskReadMda TS(d->m_mv_file.timeseriesPathResolved(name));
        d->m_view_agent->addTimeseries(name, DiskReadMda(TS));
    }

    d->m_view_agent->setOption("clip_size", d->m_mv_file.viewOptions()["clip_size"].toInt());
    d->m_view_agent->setOption("cc_max_dt_msec", d->m_mv_file.viewOptions()["cc_max_dt_msec"].toDouble());

    d->m_view_agent->setEventFilter(MVEventFilter::fromJsonObject(d->m_mv_file.eventFilter()));
    //d->m_control_panel->setEventFilter();
    if (!d->m_mv_file.currentTimeseriesName().isEmpty()) {
        d->m_view_agent->setCurrentTimeseriesName(d->m_mv_file.currentTimeseriesName());
    }
    else {
        d->m_view_agent->setCurrentTimeseriesName(timeseries_names.value(0));
    }

    d->m_view_agent->setCurrentTimeRange(MVRange(0, d->m_view_agent->currentTimeseries().N2() - 1));

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

MVFile MVMainWindow::getMVFile()
{
    /// TODO (LOW) improve relationship between mvfile and view agent
    QJsonObject view_options;
    view_options["clip_size"] = d->m_view_agent->option("clip_size").toInt();
    view_options["cc_max_dt_msec"] = d->m_view_agent->option("cc_max_dt_msec").toDouble();
    //d->m_mv_file.setEventFilter(d->m_control_panel->eventFilter().toJsonObject());
    d->m_mv_file.setEventFilter(d->m_view_agent->eventFilter().toJsonObject());
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

    return d->m_mv_file;
}

void MVMainWindow::registerViewFactory(MVAbstractViewFactory *f)
{
    // sort by group name and order
    QList<MVAbstractViewFactory*>::iterator iter
            = qUpperBound(d->m_viewFactories.begin(), d->m_viewFactories.end(),
                          f, [](MVAbstractViewFactory *f1, MVAbstractViewFactory *f2) {
            if (f1->group() < f2->group())
                return true;
            if (f1->group() == f2->group() && f1->order() < f2->order())
                return true;
            return false;
    });
    d->m_viewFactories.insert(iter, f);
}

void MVMainWindow::unregisterViewFactory(MVAbstractViewFactory *f)
{
    d->m_viewFactories.removeOne(f);
}

const QList<MVAbstractViewFactory *> &MVMainWindow::viewFactories() const
{
    return d->m_viewFactories;
}

MVMainWindow *MVMainWindow::instance()
{
    return window_instance;
}

TabberTabWidget *MVMainWindow::tabWidget(QWidget *w) const
{
    return d->tab_widget_of(w);
}

Tabber *MVMainWindow::tabber() const
{
    return d->m_tabber;
}

void MVMainWindow::openView(const QString &id)
{
    MVAbstractViewFactory *f = d->viewFactoryById(id);
    if (f)
        d->openView(f);
}

MVViewAgent *MVMainWindow::viewAgent() const
{
    return d->m_view_agent;
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
#if 0
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
    else if (str == "open-amplitude-histograms") {
        d->open_amplitude_histograms();
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
    else if (str == "recalculate-all") {
        d->recalculate_views("all");
    }
    else if (str == "recalculate-all-suggested") {
        d->recalculate_views("all-suggested");
    }
    else if (str == "recalculate-all-visible") {
        d->recalculate_views("all-visible");
    }
    else if (str == "recalculate-all-suggested-and-visible") {
        d->recalculate_views("all-suggested-and-visible");
    }
#endif
}

/// TODO (LOW) figure out how to implement such functionality outside main window class
void MVMainWindow::slot_auto_correlogram_activated()
{
    TabberTabWidget* TW = d->tab_widget_of((QWidget*)sender());
    d->m_tabber->setCurrentContainer(TW);
    d->m_tabber->switchCurrentContainer();
    d->open_cross_correlograms(d->m_view_agent->currentCluster());
}


void MVMainWindow::slot_amplitude_histogram_activated()
{
    //not sure what to do here
}

void MVMainWindow::slot_update_buttons()
{
    bool something_selected = (!d->m_view_agent->selectedClusters().isEmpty());

    d->set_button_enabled("open-clusters", something_selected);
    d->set_button_enabled("find-nearby-events", d->m_view_agent->selectedClusters().count() >= 2);

    d->set_button_enabled("annotate_selected", something_selected);
    d->set_button_enabled("merge_selected", d->m_view_agent->selectedClusters().count() >= 2);
    d->set_button_enabled("unmerge_selected", something_selected);
    d->set_button_enabled("export_mountainview_document", true);
    d->set_button_enabled("export_original_firings", true);
    d->set_button_enabled("export_filtered_firings", true);
}

/// TODO (MEDIUM) this functionality should be moved to tabber
void MVMainWindow::slot_action_move_to_other_tab_widget()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (!a)
        return;
    MVAbstractView* W = qobject_cast<MVAbstractView*>(a->parentWidget());
    if (!W)
        return;
    d->m_tabber->moveWidgetToOtherContainer(W);
}

void MVMainWindow::slot_pop_out_widget()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (!a)
        return;
    MVAbstractView* W = qobject_cast<MVAbstractView*>(a->parentWidget());
    if (!W)
        return;
    d->m_tabber->popOutWidget(W);
}

void MVMainWindow::slot_open_view(QObject *o)
{
    MVAbstractViewFactory *factory = qobject_cast<MVAbstractViewFactory*>(o);
    if (!factory) return;
    d->openView(factory);
}

void MVMainWindowPrivate::registerAllViews()
{
    // unregister all existing views
    QLayoutItem *item;
    while((item = m_control_panel->viewLayout()->takeAt(0))) {
        delete item;
    }

    // register all views again
    foreach(MVAbstractViewFactory *f, m_viewFactories) {
        QToolButton *button = new QToolButton;
        QFont font = button->font();
        font.setPixelSize(14);
        button->setFont(font);
        button->setText(f->name());
        button->setProperty("action_name", f->id());
        button->setEnabled(f->isEnabled());
        m_control_panel->viewLayout()->addWidget(button);
        m_viewMapper->setMapping(button, f);
        QObject::connect(button, SIGNAL(clicked()), m_viewMapper, SLOT(map()));
        QObject::connect(f, SIGNAL(enabledChanged(bool)), button, SLOT(setEnabled(bool)));
    }
}

MVAbstractViewFactory *MVMainWindowPrivate::viewFactoryById(const QString &id) const
{
    foreach(MVAbstractViewFactory *f, m_viewFactories) {
        if (f->id() == id) return f;
    }
    return Q_NULLPTR;
}

MVAbstractView *MVMainWindowPrivate::openView(MVAbstractViewFactory *factory)
{
    MVAbstractView *view = factory->createView(m_view_agent);
    if (!view) return Q_NULLPTR;
//    set_tool_button_menu(view);
    add_tab(view, factory->title());
    return view;
}

void MVMainWindowPrivate::update_sizes()
{
    float W0 = q->width();
    float H0 = q->height() - MV_STATUS_BAR_HEIGHT;

    int W1 = W0 / 3;
    if (W1 < 150)
        W1 = 150;
    if (W1 > 600)
        W1 = 600;
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
            tv_height = 300;
        }
        else {
            tv_height = 200;
        }
        int cp_height = H0 - tv_height;
        QList<int> sizes;
        sizes << cp_height << tv_height;
        m_left_splitter->setSizes(sizes);
    }
}

void MVMainWindowPrivate::add_tab(MVAbstractView* W, QString label)
{
    W->setFocusPolicy(Qt::StrongFocus);
    m_tabber->addWidget(m_tabber->currentContainerName(), label, W);
}

MVCrossCorrelogramsWidget2* MVMainWindowPrivate::open_cross_correlograms(int k)
{
    MVCrossCorrelogramsWidget2* X = new MVCrossCorrelogramsWidget2(m_view_agent);
    CrossCorrelogramOptions opts;
    opts.mode = Cross_Correlograms;
    opts.ks << k;
    X->setOptions(opts);
    QString str = QString("CC for %1").arg(k);
    add_tab(X, str);
    return X;
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
    MVFile ff = q->getMVFile();
    if (!ff.write(fname)) {
        TaskProgress task("export mountainview document");
        task.error("Error writing .mv file: " + fname);
    }
}

void MVMainWindowPrivate::export_original_firings()
{
    QString default_dir = "";
    QString fname = QFileDialog::getSaveFileName(q, "Export original firings", default_dir, "*.mda");
    if (QFileInfo(fname).suffix() != "mda")
        fname = fname + ".mda";

    DiskReadMda firings = m_view_agent->firings();
    if (!fname.isEmpty()) {
        export_file(firings.makePath(), fname, true);
    }
}

void MVMainWindowPrivate::export_filtered_firings()
{
    QString default_dir = "";
    QString fname = QFileDialog::getSaveFileName(q, "Export original firings", default_dir, "*.mda");
    if (QFileInfo(fname).suffix() != "mda")
        fname = fname + ".mda";

    DiskReadMda firings = m_view_agent->firings();
    firings = compute_filtered_firings_remotely(m_view_agent->mlProxyUrl(), firings, m_view_agent->eventFilter());
    if (!fname.isEmpty()) {
        export_file(firings.makePath(), fname, true);
    }
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

void MVMainWindowPrivate::recalculate_views(QString str)
{
    QList<MVAbstractView*> widgets = m_tabber->allWidgets();
    foreach (MVAbstractView* VV, widgets) {
        if (VV) {
            bool do_it = false;
            if (str == "all")
                do_it = true;
            else if (str == "all-suggested")
                do_it = VV->recalculateSuggested();
            else if (str == "all-visible")
                do_it = VV->isVisible();
            else if (str == "all-suggested-and-visible")
                do_it = ((VV->isVisible()) && (VV->recalculateSuggested()));
            if (do_it) {
                VV->recalculate();
            }
        }
    }
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
