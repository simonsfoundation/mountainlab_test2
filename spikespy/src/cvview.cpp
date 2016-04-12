#include "affinetransformation.h"
#include "cvview.h"
#include "memorymda_prev.h"

#include <QPainter>
#include <QImage>
#include <math.h>
#include <QMouseEvent>
#include <QTimer>
#include <QTime>
#include <QApplication>
#include <QPair>
#include <QWheelEvent>
#include <QSet>

//shuffling
struct QPairFirstComparer
{
	template<typename T1, typename T2>
	bool operator()(const QPair<T1,T2> & a, const QPair<T1,T2> & b) const
	{return a.first < b.first;}
};

class CVViewPrivate {
public:
	CVView *q;
	QList<CVDataPoint> m_data_points;
    int m_max_label;
	MemoryMda m_grid;

	AffineTransformation m_view_transformation;
	AffineTransformation m_inverse_view_transformation;
	QImage m_image;
	QPoint m_drag_anchor;
	QList<long> m_shuffling;
	long m_shuffling_offset;
	float m_window_max;
	float m_increment_weight;
	bool m_need_initialize_window;
	QList<QColor> m_label_colors;
	float m_radius;
	QSet<long> m_active_pixels;
	float m_scale;
	float m_dx,m_dy;
	float m_perspective_factor;
	CVLine m_selected_line;
	QList<int> m_selected_point_indices;
	int m_num_datapoints_to_select;
	bool m_is_moving;
    QStringList m_label_strings;

	long m_update_view_code;
	void start_update_view();
	void create_shuffling();
	CVPoint coord_to_pixel(CVPoint p);
	CVPoint pixel_to_coord(CVPoint pp);
	void draw_overlay(QPainter *painter);
    void draw_legend(QPainter *painter);
	void zoom(float factor);
	void set_selected_line(CVLine L);
	QList<int> retrieve_data_point_indices_closest_to_selected_line(int num_to_retrieve);
};

CVView::CVView(QWidget *parent) : QWidget(parent)
{
	d=new CVViewPrivate;
	d->q=this;
    d->m_max_label=1;

	d->m_view_transformation.scale(0.6,0.6,0.6);
	d->m_inverse_view_transformation=d->m_view_transformation.inverse();
	this->setMouseTracking(true);

	d->m_update_view_code=0;
	d->m_window_max=0.5;
	d->m_increment_weight=1;
	d->m_need_initialize_window=false;
	d->m_radius=0.5;
	d->m_scale=1;
	d->m_dx=d->m_dy=0;
	d->m_perspective_factor=1.2;
	d->m_selected_line=cvline(0,0,0,0,0,0);
	d->m_num_datapoints_to_select=10;
	d->m_is_moving=false;

	connect(this,SIGNAL(signal_do_update_view(long,long)),this,SLOT(slot_update_view(long,long)),Qt::QueuedConnection);
}

CVView::~CVView()
{
	delete d;
}

void CVView::addDataPoints(const QList<CVDataPoint> &points)
{
    for (int i=0; i<points.count(); i++) {
        if (points[i].label>d->m_max_label) {
            d->m_max_label=points[i].label;
        }
    }
	d->m_data_points.append(points);
	d->m_shuffling.clear();
	d->start_update_view();
	d->m_need_initialize_window=true;
}

void CVView::setLabelColors(const QList<QColor> colors)
{
	d->m_label_colors=colors;
}

CVLine CVView::selectedLine()
{
	return d->m_selected_line;
}

QList<int> CVView::selectedDataPointIndices()
{
	return d->m_selected_point_indices;
}

void CVView::setSelectedDataPointIndices(const QList<int> &L)
{
	d->m_selected_line=cvline(0,0,0,0,0,0);
	d->m_selected_point_indices=L;
	emit selectedLineChanged();
	emit selectedDataPointsChanged();
	d->start_update_view();
}

void CVView::setNumDataPointsToSelect(int num)
{
    d->m_num_datapoints_to_select=num;
}

void CVView::setLabelStrings(const QStringList &strings)
{
    d->m_label_strings=strings;
    update();
}

