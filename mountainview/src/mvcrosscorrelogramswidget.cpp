#include "mvcrosscorrelogramswidget.h"
#include <QApplication>
#include <QGridLayout>
#include <QProgressDialog>
#include "diskreadmda.h"
#include <math.h>
#include "histogramview.h"
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>

class MVCrossCorrelogramsWidgetPrivate {
public:
	MVCrossCorrelogramsWidget *q;

	int m_base_unit_num;
	int m_current_unit_num;
	QSet<int> m_selected_unit_nums;
	QList<int> m_unit_numbers;
	DiskReadMda m_data;

	QGridLayout *m_grid_layout;
	QList<HistogramView *> m_histogram_views;
	int m_num_columns;

	QList<QWidget *> m_child_widgets;
    QStringList m_labels;
	QMap<QString,QColor> m_colors;

	void do_highlighting();
};

MVCrossCorrelogramsWidget::MVCrossCorrelogramsWidget()
{
	d=new MVCrossCorrelogramsWidgetPrivate;
	d->q=this;
	d->m_current_unit_num=0;
	d->m_base_unit_num=0;
	d->m_unit_numbers.clear();
	d->m_num_columns=-1;

	QGridLayout *GL=new QGridLayout;
	GL->setHorizontalSpacing(20); GL->setVerticalSpacing(0);
	GL->setMargin(0);
	this->setLayout(GL);
	d->m_grid_layout=GL;

	d->m_colors["background"]=QColor(240,240,240);
	d->m_colors["frame1"]=QColor(245,245,245);
	d->m_colors["info_text"]=QColor(80,80,80);
	d->m_colors["view_background"]=QColor(245,245,245);
	d->m_colors["view_background_highlighted"]=QColor(250,220,200);
	d->m_colors["view_background_selected"]=QColor(250,240,230);
	d->m_colors["view_background_hovered"]=QColor(240,245,240);

	this->setFocusPolicy(Qt::StrongFocus);
}

MVCrossCorrelogramsWidget::~MVCrossCorrelogramsWidget()
{
	delete d;
}

void MVCrossCorrelogramsWidget::setCrossCorrelogramsPath(const QString &path)
{
	d->m_data.setPath(path);
}

void MVCrossCorrelogramsWidget::setCrossCorrelogramsData(const DiskReadMda &X)
{
    d->m_data=X;
}

void MVCrossCorrelogramsWidget::setLabels(const QStringList &labels)
{
	d->m_labels=labels;
}

void MVCrossCorrelogramsWidget::setColors(const QMap<QString, QColor> &colors)
{
	d->m_colors=colors;
	foreach (HistogramView *V,d->m_histogram_views) {
		V->setColors(d->m_colors);
	}
}

typedef QList<float> FloatList;

QList<FloatList> get_cross_correlogram_datas_2(DiskReadMda &X,int k) {
	int K=0;
	for (int i=0; i<X.N2(); i++) {
		int k1=(int)X.value(0,i);
		int k2=(int)X.value(1,i);
		if (k1>K) K=k1;
		if (k2>K) K=k2;
	}

	QList<FloatList> ret;
	for (int i=0; i<=K; i++) {
		ret << FloatList();
	}
	for (int i=0; i<X.N2(); i++) {
		int k1=(int)X.value(0,i);
		int k2=(int)X.value(1,i);
		if ((k1==k)||((k==0)&&(k1==k2))) {
			if ((1<=k2)&&(k2<=K)) {
				ret[k2] << X.value(2,i);
			}
		}
	}
	return ret;
}

QList<FloatList> get_cross_correlogram_datas_3(DiskReadMda &X,const QList<int> &unit_numbers) {

	QSet<int> the_set=QSet<int>::fromList(unit_numbers);

	QList<FloatList> ret;
	ret << FloatList();
	for (int i1=0; i1<unit_numbers.count(); i1++) {
		for (int i2=0; i2<unit_numbers.count(); i2++) {
			ret << FloatList();
		}
	}
	for (int i=0; i<X.N2(); i++) {
		int k1=(int)X.value(0,i);
		int k2=(int)X.value(1,i);
		if (the_set.contains(k1)&&(the_set.contains(k2))) {
			int ind1=unit_numbers.indexOf(k1);
			int ind2=unit_numbers.indexOf(k2);
			ret[1+(ind2+unit_numbers.count()*ind1)] << X.value(2,i);
		}
	}

	return ret;
}

