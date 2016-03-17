#include "ssabstractview.h"
#include "sstimeseriesplot.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QDebug>
#include "sslabelsmodel1.h"
#include "diskreadmda.h"

class SSAbstractViewUnderlayPainter : public OverlayPainter {
public:
	SSAbstractViewPrivate *d;
	SSAbstractView *q;
	void paint(QPainter *painter);
	void draw_time_axis(QPainter *painter);
};

class SSAbstractViewPrivate {
public:
	SSAbstractView *q;

	double m_current_x;
	double m_selected_xmin;
	double m_selected_xmax;
	int m_current_channel;
    double m_samplerate;
	bool m_signals_enabled;
	bool m_timeline_visible;
	QString m_title;
	bool m_activated;

	void do_translate(int dx);
    void translate_to(int x_left);
	void zoom_in();
	void zoom_out();
	void do_zoom(double center_x, double frac);
	void do_zoom2(double xmin,double xmax);
	void scroll_to_current_x_if_needed(bool force_center=false);
	void on_left_right_arrow(bool left);

	DiskReadMda m_timepoint_mapping;

	int m_max_timepoint;
	double m_click_anchor;
    double m_click_anchor_x_left;
	QPoint m_click_anchor_pix;
	double m_selected_x_anchor;
	bool m_is_moving;
	SSAbstractViewUnderlayPainter *m_underlay_painter;
	bool m_cursor_visible;
};


SSAbstractView::SSAbstractView(QWidget *parent) : QWidget(parent) {
	d=new SSAbstractViewPrivate;
	d->q=this;

    d->m_samplerate=0;
	d->m_signals_enabled=true;
	d->m_current_x=-1;
	d->m_selected_xmin=-1;
	d->m_selected_xmax=-1;
	d->m_current_channel=-1;
	d->m_timeline_visible=true;
	d->m_activated=true;

	d->m_cursor_visible=true;
	d->m_click_anchor=-1;
    d->m_click_anchor_x_left=0;
	d->m_click_anchor_pix=QPoint(-1,-1);
	d->m_selected_x_anchor=-1;
	d->m_max_timepoint=0;

	d->m_underlay_painter=new SSAbstractViewUnderlayPainter;
	d->m_underlay_painter->d=d;
	d->m_underlay_painter->q=this;
	
	//this->setMouseTracking(true);
	this->setFocusPolicy(Qt::StrongFocus);
}

SSAbstractView::~SSAbstractView() {
	delete d;
}

void SSAbstractView::setTitle(QString title)
{
	d->m_title=title;
}


