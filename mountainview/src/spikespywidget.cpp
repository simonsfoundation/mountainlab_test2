/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/6/2016
*******************************************************/

#include "mvmainwindow.h"
#include "mvtimeseriesview.h"
#include "spikespywidget.h"
#include "taskprogressview.h"
#include "mvviewagent.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenuBar>

/// TODO put channel colors into mvviewagent

class SpikeSpyWidgetPrivate {
public:
    SpikeSpyWidget* q;
    double m_samplerate;
    QList<QColor> m_channel_colors;
    QList<SpikeSpyViewData> m_datas;
    QList<MVTimeSeriesView*> m_views;
    MVViewAgent* m_view_agent;
    int m_current_view_index;

    QSplitter* m_splitter;
    TaskProgressView* m_task_progress_view;

    QMenuBar* m_menu_bar;

    void update_activated();
};

SpikeSpyWidget::SpikeSpyWidget(MVViewAgent* view_agent)
{
    d = new SpikeSpyWidgetPrivate;
    d->q = this;
    d->m_samplerate = 0;
    d->m_view_agent = view_agent;
    d->m_current_view_index = 0;

    d->m_splitter = new QSplitter(Qt::Vertical);
    d->m_task_progress_view = new TaskProgressView;
    d->m_task_progress_view->setWindowFlags(Qt::Tool);

    QMenuBar* menubar = new QMenuBar(this);
    d->m_menu_bar = menubar;
    { //Tools
        QMenu* menu = new QMenu("Tools", menubar);
        d->m_menu_bar->addMenu(menu);
        {
            QAction* A = new QAction("Open MountainView", menu);
            menu->addAction(A);
            A->setObjectName("open_mountainview");
            A->setShortcut(QKeySequence("Ctrl+M"));
            connect(A, SIGNAL(triggered()), this, SLOT(slot_open_mountainview()));
        }
        {
            QAction* A = new QAction("Show tasks (for debugging)", menu);
            menu->addAction(A);
            A->setObjectName("show_tasks");
            A->setShortcut(QKeySequence("Ctrl+D"));
            connect(A, SIGNAL(triggered()), this, SLOT(slot_show_tasks()));
        }
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(d->m_menu_bar);
    layout->addWidget(d->m_splitter);
    d->m_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(layout);
}

SpikeSpyWidget::~SpikeSpyWidget()
{
    delete d->m_task_progress_view;
    delete d;
}

void SpikeSpyWidget::setSampleRate(double samplerate)
{
    d->m_samplerate = samplerate;
    foreach(MVTimeSeriesView * V, d->m_views)
    {
        V->setSampleRate(samplerate);
    }
}

void SpikeSpyWidget::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
    foreach(MVTimeSeriesView * V, d->m_views)
    {
        V->setChannelColors(colors);
    }
}

void SpikeSpyWidget::setLabelColors(const QList<QColor>& colors)
{
    d->m_view_agent->setClusterColors(colors);
}

void SpikeSpyWidget::addView(const SpikeSpyViewData& data)
{
    MVTimeSeriesView* W = new MVTimeSeriesView(d->m_view_agent);
    W->setChannelColors(d->m_channel_colors);
    W->setTimeseries(data.timeseries);
    QVector<double> times;
    QVector<int> labels;
    for (long i = 0; i < data.firings.N2(); i++) {
        times << data.firings.value(1, i);
        labels << (int)data.firings.value(2, i);
    }
    W->setTimesLabels(times, labels);
    W->setSampleRate(d->m_samplerate);
    d->m_splitter->addWidget(W);
    d->m_views << W;
    d->m_datas << data;

    connect(W, SIGNAL(clicked()), this, SLOT(slot_view_clicked()));

    d->update_activated();
}

void SpikeSpyWidget::slot_show_tasks()
{
    d->m_task_progress_view->show();
    d->m_task_progress_view->raise();
}

void SpikeSpyWidget::slot_open_mountainview()
{
    int current_view_index = d->m_current_view_index;
    if (current_view_index >= d->m_datas.count())
        return;
    SpikeSpyViewData data = d->m_datas[current_view_index];

    MVMainWindow* W = new MVMainWindow(d->m_view_agent);
    W->setChannelColors(d->m_channel_colors);
    MVFile ff;
    ff.addTimeseriesPath("Timeseries", data.timeseries.path());
    ff.setFiringsPath(data.firings.path());
    ff.setSampleRate(d->m_samplerate);
    W->setMVFile(ff);
    W->setDefaultInitialization();
    W->show();
    W->setGeometry(this->geometry().adjusted(50, 50, 50, 50));
}

void SpikeSpyWidget::slot_view_clicked()
{
    /// TODO highlight the current view differently
    for (int i = 0; i < d->m_views.count(); i++) {
        if (d->m_views[i] == sender()) {
            d->m_current_view_index = i;
        }
    }
    d->update_activated();
}

void SpikeSpyWidgetPrivate::update_activated()
{
    for (int i = 0; i < m_views.count(); i++) {
        m_views[i]->setActivated(i == m_current_view_index);
    }
}