class TimeScaleWidget : public QWidget {
public:
	TimeScaleWidget();
	int m_time_width;
protected:
	void paintEvent(QPaintEvent *evt);
};

TimeScaleWidget::TimeScaleWidget()
{
	setFixedHeight(25);
	m_time_width=0;
}

void TimeScaleWidget::paintEvent(QPaintEvent *evt)
{
	Q_UNUSED(evt)
	QPainter painter(this);
	QPen pen=painter.pen();
	pen.setColor(Qt::black);
	painter.setPen(pen);

	int W0=width();
	//int H0=height();
	int H1=8;
	int margin1=6;
	int len1=6;
	QPointF pt1(0+margin1,H1);
	QPointF pt2(W0-margin1,H1);
	QPointF pt3(0+margin1,H1-len1);
	QPointF pt4(W0-margin1,H1-len1);
	painter.drawLine(pt1,pt2);
	painter.drawLine(pt1,pt3);
	painter.drawLine(pt2,pt4);

	QFont font=painter.font();
	font.setPixelSize(12);
	painter.setFont(font);
	QRect text_box(0,H1+3,W0,H1+3);
	QString txt=QString("%1 ms").arg((int)(m_time_width+0.5));
	painter.drawText(text_box,txt,Qt::AlignCenter|Qt::AlignTop);
}

float compute_max(const QList<FloatList> &data0) {
	float ret=0;
	for (int i=0; i<data0.count(); i++) {
		QList<float> tmp=data0[i];
		for (int j=0; j<tmp.count(); j++) {
			if (tmp[j]>ret) ret=tmp[j];
		}
	}
	return ret;
}

void MVCrossCorrelogramsWidget::updateWidget()
{
	QGridLayout *GL=d->m_grid_layout;
	qDeleteAll(d->m_histogram_views);
	d->m_histogram_views.clear();

	qDeleteAll(d->m_child_widgets);
	d->m_child_widgets.clear();

	QProgressDialog dlg;
	dlg.show();
	dlg.setLabelText("Loading cross correlograms...");
	dlg.repaint(); qApp->processEvents();
	QList<FloatList> data0;
	if (d->m_unit_numbers.isEmpty()) {
		data0=get_cross_correlogram_datas_2(d->m_data,d->m_base_unit_num);
	}
	else {
		data0=get_cross_correlogram_datas_3(d->m_data,d->m_unit_numbers);
	}

	int K=data0.count()-1;
	int num_rows=(int)sqrt(K); if (num_rows<1) num_rows=1;
	int num_cols=(K+num_rows-1)/num_rows;
	d->m_num_columns=num_cols;

	float sample_freq=30000; //FIX: this is hard-coded!!!
	float bin_max=compute_max(data0);
	float bin_min=-bin_max;
	//int num_bins=100;
	int bin_size=20;
	int num_bins=(bin_max-bin_min)/bin_size;
	if (num_bins<100) num_bins=100;
	if (num_bins>2000) num_bins=2000;
	float time_width=(bin_max-bin_min)/sample_freq*1000;

	for (int k1=1; k1<=K; k1++) {
		HistogramView *HV=new HistogramView;
		HV->setData(data0[k1]);
		HV->setColors(d->m_colors);
		//HV->autoSetBins(50);
		HV->setBins(bin_min,bin_max,num_bins);
		int k2=k1; if (d->m_base_unit_num>=1) k2=d->m_base_unit_num;
		QString title0;
        if (!d->m_labels.isEmpty()) {
            title0=d->m_labels.value(k1);
        }
        else {
            if (!d->m_unit_numbers.isEmpty()) {
                title0=QString("%1/%2").arg(d->m_unit_numbers.value((k1-1)/num_cols)).arg(d->m_unit_numbers.value((k1-1)%num_cols));
            }
            else {
                title0=QString("%1/%2").arg(k1).arg(k2);
            }
        }
		HV->setTitle(title0);
		GL->addWidget(HV,(k1-1)/num_cols,(k1-1)%num_cols);
		if (d->m_unit_numbers.isEmpty()) {
			HV->setProperty("unit_number",k1);
		}
		connect(HV,SIGNAL(control_clicked()),this,SLOT(slot_histogram_view_control_clicked()));
		connect(HV,SIGNAL(clicked()),this,SLOT(slot_histogram_view_clicked()));
		connect(HV,SIGNAL(activated()),this,SLOT(slot_histogram_view_activated()));
		d->m_histogram_views << HV;
	}

	TimeScaleWidget *TSW=new TimeScaleWidget;
	TSW->m_time_width=time_width;
	GL->addWidget(TSW,num_rows,0);

	d->m_child_widgets << TSW;

}

