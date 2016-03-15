#include "mvclipsview.h"
#include <QDebug>

class MVClipsViewPrivate {
public:
    MVClipsView *q;
    Mda m_clips;
    QList<double> m_times;
	QList<int> m_labels;
	int m_current_clip_index;

	void set_current_clip_index(int index);
};

MVClipsView::MVClipsView()
{
    d=new MVClipsViewPrivate;
    d->q=this;
	d->m_current_clip_index=-1;
    this->initialize();
	this->plot()->setControlPanelVisible(false);
	//connect(this,SIGNAL(currentXChanged()),this,SIGNAL(currentClipTimepointChanged()));
	connect(this,SIGNAL(currentXChanged()),this,SLOT(slot_current_x_changed()));
}

MVClipsView::~MVClipsView()
{
    delete d;
}

void MVClipsView::setClips(const Mda &clips)
{
    d->m_clips=clips;
    DiskArrayModel *DAM=new DiskArrayModel;
    DAM->setFromMda(d->m_clips);
    this->setData(DAM,true);
}

void MVClipsView::setTimes(const QList<double> &times)
{
	d->m_times=times;
}

void MVClipsView::setLabels(const QList<int> &labels)
{
	d->m_labels=labels;
}

int MVClipsView::currentClipIndex()
{
	return d->m_current_clip_index;
}

MVEvent MVClipsView::currentEvent()
{
	MVEvent evt;
	if (d->m_current_clip_index<0) {
		evt.time=-1;
		evt.label=-1;
		return evt;
	}
	evt.time=d->m_times.value(d->m_current_clip_index);
	evt.label=d->m_labels.value(d->m_current_clip_index);
	return evt;
}

void MVClipsView::setCurrentEvent(MVEvent evt)
{
	if ((d->m_times.isEmpty())||(d->m_labels.isEmpty())) return;
	for (int i=0; i<d->m_times.count(); i++) {
		if ((evt.time==d->m_times[i])&&(evt.label==d->m_labels.value(i))) {
			d->set_current_clip_index(i);
			return;
		}
	}
}

void MVClipsView::slot_current_x_changed()
{
	int T=d->m_clips.N2();
	//int Tmid=(int)((T+1)/2)-1;
	double tp=this->currentX();
	int clip_index=(int)(tp/T);
	d->set_current_clip_index(clip_index);
	//double offset=tp-(clip_index*T)-Tmid;
	//qDebug() << tp << clip_index << offset;
	//return d->m_times.value(clip_index)+offset;
}


void MVClipsViewPrivate::set_current_clip_index(int index)
{
	int T=m_clips.N2();
	int Tmid=(int)((T+1)/2)-1;
	if (m_current_clip_index==index) return;
	m_current_clip_index=index;
	if (m_current_clip_index>=0) {
		q->setCurrentX(index*T+Tmid);
	}
	emit q->currentEventChanged();
}
