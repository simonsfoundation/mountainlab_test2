#include "mvmainwindow.h"
#include "diskreadmda.h"
#include "mvcrosscorrelogramswidget3.h"
#include "mvclusterdetailwidget.h"
#include "mvclipswidget.h"
#include "mvclusterwidget.h"
#include "mvspikesprayview.h"
#include "mvfiringeventview2.h"
#include "tabber.h"
#include "taskprogressview.h"
#include "mvcontrolpanel2.h"
#include "taskprogress.h"
#include "mvcontext.h"
#include "mvstatusbar.h"
#include "mvtimeseriesview2.h"
#include "mlcommon.h"
#include "mvabstractviewfactory.h"
#include "mvamphistview2.h"
#include "mvamphistview3.h"
#include "mvdiscrimhistview.h"
#include "guidev1.h"

#include <QApplication>
#include <QProcess>
#include <mvtemplatesview2.h>

#include "mvabstractviewfactory.h"
#include "mvabstractcontextmenuhandler.h"
#include "mvclustercontextmenuhandler.h"
#include "mvclusterpaircontextmenuhandler.h"

/// TODO, get rid of computationthread
/// TODO: (HIGH) create test dataset to be distributed
/// TODO: (MEDIUM) move export options to main menu

#include <QHBoxLayout>
#include <QMessageBox>
#include <QSignalMapper>
#include <QSplitter>
#include <QTime>
#include <QTimer>
#include <math.h>
#include <QProgressDialog>
#include "mlcommon.h"
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
#include <QShortcut>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <mvdiscrimhistview_guide.h>
#include <guidev2.h>
#include <firetrackview.h>
#include <cachemanager.h>

#include "clusterannotationguide.h"

/// TODO (LOW) put styles in central place?
#define MV_STATUS_BAR_HEIGHT 30

class MVMainWindowPrivate {
public:
    MVMainWindow* q;
    MVContext* m_context; //gets passed to all the views and the control panel

    //these widgets go on the left
    QSplitter* m_left_splitter;
    MVControlPanel2* m_control_panel;
    TaskProgressView* m_task_progress_view;

    QSplitter* m_hsplitter, *m_vsplitter;
    TabberTabWidget* m_tabs1, *m_tabs2;
    Tabber* m_tabber; //manages the views in the two tab widgets
    QList<MVAbstractViewFactory*> m_viewFactories;
    QSignalMapper* m_viewMapper;
    QList<MVAbstractContextMenuHandler*> m_menuHandlers;

    MVAbstractViewFactory* viewFactoryById(const QString& id) const;
    MVAbstractView* openView(MVAbstractViewFactory* factory);

    ClusterAnnotationGuide* m_cluster_annotation_guide;
    GuideV1* m_guide_v1;
    GuideV2* m_guide_v2;

    void update_sizes(); //update sizes of all the widgets when the main window is resized
    void add_tab(MVAbstractView* W, QString label);

    //MVCrossCorrelogramsWidget3* open_auto_correlograms();
    //MVCrossCorrelogramsWidget3* open_cross_correlograms(int k);
    //MVCrossCorrelogramsWidget3* open_matrix_of_cross_correlograms();

    TabberTabWidget* tab_widget_of(QWidget* W);
};