int MVCrossCorrelogramsWidget::currentUnit()
{
	return d->m_current_unit_num;
}

QList<int> MVCrossCorrelogramsWidget::selectedUnits()
{
	QList<int> ret=d->m_selected_unit_nums.toList();
	qSort(ret);
	return ret;
}

void MVCrossCorrelogramsWidget::setCurrentUnit(int num)
{
	if (d->m_current_unit_num==num) return;
	if (num>d->m_histogram_views.count()) return;

	d->m_current_unit_num=num;
	d->do_highlighting();
	emit currentUnitChanged();
}

bool sets_match(const QSet<int> &S1,const QSet<int> &S2) {
	foreach (int a,S1) if (!S2.contains(a)) return false;
	foreach (int a,S2) if (!S1.contains(a)) return false;
	return true;
}

void MVCrossCorrelogramsWidget::setSelectedUnits(const QList<int> &nums)
{
	if (sets_match(nums.toSet(),d->m_selected_unit_nums)) return;
	d->m_selected_unit_nums=QSet<int>::fromList(nums);
	d->do_highlighting();
	emit selectedUnitsChanged();
}

int MVCrossCorrelogramsWidget::baseUnit()
{
	return d->m_base_unit_num;
}

void MVCrossCorrelogramsWidget::setBaseUnit(int num)
{
	d->m_base_unit_num=num;
}

void MVCrossCorrelogramsWidget::setUnitNumbers(const QList<int> &numbers)
{
	d->m_unit_numbers=numbers;
}

void MVCrossCorrelogramsWidget::slot_histogram_view_clicked()
{
	int num=sender()->property("unit_number").toInt();
	d->m_selected_unit_nums.clear();
	if (d->m_current_unit_num==num) {
	}
	else {
		setCurrentUnit(num);
		d->m_selected_unit_nums.clear();
		d->m_selected_unit_nums << num;
		d->do_highlighting();
		emit selectedUnitsChanged();
		update();
	}
	emit selectedUnitsChanged();
}

void MVCrossCorrelogramsWidget::slot_histogram_view_control_clicked()
{
	int num=sender()->property("unit_number").toInt();
    if (d->m_current_unit_num==num) {
        setCurrentUnit(-1);
    }
	if (!d->m_selected_unit_nums.contains(num)) {
		d->m_selected_unit_nums << num;
		d->do_highlighting();
		if (d->m_current_unit_num<=0) setCurrentUnit(num);
	}
	else {
		d->m_selected_unit_nums.remove(num);
		d->do_highlighting();
	}
	emit selectedUnitsChanged();
}

void MVCrossCorrelogramsWidget::slot_histogram_view_activated()
{
	emit unitActivated(currentUnit());
}

void MVCrossCorrelogramsWidget::keyPressEvent(QKeyEvent *evt)
{
	if (evt->key()==Qt::Key_Left) {
		setCurrentUnit(this->currentUnit()-1);
	}
	else if (evt->key()==Qt::Key_Right) {
		setCurrentUnit(this->currentUnit()+1);
	}
	else if (evt->key()==Qt::Key_Up) {
		setCurrentUnit(this->currentUnit()-d->m_num_columns);
	}
	else if (evt->key()==Qt::Key_Down) {
		setCurrentUnit(this->currentUnit()+d->m_num_columns);
	}
}


void MVCrossCorrelogramsWidgetPrivate::do_highlighting()
{
	for (int i=0; i<m_histogram_views.count(); i++) {
		HistogramView *HV=m_histogram_views[i];
		int k=HV->property("unit_number").toInt();
		if (k==m_current_unit_num) {
			HV->setCurrent(true);
		}
		else {
			HV->setCurrent(false);
		}
		if (m_selected_unit_nums.contains(k)) {
			HV->setSelected(true);
		}
		else {
			HV->setSelected(false);
		}
	}
}