void SSAbstractViewUnderlayPainter::paint(QPainter *painter) {
	q->plot()->updateSize();

	int H=q->plot()->height();

	Vec2 SR=q->selectionRange();
	double selected_xmin=SR.x;
	double selected_xmax=SR.y;

	//current location
	if (selected_xmin < 0) {
		Vec2 p0 = q->plot()->coordToPix(vec2(q->currentX(), 0)); Q_UNUSED(p0);
		Vec2 p1 = q->plot()->coordToPix(vec2(q->currentX(), 0));
		p0.y = 0;
		p1.y = H;

		if ((d->m_cursor_visible)&&(q->currentX()>=0)) {

            for (int pass=1; pass<=2; pass++) {
                QPainterPath path;
                Vec2 pp=p0;
                int sign=-1;
                if (pass==2) {
                    pp=p1; sign=1;
                }
                path.moveTo(pp.x,pp.y-10*sign);
                path.lineTo(pp.x-8,pp.y-2*sign);
                path.lineTo(pp.x+8,pp.y-2*sign);
                path.lineTo(pp.x,pp.y-10*sign);
                QColor col=QColor(60,80,60);
                if (d->m_activated) {
                    //col=Qt::gray;
                    col=QColor(50,50,220);
                }
                painter->fillPath(path,QBrush(col));
            }


            QPainterPath path2;
            path2.moveTo(p0.x,p0.y+10);
            path2.lineTo(p1.x,p1.y-10);
			painter->setPen(QPen(QBrush(QColor(50,50,220,60)),0));
            painter->drawPath(path2);

			//painter->setPen(QPen(QBrush(QColor(50,50,220,180)),1));
			//painter->drawPath(path2);

		} else {
		}
	}

	//selected
	if (selected_xmin >= 0) {
		double ymin = q->plot()->yRange().x;
		double ymax = q->plot()->yRange().y;
		Vec2 p0 = q->plot()->coordToPix(vec2(selected_xmin, ymin));
		Vec2 p1 = q->plot()->coordToPix(vec2(selected_xmax, ymax));

		QPainterPath path;
		path.moveTo(p0.x,p0.y);
		path.lineTo(p1.x, p0.y);
		path.lineTo(p1.x, p1.y);
		path.lineTo(p0.x, p1.y);
		path.lineTo(p0.x, p0.y);

		int pen_width=6;
		QColor pen_color=QColor(150,150,150);
		painter->setPen(QPen(QBrush(pen_color),pen_width));
		painter->drawPath(path);
	}

	if ((d->m_timeline_visible)&&(d->m_timepoint_mapping.totalSize()==1))
		draw_time_axis(painter);

	//draw title
	Vec2 range=q->plot()->xRange();
	Vec2 pix1=q->plot()->coordToPix(vec2(range.x,0));
	//Vec2 pix2=q->plot()->coordToPix(vec2(range.y,0));
	painter->setFont(QFont("Arial",8));\
	painter->setPen(QPen(Qt::darkGray));
	painter->drawText(pix1.x+8,q->plot()->height()-6,d->m_title);
}
void SSAbstractView::mousePressEvent(QMouseEvent *evt) {
	emit clicked();
	QPoint pos=evt->pos()-plot()->pos();
	Vec2 coord = plot()->pixToCoord(vec2(pos.x(), pos.y()));
	double x0 = (int) coord.x;
	if (evt->button()==Qt::LeftButton) {
		d->m_click_anchor=x0;
        d->m_click_anchor_x_left=this->xRange().x;
		d->m_click_anchor_pix=evt->pos();
		d->m_selected_x_anchor=-1;
		setSelectionRange(vec2(-1,-1));
	} else if (evt->button()==Qt::RightButton) {
		this->setSelectionRange(vec2(-1,-1));
		d->m_selected_x_anchor=x0;
	}
	d->m_is_moving=false;
	setCursor(Qt::ArrowCursor);
}

void SSAbstractView::mouseReleaseEvent(QMouseEvent *evt) {
	Vec2 SR=selectionRange();
	double selected_xmin=SR.x;
	double selected_xmax=SR.y;
	Q_UNUSED(selected_xmax)

	QPoint pos=evt->pos()-plot()->pos();
	Vec2 coord = plot()->pixToCoord(vec2(pos.x(), pos.y()));
	double x0 = (int) coord.x;
	if (!d->m_is_moving) {
		if (selected_xmin<0) {
			setCurrentX(x0);
			//int channel=plot()->pixToChannel(vec2(pos.x(), pos.y()));
			//setCurrentChannel(channel);
		}
	} else {
		d->m_is_moving=false;
		setCursor(Qt::ArrowCursor);
	}
	d->m_selected_x_anchor=-1;
}

void SSAbstractView::mouseMoveEvent(QMouseEvent *evt) {
	QPoint pos=evt->pos()-plot()->pos();
	Vec2 coord = plot()->pixToCoord(vec2(pos.x(), pos.y()));
	double x0 = (int) coord.x;

	if (evt->buttons()&Qt::LeftButton) {
		if ((d->m_click_anchor>=0)&&(qAbs(evt->pos().x()-d->m_click_anchor_pix.x())>=15)) { //don't be so sensitive!
            d->do_translate((int)(d->m_click_anchor-x0));
            d->m_is_moving=true;
			setCursor(Qt::OpenHandCursor);
		}
	} else if (evt->buttons()&Qt::RightButton) {
		if (d->m_selected_x_anchor >= 0) {
			double x1=qMin(d->m_selected_x_anchor, qMax(x0,0.0));
			double x2=qMax(d->m_selected_x_anchor, qMax(x0,0.0));
			this->setSelectionRange(vec2(x1,x2));
		}
	}
}