float compute_distance_from_point_to_line(const CVPoint &P,const CVLine &L) {
	//is there a faster formula? If anyone can help it would be appreciated.

	//first we move the point to zero
	CVLine L2=cvline(L.p1.x-P.x,L.p1.y-P.y,L.p1.z-P.z,L.p2.x-P.x,L.p2.y-P.y,L.p2.z-P.z);

	//Define n = the direction as a unit vector
	CVPoint n=cvpoint(L2.p2.x-L2.p1.x,L2.p2.y-L2.p1.y,L2.p2.z-L2.p1.z);
	float mag=sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
	if (mag>0) {n.x/=mag; n.y/=mag; n.z/=mag;}

	//find the point along the line closest to zero
	float dotprod=L2.p1.x*n.x+L2.p1.y*n.y+L2.p1.z*n.z;
	CVPoint q=cvpoint(L2.p1.x-dotprod*n.x,L2.p1.y-dotprod*n.y,L2.p1.z-dotprod*n.z);

	//now return the distance from zero
	return sqrt(q.x*q.x+q.y*q.y+q.z*q.z);
}

QList<int> CVViewPrivate::retrieve_data_point_indices_closest_to_selected_line(int num_to_retrieve)
{
	QList<QPair<float,int> > array;
	for (int i=0; i<m_data_points.count(); i++) {
		float dist=compute_distance_from_point_to_line(m_data_points[i].p,m_selected_line);
		array.append(qMakePair(dist,i));
	}

	// Ordering ascending
	qSort(array.begin(), array.end(), QPairFirstComparer());

	QList<int> ret;
	for (int j=0; (j<array.count())&&(j<num_to_retrieve); j++) {
		ret << array[j].second;
	}

	return ret;
}

void draw_point(MemoryMda *X,float x,float y,float r,float g,float b,float rad,QSet<long> &active_pixels) {
	for (int dy=-rad-1; dy<=rad+1; dy++) {
		int y0=(int)(y+dy+0.5);
		if ((0<=y0)&&(y0<X->N2())) {
			for (int dx=-rad-1; dx<=rad+1; dx++) {
				float dist=sqrt(dx*dx+dy*dy);
				if ((dist<=rad)) {
					int x0=(int)(x+dx+0.5);
					if ((0<=x0)&&(x0<X->N1())) {
						float valr=X->value(x0,y0,0)+r*(dist+1)/(rad+1);
						float valg=X->value(x0,y0,1)+g*(dist+1)/(rad+1);
						float valb=X->value(x0,y0,2)+b*(dist+1)/(rad+1);
						X->setValue(valr,x0,y0,0);
						X->setValue(valg,x0,y0,1);
						X->setValue(valb,x0,y0,2);
						active_pixels.insert(x0+X->N1()*y0);
					}
				}
			}
		}
	}
}

QColor combine_colors(const QList<QColor> colors) {
	QColor ret(0,0,0);
	for (int i=0; i<colors.count(); i++) {
		ret.setRed(qMin(255,(ret.red()+colors[i].red())));
		ret.setGreen(qMin(255,(ret.green()+colors[i].green())));
		ret.setBlue(qMin(255,(ret.blue()+colors[i].blue())));
	}
	return ret;
}

void CVView::paintEvent(QPaintEvent *evt)
{
	Q_UNUSED(evt);
	QPainter painter(this);

	QTime timer; timer.start();

	int W=width();
	int H=height();

	int N1=d->m_grid.N1();
	int N2=d->m_grid.N2();
	if ((N1!=W)||(N2!=H)) {
		d->start_update_view();
		return;
	}

	if (d->m_need_initialize_window) {
		float max=0;
		MemoryMda *grid=&d->m_grid;
		foreach (int pt,d->m_active_pixels) {
			long x=pt%N1;
			long y=pt/N1;
			max=qMax(max,(float)grid->value(x,y)*d->m_increment_weight);
		}
		d->m_window_max=max/3;
		d->m_need_initialize_window=false;
	}

	d->m_image=QImage(N1,N2,QImage::Format_ARGB32);
	d->m_image.fill(Qt::black);

	foreach (long pt,d->m_active_pixels) {
		long x=pt%N1;
		long y=pt/N1;
		int r0=(int)qMin(255.0,d->m_grid.value(x,y,0)*1.0/d->m_window_max*d->m_increment_weight*255);
		int g0=(int)qMin(255.0,d->m_grid.value(x,y,1)*1.0/d->m_window_max*d->m_increment_weight*255);
		int b0=(int)qMin(255.0,d->m_grid.value(x,y,2)*1.0/d->m_window_max*d->m_increment_weight*255);
		QColor CC(r0,g0,b0);
		d->m_image.setPixel(x,y,CC.rgb());
	}

	painter.drawImage(0,0,d->m_image);

	d->draw_overlay(&painter);
    d->draw_legend(&painter);
}

