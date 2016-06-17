#include "mvclipsview.h"
#include <QDebug>

class MVClipsViewPrivate {
public:
    MVClipsView* q;

    DiskReadMda m_clips;
    //QList<double> m_times;
    //QList<int> m_labels;
    MVViewAgent* m_view_agent;
};

MVClipsView::MVClipsView(MVViewAgent* view_agent)
{
    d = new MVClipsViewPrivate;
    d->q = this;

    d->m_view_agent = view_agent;
}

MVClipsView::~MVClipsView()
{
    delete d;
}

void MVClipsView::setClips(const DiskReadMda& clips)
{
    d->m_clips = clips;
}

/*
void MVClipsView::setTimes(const QList<double>& times)
{
    d->m_times = times;
}

void MVClipsView::setLabels(const QList<int>& labels)
{
    d->m_labels = labels;
}
*/
