#include "mvclipsview.h"
#include <QDebug>

/// TODO labels above the clips in the clips view -- but rewrite the whole thing anyway

class MVClipsViewImpl : public MVClipsView {
    Q_OBJECT
public:
    friend class MVClipsViewPrivate;
    MVClipsViewImpl();
    virtual ~MVClipsViewImpl();
    void setClips(const Mda& clips);
    void setTimes(const QList<double>& times);
    void setLabels(const QList<int>& labels);
    int currentClipIndex();
    //double currentClipTimepoint();
    void setViewAgent(MVViewAgent* agent);
public slots:
    void slot_current_x_changed();

private:
    Mda m_clips;
    QList<double> m_times;
    QList<int> m_labels;
    int m_current_clip_index;
    MVViewAgent* m_view_agent;
    void set_current_clip_index(int index);
};
#include "mvclipsview.moc"
MVClipsView* MVClipsView::newInstance()
{
    return new MVClipsViewImpl;
}

MVClipsViewImpl::MVClipsViewImpl()
{
    m_current_clip_index = -1;
    m_view_agent = 0;
    this->initialize();
    this->plot()->setControlPanelVisible(false);
    //connect(this,SIGNAL(currentXChanged()),this,SIGNAL(currentClipTimepointChanged()));
    connect(this, SIGNAL(currentXChanged()), this, SLOT(slot_current_x_changed()));
}

MVClipsViewImpl::~MVClipsViewImpl()
{
}

void MVClipsViewImpl::setClips(const Mda& clips)
{
    m_clips = clips;
    DiskArrayModel_New* DAM = new DiskArrayModel_New;
    DAM->setFromMda(m_clips);
    this->setData(DAM, true);
}

void MVClipsViewImpl::setTimes(const QList<double>& times)
{
    m_times = times;
}

void MVClipsViewImpl::setLabels(const QList<int>& labels)
{
    m_labels = labels;
}

int MVClipsViewImpl::currentClipIndex()
{
    return m_current_clip_index;
}

void MVClipsViewImpl::setViewAgent(MVViewAgent* agent)
{
    m_view_agent = agent;
}

void MVClipsViewImpl::slot_current_x_changed()
{
    int T = m_clips.N2();
    //int Tmid=(int)((T+1)/2)-1;
    double tp = this->currentX();
    int clip_index = (int)(tp / T);
    set_current_clip_index(clip_index);
    //double offset=tp-(clip_index*T)-Tmid;
    //return m_times.value(clip_index)+offset;
}

void MVClipsViewImpl::set_current_clip_index(int index)
{
    int T = m_clips.N2();
    int Tmid = (int)((T + 1) / 2) - 1;
    if (m_current_clip_index == index)
        return;
    m_current_clip_index = index;
    if (m_current_clip_index >= 0) {
        setCurrentX(index * T + Tmid);
    }
    if (m_view_agent) {
        MVEvent evt;
        evt.time = m_times.value(index);
        evt.label = m_labels.value(index);
        m_view_agent->setCurrentEvent(evt);
        m_view_agent->clickCluster(evt.label, Qt::NoModifier);
    }
}