MVMainWindow::MVMainWindow(MVContext* context, QWidget* parent)
    : QWidget(parent)
{
    d = new MVMainWindowPrivate;
    d->q = this;
    d->m_viewMapper = new QSignalMapper(this);
    connect(d->m_viewMapper, SIGNAL(mapped(QObject*)),
        this, SLOT(slot_open_view(QObject*)));

    d->m_context = context;

    registerViewFactory(new MVClusterDetailsFactory(context, this));
    registerViewFactory(new MVTemplatesView2Factory(context, this));
    registerViewFactory(new MVAutoCorrelogramsFactory(context, this));
    registerViewFactory(new MVSelectedAutoCorrelogramsFactory(context, this));
    registerViewFactory(new MVCrossCorrelogramsFactory(context, this));
    registerViewFactory(new MVMatrixOfCrossCorrelogramsFactory(context, this));
    registerViewFactory(new MVSelectedCrossCorrelogramsFactory(context, this));
    registerViewFactory(new MVTimeSeriesDataFactory(context, this));
    registerViewFactory(new MVClipsFactory(context, this));
    registerViewFactory(new MVPCAFeaturesFactory(context, this));
    registerViewFactory(new MVChannelFeaturesFactory(context, this));
    registerViewFactory(new MVSpikeSprayFactory(context, this));
    registerViewFactory(new MVFiringEventsFactory(context, this));
    registerViewFactory(new MVAmplitudeHistogramsFactory(context, this));
    registerViewFactory(new MVAmplitudeHistograms3Factory(context, this));
    registerViewFactory(new MVDiscrimHistFactory(context, this));
    registerViewFactory(new MVDiscrimHistGuideFactory(context, this));
    registerViewFactory(new MVFireTrackFactory(context, this));

    registerContextMenuHandler(new MVClusterContextMenuHandler(context, this));
    registerContextMenuHandler(new MVClusterPairContextMenuHandler(context, this));

    QToolBar* main_toolbar = new QToolBar;
    d->m_cluster_annotation_guide = new ClusterAnnotationGuide(d->m_context, this);
    d->m_guide_v1 = new GuideV1(d->m_context, this);
    d->m_guide_v2 = new GuideV2(d->m_context, this);
    {

        {
            QToolButton* B = new QToolButton();
            B->setText("File");
            QMenu* menu = new QMenu;
            B->setMenu(menu);
            B->setPopupMode(QToolButton::InstantPopup);
            main_toolbar->addWidget(B);
            {
                QAction* A = new QAction("Export .mv file", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalExportMVFile()));
            }
            {
                QAction* A = new QAction("Export firings file", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalExportFiringsFile()));
            }
            {
                QAction* A = new QAction("Export cluster annotation file", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalExportClusterAnnotationFile()));
            }
            {
                QAction* A = new QAction("Export static views (.smv)", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalExportStaticViews()));
            }
            {
                QAction* A = new QAction("Share views on web", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalShareViewsOnWeb()));
            }
        }
        {
            QToolButton* B = new QToolButton();
            //B->setIcon(QIcon(":/images/gear.png"));
            B->setText("Guides");
            QMenu* menu = new QMenu;
            B->setMenu(menu);
            B->setPopupMode(QToolButton::InstantPopup);
            main_toolbar->addWidget(B);
            {
                QAction* A = new QAction("Guide version 2", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_guide_v2()));
            }
        }

        {
            QToolButton* B = new QToolButton();
            B->setText("Tools");
            QMenu* menu = new QMenu;
            B->setMenu(menu);
            B->setPopupMode(QToolButton::InstantPopup);
            main_toolbar->addWidget(B);
            {
                QAction* A = new QAction("Extract selected clusters in new view", this);
                menu->addAction(A);
                QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(extractSelectedClusters()));
            }
        }

        /*
        {
            QAction* A = new QAction("Guide version 1", this);
            menu->addAction(A);
            QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_guide_v1()));
        }
        {
            QAction* A = new QAction("Cluster annotation guide", this);
            menu->addAction(A);
            QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_cluster_annotation_guide()));
        }
        */
    }

    d->m_control_panel = new MVControlPanel2(context, this);

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
    QObject::connect(d->m_tabber, SIGNAL(widgetsChanged()), this, SIGNAL(viewsChanged()));

    vsplitter->addWidget(d->m_tabs1);
    vsplitter->addWidget(d->m_tabs2);

    MVStatusBar* status_bar = new MVStatusBar();
    status_bar->setFixedHeight(MV_STATUS_BAR_HEIGHT);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addWidget(main_toolbar);
    vlayout->addWidget(hsplitter);
    vlayout->addWidget(status_bar);
    this->setLayout(vlayout);

    QFont fnt = this->font();
    fnt.setPointSize(12);
    this->setFont(fnt);

    QShortcut* closeWindowShortcut = new QShortcut(QKeySequence("Ctrl+W"), this, SLOT(close()));
    Q_UNUSED(closeWindowShortcut)
}

MVMainWindow::~MVMainWindow()
{
    delete d->m_cluster_annotation_guide;
    delete d;
}

void MVMainWindow::setDefaultInitialization()
{
    openView("open-cluster-details");
    d->m_tabber->switchCurrentContainer();
    openView("open-auto-correlograms");
}