void SSAbstractView::keyPressEvent(QKeyEvent *evt) {
	if (evt->key()==Qt::Key_Equal) {
		d->zoom_in();
	} else if (evt->key()==Qt::Key_Return) {
		d->zoom_in();
	} else if (evt->key()==Qt::Key_Minus) {
		d->zoom_out();
	} else if (evt->key()==Qt::Key_0) {
		setXRange(vec2(0,d->m_max_timepoint));
	} else if (evt->key()==Qt::Key_Left) {
		if ((evt->modifiers()&Qt::ControlModifier)||(evt->modifiers()&Qt::ShiftModifier)) {
			float numpix=0.5; if (evt->modifiers()&Qt::ShiftModifier) numpix=10;
			double x0=currentX();
			if (x0<0) return;
			int diff=plot()->pixToCoord(vec2(numpix,0)).x-plot()->pixToCoord(vec2(0,0)).x;
			x0-=qMax(1,diff);
			if (x0<0) return;
			setCurrentX(x0);
		}
		else {
			d->on_left_right_arrow(true);
		}
	} else if (evt->key()==Qt::Key_Right) {
		if ((evt->modifiers()&Qt::ControlModifier)||(evt->modifiers()&Qt::ShiftModifier)) {
			float numpix=0.5; if (evt->modifiers()&Qt::ShiftModifier) numpix=10;
			double x0=currentX();
			if (x0<0) return;
			int diff=plot()->pixToCoord(vec2(numpix,0)).x-plot()->pixToCoord(vec2(0,0)).x;
			x0+=qMax(1,diff);
			if (x0<0) return;
			setCurrentX(x0);
		}
		else {
			d->on_left_right_arrow(false);
		}
	}
	else if (evt->key()==Qt::Key_Up) {
		plot()->setVerticalZoomFactor(plot()->verticalZoomFactor()*1.2);
	}
	else if (evt->key()==Qt::Key_Down) {
		plot()->setVerticalZoomFactor(plot()->verticalZoomFactor()/1.2);
	}
	else if (evt->key()==Qt::Key_Backspace) {
		plot()->setVerticalZoomFactor(1);
	}
	else if (evt->matches(QKeySequence::MoveToStartOfLine)) {
		setCurrentX(0);
	} else if (evt->matches(QKeySequence::MoveToEndOfLine)) {
		setCurrentX(d->m_max_timepoint);
	}
	else if (evt->key()==Qt::Key_F) {  //ahb
	  plot()->setChannelFlip(!plot()->channelFlip()); // toggle channelFlip
	}
	else if ((evt->key()==Qt::Key_W)&&(evt->modifiers()&Qt::ControlModifier)) {
		this->topLevelWidget()->close();
	}
	else if (evt->key()==Qt::Key_Space) {
		d->scroll_to_current_x_if_needed(true/*force_center*/);
	}
}
void SSAbstractView::wheelEvent(QWheelEvent *evt) {
	int delta=evt->delta();
	if (!(evt->modifiers()&Qt::ControlModifier)) {
		if (delta<0) {
			d->zoom_out();
		} else if (delta>0) {
			d->zoom_in();
		}
	}
	else {
		float frac=1;
		if (delta<0) frac=1/0.8;
		else if (delta>0) frac=0.8;
		Vec2 coord=this->plot()->pixToCoord(vec2(evt->pos().x(),evt->pos().y()));
		d->do_zoom(coord.x,frac);
	}
}

void SSAbstractView::setMaxTimepoint(int t)
{
	d->m_max_timepoint=t;
}

void SSAbstractView::on_x_range_changed()
{
	if (d->m_signals_enabled) {
		emit xRangeChanged();
	}
}

void SSAbstractView::on_replot_needed()
{
	if (d->m_signals_enabled) {
		emit replotNeeded();
	}
}

struct TickLocations {
	QList<double> major_locations;
	QList<QString> major_labels;
	QList<double> minor_locations;
	QList<QString> minor_labels;
};