void CVView::mouseMoveEvent(QMouseEvent *evt)
{
	if (evt->buttons()&Qt::LeftButton) {
		QPoint delta=evt->pos()-d->m_drag_anchor;

		//don't rotate if no movement
		if (!d->m_is_moving) {
			if ((qAbs(delta.x()<=4))&&(qAbs(delta.y())<=4)) return;
		}
		d->m_is_moving=true;

		d->m_drag_anchor=evt->pos();

		float theta_y=delta.x()*M_PI/180*1.5;
		float theta_x=-delta.y()*M_PI/180*1.5;

		d->m_view_transformation.rotateX(theta_x);
		d->m_view_transformation.rotateY(theta_y);
		d->m_inverse_view_transformation=d->m_view_transformation.inverse();

		d->start_update_view();
	}
}

void CVView::mousePressEvent(QMouseEvent *evt)
{
	d->m_drag_anchor=evt->pos();
	d->m_is_moving=false;
	this->setFocus();
}

void CVView::mouseReleaseEvent(QMouseEvent *evt)
{

	if (!d->m_is_moving) {
		CVPoint pp1=cvpoint(evt->x(),evt->y(),0); pp1.z=-1;
		CVPoint pp2=pp1; pp1.z=1;
		CVPoint qq1=d->pixel_to_coord(pp1);
		CVPoint qq2=d->pixel_to_coord(pp2);

		//now make sure the length of the line is 2
		CVPoint diff=cvpoint(qq2.x-qq1.x,qq2.y-qq1.y,qq2.z-qq1.z);
		CVPoint mid=cvpoint((qq2.x+qq1.x)/2,(qq2.y+qq1.y)/2,(qq2.z+qq1.z)/2);
		float dist=sqrt(diff.x*diff.x+diff.y*diff.y+diff.z*diff.z);
		if (dist>0) {
			qq1=cvpoint(mid.x-diff.x/dist,mid.y-diff.y/dist,mid.z-diff.z/dist);
			qq2=cvpoint(mid.x+diff.x/dist,mid.y+diff.y/dist,mid.z+diff.z/dist);
		}

		d->set_selected_line(cvline(qq1,qq2));
		update();
		d->start_update_view();
	}

	d->m_is_moving=false;
}

void CVView::keyPressEvent(QKeyEvent *evt)
{
	if (evt->key()==Qt::Key_Equal) {
		if (evt->modifiers()&&Qt::ControlModifier) {
			d->m_window_max/=1.2;
			update();
		}
		else {
			d->zoom(1.2);
		}
	}
	else if (evt->key()==Qt::Key_Minus) {
		if (evt->modifiers()&&Qt::ControlModifier) {
			d->m_window_max*=1.2;
			update();
		}
		else {
			d->zoom(1/1.2);
		}
	}
	else if (evt->key()==Qt::Key_R) {
		d->m_radius*=1.3;
		d->m_window_max*=1.3;
		d->start_update_view();
	}
	else if (evt->key()==Qt::Key_S) {
		d->m_radius/=1.3;
		d->m_window_max/=1.3;
		d->start_update_view();
	}
}

void CVView::wheelEvent(QWheelEvent *evt)
{

	if (evt->delta()>0) {
		d->zoom(1.2);

	}
	else if (evt->delta()<0) {
		d->zoom(1/1.2);
	}
}

void CVViewPrivate::start_update_view()
{
	if (m_shuffling.isEmpty())
		create_shuffling();
	if (!m_shuffling.isEmpty()) {
		m_shuffling_offset=qrand()%m_shuffling.count();
	}

	m_update_view_code++;
	emit q->signal_do_update_view(m_update_view_code,0);
}



void CVView::slot_update_view(long code,long data_point_index)
{
	if (code!=d->m_update_view_code) return;

	QSet<int> selected_point_indices=d->m_selected_point_indices.toSet();

	int W=width();
	int H=height();

	if (data_point_index==0) {
		d->m_grid.allocate(W,H,3);
		d->m_active_pixels.clear();
	}

	if (W<H) d->m_scale=W*1.0/2;
	else d->m_scale=H*1.0/2;
	d->m_dx=W*1.0/2;
	d->m_dy=H*1.0/2;
	float rad=d->m_radius*1.0F/100*d->m_scale;
	QTime timer; timer.start();
	for (long ii=data_point_index; ii<d->m_data_points.count(); ii++) {
		long i=d->m_shuffling.value((ii+d->m_shuffling_offset)%d->m_data_points.count());
		d->m_increment_weight=d->m_data_points.count()*1.0F/ii;
		if (code!=d->m_update_view_code) {
			update();
			return;
		}
		if (timer.elapsed()>=150) {
			update();
			qApp->processEvents();
			emit signal_do_update_view(code,ii);
			return;
		}
		CVDataPoint *dp=&d->m_data_points[i];
		if (dp->label>=1) {

			CVPoint pp=d->coord_to_pixel(dp->p);
			float x=pp.x;
			float y=pp.y;

			//float weight=qMin(1.0F,qMax(0.0F,(pp.z+1)/2));
			float weight=1;
			QColor col=d->m_label_colors[(dp->label-1)%d->m_label_colors.count()];
			float r=col.red()*weight;
			float g=col.green()*weight;
			float b=col.blue()*weight;

			float rad0=rad;
			if (selected_point_indices.contains(i)) {
				rad0=rad*2;
				r=0; g=255; b=0;
			}

			draw_point(&d->m_grid,x,y,r,g,b,rad0,d->m_active_pixels);
		}
	}

	if (code!=d->m_update_view_code) return;

	update();
}

