/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "mvclipswidget.h"
#include "mvclipsview.h"
#include <QHBoxLayout>
#include "computationthread.h"
#include "mountainsortthread.h"
#include "msmisc.h"

class MVClipsWidgetComputer : public ComputationThread {
public:
    //input
    DiskReadMda firings;
    DiskReadMda timeseries;
    int clip_size;
    QList<int> labels_to_use;

    //output
    Mda clips;
    QList<double> times;

    void compute();
};

class MVClipsWidgetPrivate {
public:
    MVClipsWidget* q;
    DiskReadMda m_timeseries;
    DiskReadMda m_firings;
    QList<int> m_labels_to_use;
    int m_clip_size;
    MVClipsView* m_view;
    MVClipsWidgetComputer m_computer;

    void start_computation();
};

MVClipsWidget::MVClipsWidget()
{
    d = new MVClipsWidgetPrivate;
    d->q = this;

    d->m_view = MVClipsView::newInstance();

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(d->m_view);
    this->setLayout(hlayout);

    connect(&d->m_computer, SIGNAL(computationFinished()), this, SLOT(slot_computation_finished()));
    connect(d->m_view,SIGNAL(currentEventChanged()),this,SIGNAL(currentEventChanged()));
}

MVClipsWidget::~MVClipsWidget()
{
    delete d;
}

void MVClipsWidget::setTimeseries(DiskReadMda& X)
{
    d->m_timeseries = X;
    d->start_computation();
}

void MVClipsWidget::setFirings(DiskReadMda& F)
{
    d->m_firings = F;
    d->start_computation();
}

void MVClipsWidget::setLabelsToUse(const QList<int>& labels)
{
    d->m_labels_to_use = labels;
    d->start_computation();
}

void MVClipsWidget::setClipSize(int clip_size)
{
    d->m_clip_size = clip_size;
    d->start_computation();
}

int MVClipsWidget::currentClipIndex()
{
    return d->m_view->currentClipIndex();
}

MVEvent MVClipsWidget::currentEvent()
{
    return d->m_view->currentEvent();
}

void MVClipsWidget::setCurrentEvent(MVEvent evt)
{
    d->m_view->setCurrentEvent(evt);
}

void MVClipsWidget::slot_computation_finished()
{
    d->m_computer.stopComputation(); //because I'm paranoid
    d->m_view->setClips(d->m_computer.clips);
    d->m_view->setTimes(d->m_computer.times);
}

void MVClipsWidgetComputer::compute()
{
    QString firings_out_path;
    {
        QString labels_str;
        foreach (int x, labels_to_use) {
            if (!labels_str.isEmpty())
                labels_str += ",";
            labels_str += QString("%1").arg(x);
        }

        MountainsortThread MT;
        QString processor_name = "mv_subfirings";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["firings"] = firings.path();
        params["labels"] = labels_str;
        MT.setInputParameters(params);

        firings_out_path = MT.makeOutputFilePath("firings_out");

        MT.compute();
    }

    QString clips_path;
    {
        MountainsortThread MT;
        QString processor_name = "extract_clips";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["timeseries"] = timeseries.path();
        params["firings"] = firings_out_path;
        params["clip_size"] = clip_size;
        MT.setInputParameters(params);

        clips_path = MT.makeOutputFilePath("clips");

        MT.compute();
    }
    DiskReadMda firings_out(firings_out_path);
    times.clear();
    for (long j = 0; j < firings_out.N2(); j++) {
        times << firings_out.value(1, j);
    }
    DiskReadMda CC(clips_path);
    CC.readChunk(clips, 0, 0, 0, CC.N1(), CC.N2(), CC.N3());
}

void MVClipsWidgetPrivate::start_computation()
{
    m_computer.stopComputation();
    m_computer.firings = m_firings;
    m_computer.timeseries = m_timeseries;
    m_computer.labels_to_use = m_labels_to_use;
    m_computer.clip_size = m_clip_size;
    m_computer.startComputation();
}