QString format_seconds(double x,bool leading_zero=true) {

	QString zz="";
	if (leading_zero) zz="0";

	if (x==0) return zz+"0";

	if (x>=60) {
		return QString("%1").arg(x,0,'f',3);
	}

	double ms_part=x-(int)x;

	if (x<10) {
		if (ms_part==0) return QString("%1%2").arg(zz).arg(x);
		else return QString("%1%2").arg(zz).arg(x,0,'f',3);
	} else {
		if (ms_part==0) return QString("%1").arg(x);
		else return QString("%1").arg(x,0,'f',3);
	}
}

QString make_time_tick_label(double val) { //ms

	if (val<1000) {
		return QString("%1 ms").arg(val);
	} else if (val<60*1000) {
		int n_sec=(int)(val/1000);
		double n_ms=val-n_sec*1000;
		if (n_ms==0) return QString("%1 s").arg(format_seconds(n_sec,false));
		else return QString("%1 s").arg(format_seconds(val/1000,false));
	} else if (val<60*60*1000) {
		int n_min=(int)(val/(60*1000));
		int n_sec=(int)((val-n_min*60*1000)/1000);
		double n_ms=val-n_min*60*1000-n_sec*1000;
		if (n_ms==0) {
			if (n_sec==0) {
				return QString("%1 m").arg(n_min);
			} else {
				if (n_min==0) {
					return QString("%1 s").arg(format_seconds(n_sec,false));
				} else {
					return QString("%1:%2").arg(n_min).arg(format_seconds(n_sec));
				}
			}
		} else {
			if (n_min==0) {
				if (n_sec==0) {
					return QString("%1 ms").arg(n_ms);
				} else {
					return QString("%1 s").arg(format_seconds(val/1000,false));
				}
			} else {
				return QString("%1:%2").arg(n_min).arg(format_seconds(n_sec+n_ms/1000));
			}
		}
	} else {
		int n_hr=(int)(val/(60*60*1000));
		double hold=val-n_hr*60*60*1000;
		if (hold==0) {
			if (n_hr==1) return QString("1 hour");
			else return QString("%1 hours").arg(n_hr);
		}
		QString tmp=make_time_tick_label(hold);
		return QString("%1h %2").arg(n_hr).arg(tmp);
	}

	return ".";
}

TickLocations get_time_tick_locations(double x1,double x2) {
	TickLocations L;

	double diff=x2-x1;

	QString units="milliseconds";
	double factor=1;
	if (diff>=2000) {
		units="seconds";
		diff/=1000;
		factor*=1000;
		if (diff>=60*2) {
			units="minutes";
			diff/=60;
			factor*=60;
			if (diff>=60*2) {
				units="hours";
				diff/=60;
				factor*=60;
			}
		}
	}
	x1/=factor;
	x2/=factor;

	double interval=1;
	double minor_interval=1;
	if (diff<0.08) {
		interval=0.01;
		minor_interval=0.01;
	} else if (diff<0.15) {
		interval=0.02;
		minor_interval=0.01;
	} else if (diff<0.3) {
		interval=0.05;
		minor_interval=0.01;
	} else if (diff<0.8) {
		interval=0.1;
		minor_interval=0.02;
	} else if (diff<1.5) {
		interval=0.2;
		minor_interval=0.1;
	} else if (diff<3) {
		interval=0.5;
		minor_interval=0.1;
	} else if (diff<8) {
		interval=1;
		minor_interval=0.2;
	} else if (diff<15) {
		interval=2;
		minor_interval=0.5;
	} else if (diff<20) {
		interval=3;
		minor_interval=0.5;
	} else if (diff<25) {
		interval=4;
		minor_interval=1;
	} else if (diff<40) {
		interval=5;
		minor_interval=1;
	} else if (diff<80) {
		interval=10;
		minor_interval=2;
	} else if (diff<150) {
		interval=20;
		minor_interval=5;
	} else if (diff<200) {
		interval=30;
		minor_interval=10;
	} else if (diff<250) {
		interval=40;
		minor_interval=10;
	} else if (diff<500) {
		interval=50;
		minor_interval=10;
	} else if (diff<800) {
		interval=100;
		minor_interval=20;
	} else if (diff<1500) {
		interval=200;
		minor_interval=50;
	} else if (diff<2000) {
		interval=300;
		minor_interval=50;
	} else {
		interval=1000;
		minor_interval=200;
	}

	double loc1=((int)(x1/interval))*interval;
	if (loc1<x1) loc1+=interval;
	for (double loc0=loc1; loc0<=x2; loc0+=interval) {
		L.major_locations << loc0*factor;
		L.major_labels << make_time_tick_label(loc0*factor);
	}
	loc1=((int)(x1/minor_interval))*minor_interval;
	if (loc1<x1) loc1+=minor_interval;
	for (double loc0=loc1; loc0<=x2; loc0+=minor_interval) {
		if (L.major_locations.indexOf(loc0)<0) {
			L.minor_locations << loc0*factor;
			L.minor_labels << "";
		}
	}
	return L;
}