void CVViewPrivate::create_shuffling()
{
	QList<QPair<int,long> > array;
	for (long i=0; i<m_data_points.count(); i++) {
		array.append(qMakePair(qrand(),i));
	}

	// Ordering ascending
	qSort(array.begin(), array.end(), QPairFirstComparer());

	m_shuffling.clear();
	for (long i=0; i<m_data_points.count(); i++) {
		m_shuffling.append(array[i].second);
	}
}

CVPoint CVViewPrivate::coord_to_pixel(CVPoint p)
{
	CVPoint pp=m_view_transformation.map(p);
	//depth perspective
	float zdist=m_perspective_factor-pp.z;
	float ff=0;
	if (zdist!=0) ff=1/qAbs(zdist);
	pp.x*=ff; pp.y*=ff;
	pp.x=m_dx+pp.x*m_scale;
	pp.y=m_dy+pp.y*m_scale;

	return pp;
}

CVPoint CVViewPrivate::pixel_to_coord(CVPoint pp)
{
	pp.x=(pp.x-m_dx)/m_scale;
	pp.y=(pp.y-m_dy)/m_scale;
	float zdist=m_perspective_factor-pp.z;
	float ff=0;
	if (zdist!=0) ff=1/qAbs(zdist);
	if (ff) {
		pp.x/=ff;
		pp.y/=ff;
	}
	CVPoint p=m_inverse_view_transformation.map(pp);

	return p;
}

void CVViewPrivate::draw_overlay(QPainter *painter)
{
	if (!is_zero(m_selected_line)) {
		CVPoint p1=coord_to_pixel(m_selected_line.p1);
		CVPoint p2=coord_to_pixel(m_selected_line.p2);
		painter->setPen(QPen(QBrush(QColor(50,70,50)),2));
		painter->drawLine(p1.x,p1.y,p2.x,p2.y);
    }
}

QColor brighten(QColor col,float factor) {
    QColor ret;
    int r=(int)(col.red()*factor); if (r>255) r=255;
    int g=(int)(col.green()*factor); if (g>255) g=255;
    int b=(int)(col.blue()*factor); if (b>255) b=255;
    return QColor(r,g,b);
}

void CVViewPrivate::draw_legend(QPainter *painter)
{
    if (m_label_strings.isEmpty()) return;
    int W0=q->width();
    //int H0=q->height();
    if (m_label_colors.count()==0) return;
    int x0=W0-60;
    int x1=W0-30;
    int vspacing=15;
    int y0=vspacing;
    for (int jj=1; jj<=m_max_label; jj++) {
        QColor col=m_label_colors[(jj-1)%m_label_colors.count()];
        col=brighten(col,1.5);
        QPen pen=painter->pen();
        pen.setWidth(5);
        pen.setColor(col);
        painter->setPen(pen);
        QPointF pt1(x0,y0);
        QPointF pt2(x1,y0);
        painter->drawLine(pt1,pt2);
        QRect RR(x1+9,y0-10,W0-x1-9,20);
        pen.setColor(QColor(200,200,200));
        painter->setPen(pen);
        painter->drawText(RR,QString("%1").arg(m_label_strings.value(jj-1)),Qt::AlignVCenter|Qt::AlignLeft);
        y0+=vspacing;
    }
}

void CVViewPrivate::zoom(float factor)
{
	m_view_transformation.scale(factor,factor,factor);
	m_inverse_view_transformation=m_view_transformation.inverse();
	m_window_max/=factor;
	start_update_view();
}

void CVViewPrivate::set_selected_line(CVLine L)
{
	if (compare(L,m_selected_line)) return;
	m_selected_line=L;
	m_selected_point_indices=retrieve_data_point_indices_closest_to_selected_line(m_num_datapoints_to_select);
	emit q->selectedLineChanged();
	emit q->selectedDataPointsChanged();
}


