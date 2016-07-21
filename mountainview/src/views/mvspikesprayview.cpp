/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/8/2016
*******************************************************/

#include "mvspikesprayview.h"
#include "extract_clips.h"
#include "mountainprocessrunner.h"
#include "mvmainwindow.h"
#include "mvspikespraypanel.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <taskprogress.h>
#include "mlcommon.h"
#include "actionfactory.h"

/// TODO: (MEDIUM) spike spray should respond to mouse wheel and show current position with marker
/// TODO: (MEDIUM) much more responsive rendering of spike spray
/// TODO: (HIGH) Implement the panels as layers, make a layer stack, and handle zoom/pan without using a scroll widget
/// TODO: (HIGH) GUI option to set max. number of spikespray spikes to render

class MVSpikeSprayComputer {
public:
    //input
    DiskReadMda timeseries;
    DiskReadMda firings;
    QString mlproxy_url;
    MVEventFilter filter;
    QSet<int> labels_to_use;
    int clip_size;

    //output
    Mda clips_to_render;
    QVector<int> labels_to_render;

    void compute();
};

class MVSpikeSprayViewPrivate {
public:
    MVSpikeSprayView* q;
    MVContext* m_context;
    QSet<int> m_labels_to_use;

    double m_amplitude_factor = 0; //zero triggers auto-calculation
    int m_panel_width = 0;

    Mda m_clips_to_render;
    QVector<int> m_labels_to_render;
    MVSpikeSprayComputer m_computer;

    QHBoxLayout* m_panel_layout;
    QList<MVSpikeSprayPanel*> m_panels;

    MVSpikeSprayPanel* add_panel();
    void set_amplitude_factor(double val);
    void set_panel_width(double w);
    double actual_panel_width();
};