void MVMainWindow::registerViewFactory(MVAbstractViewFactory* f)
{
    // sort by group name and order
    QList<MVAbstractViewFactory*>::iterator iter
        = qUpperBound(d->m_viewFactories.begin(), d->m_viewFactories.end(),
            f, [](MVAbstractViewFactory* f1, MVAbstractViewFactory* f2) {
            if (f1->group() < f2->group())
                return true;
            if (f1->group() == f2->group() && f1->order() < f2->order())
                return true;
            return false;
            });
    d->m_viewFactories.insert(iter, f);
}

void MVMainWindow::unregisterViewFactory(MVAbstractViewFactory* f)
{
    d->m_viewFactories.removeOne(f);
}

const QList<MVAbstractViewFactory*>& MVMainWindow::viewFactories() const
{
    return d->m_viewFactories;
}

void MVMainWindow::registerContextMenuHandler(MVAbstractContextMenuHandler* h)
{
    d->m_menuHandlers.append(h);
}

void MVMainWindow::unregisterContextMenuHandler(MVAbstractContextMenuHandler* h)
{
    d->m_menuHandlers.removeOne(h);
}

const QList<MVAbstractContextMenuHandler*>& MVMainWindow::contextMenuHandlers() const
{
    return d->m_menuHandlers;
}

void MVMainWindow::addControl(MVAbstractControl* control, bool start_expanded)
{
    d->m_control_panel->addControl(control, start_expanded);
}

void MVMainWindow::setCurrentContainerName(const QString& name)
{
    d->m_tabber->setCurrentContainerName(name);
}

QList<MVAbstractView*> MVMainWindow::allViews()
{
    return d->m_tabber->allWidgets();
}

QJsonObject MVMainWindow::exportStaticViews() const
{
    QList<MVAbstractView*> views = d->m_tabber->allWidgets();
    QJsonObject ret;
    ret["export-static-views-version"] = "0.1";
    ret["mvcontext"] = d->m_context->toMVFileObject();
    QJsonArray SV;
    foreach (MVAbstractView* V, views) {
        QJsonObject obj = V->exportStaticView();
        if (!obj.isEmpty()) {
            QJsonObject tmp;
            tmp["container"] = V->property("container").toString();
            tmp["title"] = V->title();
            if (V->viewFactory()) {
                tmp["view-factory-title"] = V->viewFactory()->title();
            }
            tmp["data"] = obj;
            SV.append(tmp);
        }
    }
    ret["static-views"] = SV;
    return ret;
}

void MVMainWindow::openView(const QString& id)
{
    MVAbstractViewFactory* f = d->viewFactoryById(id);
    if (f)
        d->openView(f);
    else
        qWarning() << "Unknow view factory: " + id;
}

void MVMainWindow::recalculateViews(RecalculateViewsMode mode)
{
    QList<MVAbstractView*> widgets = d->m_tabber->allWidgets();
    bool do_it = false;
    foreach (MVAbstractView* VV, widgets) {
        if (!VV)
            continue;
        switch (mode) {
        case All:
            do_it = true;
            break;
        case Suggested:
            do_it = VV->recalculateSuggested();
            break;
        case AllVisible:
            do_it = VV->isVisible();
            break;
        case SuggestedVisible:
            do_it = ((VV->isVisible()) && (VV->recalculateSuggested()));
            break;
        default:
            do_it = false;
        }
        if (do_it) {
            VV->recalculate();
        }
    }
}

void MVMainWindow::extractSelectedClusters()
{
    QString tmp_fname = CacheManager::globalInstance()->makeLocalFile() + ".mv";
    QJsonObject obj = this->mvContext()->toMVFileObject();
    QString json = QJsonDocument(obj).toJson();
    TextFile::write(tmp_fname, json);
    QString exe = qApp->applicationFilePath();
    QStringList args;
    QList<int> clusters = mvContext()->selectedClusters();
    QStringList clusters_str;
    foreach (int cluster, clusters) {
        clusters_str << QString("%1").arg(cluster);
    }

    args << tmp_fname << "--clusters=" + clusters_str.join(",");
    qDebug() << "EXECUTING: " + exe + " " + args.join(" ");
    QProcess::startDetached(exe, args);
}

MVContext* MVMainWindow::mvContext() const
{
    return d->m_context;
}