void SSAbstractViewUnderlayPainter::draw_time_axis(QPainter *painter) {
	Vec2 range=q->plot()->xRange();
	double x1=range.x;
	double x2=range.y;

	//now convert to milliseconds
    double samplerate=d->m_samplerate;
    if (samplerate) {
        x1*=1000/samplerate;
        x2*=1000/samplerate;
	}
	else {
		x1=x2=0;
	}

	TickLocations locations=get_time_tick_locations(x1,x2);

	Vec2 pix1=q->plot()->coordToPix(vec2(range.x,0));
	Vec2 pix2=q->plot()->coordToPix(vec2(range.y,0));

	int y0=q->plot()->height()-20;
	QColor col(255,80,80);
	painter->setPen(QPen(QBrush(col),2));
	painter->drawLine(pix1.x,y0,pix2.x,y0);

	for (int aa=1; aa<=2; aa++) { //minor/major
		QList<double> locations0=locations.major_locations;
		QList<QString> labels0=locations.major_labels;
		int ticklen=5;
		if (aa==2) {
			locations0=locations.minor_locations;
			labels0=locations.minor_labels;
			ticklen=2;
		}
		for (int i=0; i<locations0.count(); i++) {
			double xloc=0;
            if (samplerate) {
                xloc=locations0[i]*samplerate/1000; //in timepoints
			}
			QString label=labels0[i];

			Vec2 pix3=q->plot()->coordToPix(vec2(xloc,0));
			painter->drawLine(pix3.x,y0,pix3.x,y0+ticklen);
			if ((pix3.x>20)&&(pix3.x<q->plot()->width()-20)) {
				QRect rect(pix3.x-50,y0+5,100,15);
				painter->setFont(QFont("Arial",9));
				painter->drawText(rect, Qt::AlignCenter, label);
			}
		}
	}

	//highlight current channel
	if (d->m_current_channel>=0) {
		//not sure how to do this yet
	}
}
void SSAbstractView::enableSignals() {
	d->m_signals_enabled=true;
}

void SSAbstractView::disableSignals() {
	d->m_signals_enabled=false;
}

void SSAbstractView::setTimelineVisible(bool val)
{
	if (val==d->m_timeline_visible) return;
	d->m_timeline_visible=val;
	plot()->update();
}

void SSAbstractView::setVerticalZoomFactor(float val)
{
	plot()->setVerticalZoomFactor(val);
}

float SSAbstractView::verticalZoomFactor()
{
	return plot()->verticalZoomFactor();
}

void SSAbstractView::setActivated(bool val)
{
	d->m_activated=val;
	plot()->update();
}

void SSAbstractView::centerOnCursor()
{
	d->scroll_to_current_x_if_needed(true);
}

double SSAbstractView::currentValue()
{
	return 0;
}

QString SSAbstractView::getTimeLabelForX(double x) {
    if (!d->m_samplerate) return "";
    return make_time_tick_label(getTimepointForX(x)*1000/d->m_samplerate);
}