MVSpikeSprayView::MVSpikeSprayView(MVContext* context)
    : MVAbstractView(context)
{
    d = new MVSpikeSprayViewPrivate;
    d->q = this;
    d->m_context = context;

    recalculateOnOptionChanged("clip_size");
    recalculateOnOptionChanged("timeseries_for_spikespray");
    recalculateOn(mvContext(), SIGNAL(clusterMergeChanged()));
    recalculateOn(mvContext(), SIGNAL(timeseriesNamesChanged()));
    recalculateOn(mvContext(), SIGNAL(filteredFiringsChanged()));

    QWidget* panel_widget = new QWidget;
    d->m_panel_layout = new QHBoxLayout;
    panel_widget->setLayout(d->m_panel_layout);

    QScrollArea* SA = new QScrollArea;
    SA->setWidget(panel_widget);
    SA->setWidgetResizable(true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(SA);
    this->setLayout(layout);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomIn, this, SLOT(slot_zoom_in()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOut, this, SLOT(slot_zoom_out()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomInVertical, this, SLOT(slot_vertical_zoom_in()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOutVertical, this, SLOT(slot_vertical_zoom_out()));

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

MVSpikeSprayView::~MVSpikeSprayView()
{
    this->stopCalculation();
    delete d;
}

void MVSpikeSprayView::setLabelsToUse(const QSet<int>& labels_to_use)
{
    d->m_labels_to_use = labels_to_use;
    qDeleteAll(d->m_panels);

    QList<int> list = labels_to_use.toList();
    qSort(list);

    {
        MVSpikeSprayPanel* P = d->add_panel();
        P->setLabelsToUse(labels_to_use);
        P->setLegendVisible(false);
    }
    if (list.count() > 1) {
        for (int i = 0; i < list.count(); i++) {
            MVSpikeSprayPanel* P = d->add_panel();
            QSet<int> tmp;
            tmp.insert(list[i]);
            P->setLabelsToUse(tmp);
        }
    }
    recalculate();
}

void MVSpikeSprayView::prepareCalculation()
{
    QString timeseries_name = d->m_context->option("timeseries_for_spikespray").toString();
    if (timeseries_name.isEmpty()) timeseries_name=d->m_context->currentTimeseriesName();
    this->setCalculatingMessage(QString("Calculating using %1...").arg(timeseries_name));
    d->m_labels_to_render.clear();
    d->m_computer.mlproxy_url = d->m_context->mlProxyUrl();
    d->m_computer.timeseries = d->m_context->timeseries(timeseries_name);
    d->m_computer.firings = d->m_context->firings();
    d->m_computer.filter = d->m_context->eventFilter();
    d->m_computer.labels_to_use = d->m_labels_to_use;
    d->m_computer.clip_size = d->m_context->option("clip_size").toInt();

    d->m_amplitude_factor = 0;
}

void MVSpikeSprayView::runCalculation()
{
    d->m_computer.compute();
}

void MVSpikeSprayView::onCalculationFinished()
{
    d->m_clips_to_render = d->m_computer.clips_to_render;
    d->m_labels_to_render = d->m_computer.labels_to_render;
    if (mvContext()->viewMerged()) {
        d->m_labels_to_render = d->m_context->clusterMerge().mapLabels(d->m_labels_to_render);
    }

    if (!d->m_amplitude_factor) {
        double maxval = qMax(qAbs(d->m_clips_to_render.minimum()), qAbs(d->m_clips_to_render.maximum()));
        if (maxval) {
            d->set_amplitude_factor(1.5 / maxval);
        }
    }

    for (long i = 0; i < d->m_panels.count(); i++) {
        d->m_panels[i]->setClipsToRender(&d->m_clips_to_render);
        d->m_panels[i]->setLabelsToRender(d->m_labels_to_render);
    }
}

void MVSpikeSprayView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
}

void MVSpikeSprayView::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Up) {
        this->slot_vertical_zoom_in();
    } else if (evt->key() == Qt::Key_Down) {
        this->slot_vertical_zoom_out();
    } else {
        QWidget::keyPressEvent(evt);
    }
}

void MVSpikeSprayView::wheelEvent(QWheelEvent* evt)
{
    if (evt->delta() > 0) {
        slot_zoom_in();
    } else if (evt->delta() < 0) {
        slot_zoom_out();
    }
}

void MVSpikeSprayView::slot_zoom_in()
{
    d->set_panel_width(d->actual_panel_width() + 10);
}

void MVSpikeSprayView::slot_zoom_out()
{
    d->set_panel_width(qMax(d->actual_panel_width() - 10, 30.0));
}

void MVSpikeSprayView::slot_vertical_zoom_in()
{
    d->set_amplitude_factor(d->m_amplitude_factor * 1.2);
}

void MVSpikeSprayView::slot_vertical_zoom_out()
{
    d->set_amplitude_factor(d->m_amplitude_factor / 1.2);
}

void MVSpikeSprayView::slot_shift_colors_left(int step)
{
    int shift = this->mvContext()->option("cluster_color_index_shift", 0).toInt();
    shift += step;
    this->mvContext()->setOption("cluster_color_index_shift", shift);
    this->recalculate();
}

void MVSpikeSprayView::slot_shift_colors_right()
{
    slot_shift_colors_left(-1);
}

MVSpikeSprayPanel* MVSpikeSprayViewPrivate::add_panel()
{
    MVSpikeSprayPanel* P = new MVSpikeSprayPanel(m_context);
    m_panel_layout->addWidget(P);
    m_panels << P;
    P->setAmplitudeFactor(m_amplitude_factor);
    P->setMinimumWidth(m_panel_width);
    return P;
}

void MVSpikeSprayViewPrivate::set_amplitude_factor(double val)
{
    m_amplitude_factor = val;
    for (int i = 0; i < m_panels.count(); i++) {
        m_panels[i]->setAmplitudeFactor(m_amplitude_factor);
    }
}

void MVSpikeSprayViewPrivate::set_panel_width(double w)
{
    m_panel_width = w;
    for (int i = 0; i < m_panels.count(); i++) {
        m_panels[i]->setMinimumWidth(w);
    }
}

double MVSpikeSprayViewPrivate::actual_panel_width()
{
    if (m_panels.isEmpty())
        return m_panel_width;
    else
        return m_panels[0]->width();
}

void MVSpikeSprayComputer::compute()
{
    TaskProgress task("Spike spray computer");

    labels_to_render.clear();

    firings = compute_filtered_firings_remotely(mlproxy_url, firings, filter);

    QString firings_out_path;
    {
        QList<int> list = labels_to_use.toList();
        qSort(list);
        QString labels_str;
        foreach(int x, list)
        {
            if (!labels_str.isEmpty())
                labels_str += ",";
            labels_str += QString("%1").arg(x);
        }

        MountainProcessRunner MT;
        QString processor_name = "mv_subfirings";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["firings"] = firings.makePath();
        params["labels"] = labels_str;
        params["max_per_label"] = 512;
        MT.setInputParameters(params);
        MT.setMLProxyUrl(mlproxy_url);

        firings_out_path = MT.makeOutputFilePath("firings_out");

        MT.runProcess();

        if (MLUtil::threadInterruptRequested()) {
            task.error(QString("Halted while running process: " + processor_name));
            return;
        }
    }

    task.setProgress(0.25);
    task.log("firings_out_path: " + firings_out_path);

    QString clips_path;
    {
        MountainProcessRunner MT;
        QString processor_name = "extract_clips";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["timeseries"] = timeseries.makePath();
        params["firings"] = firings_out_path;
        params["clip_size"] = clip_size;
        MT.setInputParameters(params);
        MT.setMLProxyUrl(mlproxy_url);

        clips_path = MT.makeOutputFilePath("clips");

        MT.runProcess();
        if (MLUtil::threadInterruptRequested()) {
            task.error(QString("Halted while running process: " + processor_name));
            return;
        }
    }

    task.setProgress(0.5);
    task.log("clips_path: " + clips_path);

    DiskReadMda clips0(clips_path);
    clips0.readChunk(clips_to_render, 0, 0, 0, clips0.N1(), clips0.N2(), clips0.N3());

    task.setProgress(0.75);

    DiskReadMda firings2(firings_out_path);
    task.log(QString("%1x%2 from %3x%4 (%5x%6x%7) (%8)").arg(firings2.N1()).arg(firings2.N2()).arg(firings.N1()).arg(firings.N2()).arg(clips0.N1()).arg(clips0.N2()).arg(clips0.N3()).arg(clips_to_render.N3()));
    Mda firings0;
    firings2.readChunk(firings0, 0, 0, firings2.N1(), firings2.N2());
    task.setProgress(0.9);
    labels_to_render.clear();
    for (long i = 0; i < firings0.N2(); i++) {
        int label0 = (int)firings0.value(2, i);
        labels_to_render << label0;
    }
}

MVSpikeSprayFactory::MVSpikeSprayFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
    connect(context, SIGNAL(selectedClustersChanged()),
            this, SLOT(updateEnabled()));
    updateEnabled();
}

QString MVSpikeSprayFactory::id() const
{
    return QStringLiteral("open-spike-spray");
}

QString MVSpikeSprayFactory::name() const
{
    return tr("Spike Spray");
}

QString MVSpikeSprayFactory::title() const
{
    return tr("Spike Spray");
}

MVAbstractView* MVSpikeSprayFactory::createView(QWidget* parent)
{
    Q_UNUSED(parent)
    QList<int> ks = mvContext()->selectedClusters();
    if (ks.isEmpty())
        ks = mvContext()->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.isEmpty()) {
        QMessageBox::warning(0, "Unable to open spike spray", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    MVSpikeSprayView* X = new MVSpikeSprayView(mvContext());
    X->setLabelsToUse(ks.toSet());
    return X;
}

void MVSpikeSprayFactory::updateEnabled()
{
    setEnabled(!mvContext()->selectedClusters().isEmpty());
}