void MVMainWindow::closeAllViews()
{
    qDeleteAll(d->m_tabber->allWidgets());
}

void MVMainWindow::resizeEvent(QResizeEvent* evt)
{
    Q_UNUSED(evt)
    d->update_sizes();
}

/*
void MVMainWindow::keyPressEvent(QKeyEvent* evt)
{
    /// TODO restore keypress M,U,T functionality

    QWidget::keyPressEvent(evt);
}
*/

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

void MVMainWindow::slot_cluster_annotation_guide()
{
    d->m_cluster_annotation_guide->show();
    d->m_cluster_annotation_guide->raise();
}

void MVMainWindow::slot_guide_v1()
{
    this->closeAllViews();
    d->m_guide_v1->show();
    d->m_guide_v1->raise();
}

void MVMainWindow::slot_guide_v2()
{
    this->closeAllViews();
    d->m_guide_v2->show();
    d->m_guide_v2->raise();
    //the following causes widget to break
    //d->m_guide_v2->setWindowFlags(d->m_guide_v2->windowFlags()|Qt::WindowStaysOnTopHint);
}

void MVMainWindow::slot_open_view(QObject* o)
{
    MVAbstractViewFactory* factory = qobject_cast<MVAbstractViewFactory*>(o);
    if (!factory)
        return;
    d->openView(factory);
}

void MVMainWindow::handleContextMenu(const QMimeData& dt, const QPoint& globalPos)
{
    QList<QAction*> actions;
    foreach (MVAbstractContextMenuHandler* handler, contextMenuHandlers()) {
        if (handler->canHandle(dt))
            actions += handler->actions(dt);
    }
    if (actions.isEmpty())
        return;
    QMenu menu;
    menu.addActions(actions);
    menu.exec(globalPos);

    // delete orphan actions
    foreach (QAction* a, menu.actions()) {
        if (!a->parent())
            a->deleteLater();
    }
}

MVAbstractViewFactory* MVMainWindowPrivate::viewFactoryById(const QString& id) const
{
    foreach (MVAbstractViewFactory* f, m_viewFactories) {
        if (f->id() == id)
            return f;
    }
    return Q_NULLPTR;
}

MVAbstractView* MVMainWindowPrivate::openView(MVAbstractViewFactory* factory)
{
    MVAbstractView* view = factory->createView();
    if (!view)
        return Q_NULLPTR;
    //    set_tool_button_menu(view);
    /// TODO: don't pass label as argument to add_tab (think about it)
    view->setTitle(factory->title());
    add_tab(view, factory->title());

    QObject::connect(view, SIGNAL(contextMenuRequested(QMimeData, QPoint)),
        q, SLOT(handleContextMenu(QMimeData, QPoint)));

    return view;
}

void MVMainWindowPrivate::update_sizes()
{
    float W0 = q->width();
    float H0 = q->height() - MV_STATUS_BAR_HEIGHT;

    int W1 = W0 / 3;
    if (W1 < 250)
        W1 = 250;
    if (W1 > 800)
        W1 = 800;
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

/*
MVCrossCorrelogramsWidget3* MVMainWindowPrivate::open_auto_correlograms()
{
    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(m_context);
    CrossCorrelogramOptions3 opts;
    opts.mode = All_Auto_Correlograms3;
    X->setOptions(opts);
    add_tab(X, "Auto-Correlograms");
    QObject::connect(X, SIGNAL(histogramActivated()), q, SLOT(slot_auto_correlogram_activated()));
    return X;
}

MVCrossCorrelogramsWidget3* MVMainWindowPrivate::open_cross_correlograms(int k)
{
    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(m_context);
    CrossCorrelogramOptions3 opts;
    opts.mode = Cross_Correlograms3;
    opts.ks << k;
    X->setOptions(opts);
    QString str = QString("CC for %1").arg(k);
    add_tab(X, str);
    return X;
}

MVCrossCorrelogramsWidget3* MVMainWindowPrivate::open_matrix_of_cross_correlograms()
{
    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(m_context);
    QList<int> ks = m_context->selectedClusters();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    CrossCorrelogramOptions3 opts;
    opts.mode = Matrix_Of_Cross_Correlograms3;
    opts.ks = ks;
    X->setOptions(opts);
    add_tab(X, QString("CC Matrix"));
    return X;
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