long SSAbstractView::getTimepointForX(int x)
{
	if (d->m_timepoint_mapping.totalSize()>1) {
		if ((0<=x)&&(x<d->m_timepoint_mapping.totalSize())) {
			return d->m_timepoint_mapping.value(x);
		}
		else return -1;
	}
	else {
		return x;
	}
}
int SSAbstractView::currentChannel() {
	return d->m_current_channel;
}

void SSAbstractView::setCurrentChannel(int channel) {
	if (d->m_current_channel==channel) return;
	d->m_current_channel=channel;
	plot()->update();
	if (d->m_signals_enabled) emit currentChannelChanged();
}



void SSAbstractView::initialize()
{
	plot()->setUnderlayPainter(d->m_underlay_painter);
	plot()->initialize();
	connect(plot(),SIGNAL(xRangeChanged()),this,SLOT(on_x_range_changed()));
	connect(plot(),SIGNAL(replotNeeded()),this,SLOT(on_replot_needed()));
}

void SSAbstractView::setTimepointMapping(const DiskReadMda &X)
{
	d->m_timepoint_mapping=X;
	//can't reshape as of 3/15/2016
	//if (d->m_timepoint_mapping.totalSize()>1)
	//	d->m_timepoint_mapping.reshape(d->m_timepoint_mapping.size(0),d->m_timepoint_mapping.size(1)*d->m_timepoint_mapping.size(2));
	plot()->setTimepointMapping(X);
}

void SSAbstractView::setSamplingFrequency(float val)
{
    d->m_samplerate=val;
}

QString SSAbstractView::title()
{
	return d->m_title;
}

Vec2 SSAbstractView::xRange() {
	return plot()->xRange();
}
void SSAbstractView::setXRange(const Vec2 &range) {
	plot()->setXRange(range);
}
Vec2 SSAbstractView::selectionRange() const {
	return vec2(d->m_selected_xmin,d->m_selected_xmax);
}
void SSAbstractView::setSelectionRange(const Vec2 &range) {
	if ((d->m_selected_xmin==range.x)&&(d->m_selected_xmax==range.y)) return;
	d->m_selected_xmin=range.x;
	d->m_selected_xmax=range.y;
	if (range.x>=0) {
		int x0=(int)((range.x+range.y)/2);
		if (x0!=d->m_current_x) {
			d->m_current_x=x0;
			if (d->m_signals_enabled) emit currentXChanged();
		}
	}
	if (d->m_signals_enabled) emit selectionRangeChanged();
	plot()->update();
}

void SSAbstractView::setCurrentX(double x) {
	if (d->m_current_x==x) return;
	d->m_current_x=x;
	plot()->update();
	if (d->m_signals_enabled)
		emit currentXChanged();

	d->scroll_to_current_x_if_needed();
}
double SSAbstractView::currentX() {
	return d->m_current_x;
}

void SSAbstractView::setCurrentTimepoint(int tt)
{
	int x0=tt;
	if ((d->m_timepoint_mapping.totalSize()>1)&&(x0>=0)) {
		for (int i=0; i<d->m_timepoint_mapping.totalSize(); i++) {
			if (d->m_timepoint_mapping.value(i)==x0) {
				x0=i;
				break;
			}
		}
	}
	setCurrentX(x0);
}

int SSAbstractView::currentTimepoint()
{
	return getTimepointForX(d->m_current_x);
}

void SSAbstractViewPrivate::translate_to(int x_left) {
    int x_right=q->xRange().y-q->xRange().x+x_left;
    if ((x_left >= 0) && (x_right <= m_max_timepoint)) {
        q->setXRange(vec2(x_left, x_right));
    }
}

void SSAbstractViewPrivate::do_translate(int dx) {
	int x_left = (int) q->xRange().x;
	int x_right = (int) q->xRange().y;
	x_left += dx;
	x_right += dx;
	if ((x_left >= 0) && (x_right <= m_max_timepoint)) {
		q->setXRange(vec2(x_left, x_right));
	}
}


void SSAbstractViewPrivate::zoom_in() {
	if (m_selected_xmin<0) {
		do_zoom(m_current_x, 0.8);
	} else {
		do_zoom2(m_selected_xmin,m_selected_xmax);
		q->setSelectionRange(vec2(-1,-1));
	}
}

