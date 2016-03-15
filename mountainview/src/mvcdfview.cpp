#include "mvcdfview.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <math.h>
#include "static_neuron_colors.h"

class MVCdfViewPrivate {
public:
	MVCdfView *q;
	QVector<int> m_times;
	QVector<int> m_labels;
	int m_max_time;
	int m_max_count;
	int m_current_label;
	int m_current_timepoint;
	QList<QPainterPath> m_painter_paths;

	QPointF coord2pix(const QPointF &pt);
	QPointF pix2coord(const QPointF &pt);
	int find_closest_label(QPointF &pt);
};

MVCdfView::MVCdfView()
{
	d=new MVCdfViewPrivate;
	d->q=this;
	d->m_max_time=0;
	d->m_max_count=0;
	d->m_current_label=0;
	d->m_current_timepoint=-1;
}

MVCdfView::~MVCdfView()
{
	delete d;
}

void MVCdfView::setTimesLabels(const QVector<int> &times, const QVector<int> &labels)
{
	d->m_times=times;
	d->m_labels=labels;
}

void MVCdfView::setTimesLabels(const Mda &times, const Mda &labels)
{
	QVector<int> times0,labels0;
	for (int i=0; i<times.totalSize(); i++) {
		times0 << times.get(i);
		labels0 << (int)labels.get(i);
	}
	setTimesLabels(times0,labels0);
}

void MVCdfView::setCurrentLabel(int val)
{
	if (val!=d->m_current_label) {
		d->m_current_label=val;
		emit currentLabelChanged();
		update();
	}
}

int MVCdfView::currentLabel()
{
	return d->m_current_label;
}

void MVCdfView::setCurrentTimepoint(int t0)
{
	if (d->m_current_timepoint==t0) return;
	d->m_current_timepoint=t0;
	emit currentTimepointChanged();
	update();
}

int MVCdfView::currentTimepoint()
{
	return d->m_current_timepoint;
}

typedef QList<int> IntList;
void MVCdfView::paintEvent(QPaintEvent *evt)
{
	Q_UNUSED(evt)
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QList<IntList> label_times;

	int max_time=0;
	int max_count=0;
	for (int i=0; i<d->m_times.count(); i++) {
		int label0=d->m_labels.value(i);
		int time0=d->m_times.value(i);
		while (label0>=label_times.count()) {
			IntList empty_list;
			label_times << empty_list;
		}
		label_times[label0] << time0;
		if (time0>max_time) max_time=time0;
		if (label_times[label0].count()>max_count) max_count=label_times[label0].count();
	}
	d->m_max_time=max_time;
	d->m_max_count=max_count;

	if (d->m_current_timepoint>=0) {
		QPointF pt0=d->coord2pix(QPointF(d->m_current_timepoint,0));
		QPointF pt1=d->coord2pix(QPointF(d->m_current_timepoint,d->m_max_count));
		QPen pen=painter.pen();
		QColor col(20,20,20,50);
		pen.setColor(col); pen.setWidth(4);
		painter.setPen(pen);
		painter.drawLine(pt0,pt1);
	}

	d->m_painter_paths.clear(); d->m_painter_paths << QPainterPath();
	for (int pass=1; pass<=2; pass++) {
		for (int k=1; k<label_times.count(); k++) {
			if ((pass==1)||(k==d->m_current_label)) {
				IntList *LL=&label_times[k];
				QPainterPath path0;
				for (int i=0; i<LL->count(); i++) {
					int time0=LL->value(i);
					QPointF pt0=d->coord2pix(QPointF(time0,i));
					if (i==0) path0.moveTo(pt0);
					else path0.lineTo(pt0);
				}
				QPen pen=painter.pen();
				if (k==d->m_current_label) {
					pen.setColor(Qt::black);
					pen.setWidth(4);
				}
				else {
					QColor col=static_neuron_color(k);
					pen.setColor(col);
					pen.setWidth(0);
				}
				if (pass==1) {
					d->m_painter_paths << path0;
				}
				painter.strokePath(path0,pen);
			}
		}
	}

}

void MVCdfView::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt)
	update();
}

void MVCdfView::mousePressEvent(QMouseEvent *evt)
{
	QPointF pt=evt->pos();
	int closest_label=d->find_closest_label(pt);
	this->setCurrentLabel(closest_label);

	QPointF pt2=d->pix2coord(pt);
	int time0=(int)pt2.x();
	this->setCurrentTimepoint(time0);
}


QPointF MVCdfViewPrivate::coord2pix(const QPointF &pt)
{
	float time0=pt.x();
	float count0=pt.y();

	int W0=q->width();
	int H0=q->height();

	if ((W0<=30)||(H0<=30)) {
		return QPointF(0,0);
	}

	float margin_left=10,margin_right=10;
	float margin_top=10,margin_bottom=10;

	float pct_x=time0*1.0/m_max_time;
	float pct_y=count0*1.0/m_max_count;

	float x0=margin_left+pct_x*(W0-margin_left-margin_right);
	float y0=H0-margin_bottom-pct_y*(H0-margin_bottom-margin_top);

	return QPointF(x0,y0);
}

QPointF MVCdfViewPrivate::pix2coord(const QPointF &pt)
{
	float x0=pt.x();
	float y0=pt.y();

	int W0=q->width();
	int H0=q->height();

	if ((W0<=30)||(H0<=30)) {
		return QPointF(0,0);
	}

	float margin_left=10,margin_right=10;
	float margin_top=10,margin_bottom=10;

	float pct_x=(x0-margin_left)/(W0-margin_left-margin_right);
	float pct_y=(H0-margin_bottom-y0)/(H0-margin_bottom-margin_top);

	float time0=pct_x*m_max_time;
	float count0=pct_y*m_max_count;

	return QPointF(time0,count0);
}

float compute_dist(QPointF pt1,QPointF pt2) {
	QPointF pt0=pt1-pt2;
	return sqrt(pt0.x()*pt0.x()+pt0.y()*pt0.y());
}

float compute_distance(QPainterPath *PP,QPointF pt) {
	float lower_pct=0;
	float upper_pct=1;
	while (upper_pct-lower_pct>=0.01) {
		float pct0=(lower_pct+upper_pct)/2;
		QPointF pt0=PP->pointAtPercent(pct0);
		if (pt0.x()<pt.x()) lower_pct=pct0;
		else upper_pct=pct0;
	}
	{
		float pct0=(lower_pct+upper_pct)/2;
		QPointF pt0=PP->pointAtPercent(pct0);
		return compute_dist(pt0,pt);
	}
}

int MVCdfViewPrivate::find_closest_label(QPointF &pt)
{
	float best_dist=9999;
	int ret=0;
	for (int i=0; i<m_painter_paths.count(); i++) {
		QPainterPath *PP=&m_painter_paths[i];
		float dist0=compute_distance(PP,pt);
		if (dist0<best_dist) {
			best_dist=dist0;
			ret=i;
		}
	}
	return ret;
}