void SSAbstractViewPrivate::zoom_out() {
	do_zoom(m_current_x, 1/0.8);
}

void SSAbstractViewPrivate::do_zoom(double center_x, double frac) {
	int xmin = (int) q->plot()->xRange().x;
	int xmax = (int) q->plot()->xRange().y;
	int diff = xmax - xmin;
	int new_diff = (int) (diff * frac);
	if ((new_diff==diff)&&(frac>1)) new_diff+=5;
	if ((new_diff==diff)&&(frac<1)) new_diff-=5;
	if ((new_diff<8)&&(new_diff<diff)) return;
	int x0 = (int) center_x;
	if (x0 < 0) {
		x0 = (xmax + xmin) / 2;
	}

	//we need to make sure that the hovered point stays in the same place
	//right now, the x0 is in the center. we need to shift it over
	Vec2 pt_left = q->plot()->coordToPix(vec2(xmin, 0));
	Vec2 pt_right = q->plot()->coordToPix(vec2(xmax, 0));
	Vec2 pt_hover = q->plot()->coordToPix(vec2(x0, 0));

	if (pt_right.x <= pt_left.x) {
		return;
	}
	double pct_pt_hover = (pt_hover.x - pt_left.x) / (pt_right.x - pt_left.x);
	//x_left = x0-pct_pt_hover*new_diff
	int x_left = (int) (x0 - pct_pt_hover * new_diff);
	int x_right = x_left + new_diff;

	if (x_left < 0) {
		x_left = 0;
		x_right = x_left + new_diff;
	}
	if (x_right > m_max_timepoint) {
		x_right = m_max_timepoint;
		x_left = x_right - new_diff;
		if (x_left < 0) {
			x_left = 0;
		}
	}
	if ((x_right-x_left<=10)&&(frac<1)) return; //don't zoom too close
	q->setXRange(vec2(x_left, x_right));
}
void SSAbstractViewPrivate::do_zoom2(double xmin,double xmax) {
	q->setXRange(vec2(xmin,xmax));
}



void SSAbstractViewPrivate::scroll_to_current_x_if_needed(bool force_center) {
	double x0=q->currentX();
	if (x0<0) return;
	Vec2 XR=q->xRange();
	double dx=0;
	if (force_center) {
		dx=x0-(XR.x+XR.y)/2;
	}
	else {
		if ((XR.x<=x0)&&(x0<=XR.y)) return;
		if (x0<XR.x) {
			float dist=qAbs(x0-XR.x);
			if (dist>1) {
				dx=x0-(XR.x+XR.y)/2;
			}
			else {
				dx=x0-XR.x;
			}
		} else if (x0>XR.y) {
			float dist=qAbs(x0-XR.y);
			if (dist>1) {
				dx=x0-(XR.x+XR.y)/2;
			}
			else {
				dx=x0-XR.y;
			}
		}
	}
	XR.x+=dx;
	XR.y+=dx;
	if (XR.x<0) {XR.y-=XR.x; XR.x-=XR.x;}
	if (XR.y>m_max_timepoint) {XR.x+=m_max_timepoint-XR.y; XR.y+=m_max_timepoint-XR.y;}
	q->setXRange(XR);

}

void SSAbstractViewPrivate::on_left_right_arrow(bool left)
{
	Vec2 range=q->plot()->xRange();
	int dx=range.y-range.x;
	if (left) dx=-dx;
	if (range.x+dx<0) dx=-range.x;
	if (range.y+dx>m_max_timepoint) dx=m_max_timepoint-range.y;
	range.x+=dx;
	range.y+=dx;
	q->plot()->setXRange(range);
	/*
	int numpix=10;
	Vec2 coord1 = q->plot()->pixToCoord(vec2(0, 0));
	Vec2 coord2 = q->plot()->pixToCoord(vec2(numpix, 0)); //move numpix pixels
	int tmp=1;
	if (left) tmp=-1;
	do_translate((coord2.x-coord1.x)*tmp);
	*/
}

