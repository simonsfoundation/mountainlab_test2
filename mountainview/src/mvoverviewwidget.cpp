#include "mvoverviewwidget.h"
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QProgressDialog>
#include <QPushButton>
#include "sstimeserieswidget.h"
#include "sstimeseriesview.h"
#include "diskreadmda.h"
#include <QList>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextBrowser>
#include "firetrackwidget.h"
#include "histogramview.h"
#include <math.h>
#include "mvstatisticswidget.h"
#include "mvcrosscorrelogramswidget.h"
#include <QSplitter>
#include "mvunitwidget.h"
#include "mvneuroncomparisonwidget.h"
#include "diskarraymodelclipssubset.h"
#include "ftelectrodearrayview.h"
#include "mvcdfview.h"

class MVOverviewWidgetPrivate {
public:
	MVOverviewWidget *q;

	Mda m_primary_channels;
	Mda m_templates;
	Mda m_locations;
	DiskArrayModel *m_raw;
	bool m_own_raw;
	Mda m_times;
	Mda m_labels;
	int m_template_view_padding;
	QString m_cross_correlograms_path;
	int m_current_unit_number;

	DiskArrayModel *m_clips;
	bool m_own_clips;
	Mda m_clips_index;

	SSTimeSeriesView *m_spike_templates_view;
	MVStatisticsWidget *m_statistics_widget;
	MVCrossCorrelogramsWidget *m_cross_correlograms_widget;
    SSTimeSeriesView *m_labeled_raw_view;
	FTElectrodeArrayView *m_electrode_view;
	MVCdfView *m_cdf_view;

	QSplitter *m_hsplitter;
	QSplitter *m_vsplitter;
	QSplitter *m_left_vsplitter;

	void update_spike_templates();
	void update_sizes();
	void set_current_unit(int num);
	void do_compare_units(const QList<int> &unit_numbers);
};


MVOverviewWidget::MVOverviewWidget(QWidget *parent) : QWidget(parent)
{
	d=new MVOverviewWidgetPrivate;
	d->q=this;

	d->m_template_view_padding=30;

	d->m_raw=0;
	d->m_own_raw=false;
	d->m_clips=0;
	d->m_own_clips=false;

	d->m_spike_templates_view=new SSTimeSeriesView;
	d->m_spike_templates_view->initialize();
	connect(d->m_spike_templates_view,SIGNAL(currentXChanged()),this,SLOT(slot_spike_templates_current_x_changed()));

    d->m_labeled_raw_view=new SSTimeSeriesView;
    d->m_labeled_raw_view->initialize();
	connect(d->m_labeled_raw_view,SIGNAL(currentXChanged()),this,SLOT(slot_current_raw_timepoint_changed()));
    SSTimeSeriesWidget *labeled_raw_widget=new SSTimeSeriesWidget;
    labeled_raw_widget->addView(d->m_labeled_raw_view);
    labeled_raw_widget->hideMenu();

	d->m_statistics_widget=new MVStatisticsWidget;
	connect(d->m_statistics_widget,SIGNAL(selectedUnitsChanged()),this,SLOT(slot_statistics_widget_selected_units_changed()));
	connect(d->m_statistics_widget,SIGNAL(currentUnitChanged()),this,SLOT(slot_statistics_widget_current_unit_changed()));
	connect(d->m_statistics_widget,SIGNAL(unitActivated(int)),this,SLOT(slot_unit_activated(int)));

	d->m_cross_correlograms_widget=new MVCrossCorrelogramsWidget;
	connect(d->m_cross_correlograms_widget,SIGNAL(currentUnitChanged()),this,SLOT(slot_cross_correlograms_current_unit_changed()));
	connect(d->m_cross_correlograms_widget,SIGNAL(selectedUnitsChanged()),this,SLOT(slot_cross_correlograms_selected_units_changed()));
	connect(d->m_cross_correlograms_widget,SIGNAL(unitActivated(int)),this,SLOT(slot_unit_activated(int)));

	d->m_electrode_view=new FTElectrodeArrayView;
	d->m_electrode_view->setShowChannelNumbers(true);
	d->m_electrode_view->setNormalizeIntensity(false);

	d->m_cdf_view=new MVCdfView;
	connect(d->m_cdf_view,SIGNAL(currentLabelChanged()),this,SLOT(slot_cdf_view_current_label_changed()));
	connect(d->m_cdf_view,SIGNAL(currentTimepointChanged()),this,SLOT(slot_cdf_view_current_timepoint_changed()));

	{
		QSplitter *vsplitter=new QSplitter(Qt::Vertical);
		vsplitter->setHandleWidth(15);
		vsplitter->addWidget(d->m_spike_templates_view);
		vsplitter->addWidget(d->m_cross_correlograms_widget);
        vsplitter->addWidget(labeled_raw_widget);
		d->m_vsplitter=vsplitter;
	}
	{
		QSplitter *vsplitter=new QSplitter(Qt::Vertical);
		vsplitter->setHandleWidth(15);
		vsplitter->addWidget(d->m_statistics_widget);
		vsplitter->addWidget(d->m_cdf_view);
		vsplitter->addWidget(d->m_electrode_view);
		d->m_left_vsplitter=vsplitter;
	}

	QSplitter *hsplitter=new QSplitter;
	hsplitter->setHandleWidth(15);
	hsplitter->addWidget(d->m_left_vsplitter);
	hsplitter->addWidget(d->m_vsplitter);
	d->m_hsplitter=hsplitter;

	QMenuBar *menu_bar=new QMenuBar(this);
	{
		QMenu *menu=new QMenu("Tools");
		menu_bar->addMenu(menu);
		{
			QAction *A=new QAction("Explore Neuron",this);
			A->setShortcut(QKeySequence("Ctrl+N"));
			connect(A,SIGNAL(triggered(bool)),this,SLOT(slot_explore_neuron()));
			menu->addAction(A);
		}
		{
			QAction *A=new QAction("Compare Neurons",this);
			A->setShortcut(QKeySequence("Ctrl+C"));
			connect(A,SIGNAL(triggered(bool)),this,SLOT(slot_compare_neurons()));
			menu->addAction(A);
		}
	}
	menu_bar->setMaximumHeight(40); //otherwise we have weird sizing problems

	QVBoxLayout *vlayout=new QVBoxLayout;
	vlayout->addWidget(menu_bar);
	vlayout->addWidget(hsplitter);
	this->setLayout(vlayout);

	//this->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	//this->setWindowFlags(Qt::Window|Qt::Tool);
	//this->setWindowFlags(this->windowFlags()|Qt::CustomizeWindowHint);
	//this->setWindowFlags(this->windowFlags() ^ Qt::WindowCloseButtonHint);
}

void MVOverviewWidget::setElectrodeLocations(const Mda &L)
{
	d->m_locations=L;
	d->m_electrode_view->setElectrodeLocations(L);
}

void MVOverviewWidget::setTemplates(const Mda &X)
{
	d->m_templates=X;

	float max_abs=0;
	for (int i=0; i<X.totalSize(); i++) {
		float val=X.value1(i);
		if (val<0) val=-val;
		if (val>max_abs) max_abs=val;
	}
	d->m_electrode_view->setGlobalAbsMax(max_abs);
}

void MVOverviewWidget::setPrimaryChannels(const Mda &X)
{
	d->m_primary_channels=X;
	d->m_statistics_widget->setPrimaryChannels(X);
}

void MVOverviewWidget::setRaw(DiskArrayModel *X,bool own_it)
{
	if ((d->m_raw)&&(d->m_own_raw)) delete d->m_raw;
    for (int i=0; i<100; i++) qApp->processEvents(); //this is a disturbing hack, but if I don't use it, then sometimes the raw data appears as all zeros (after the first run where the file hierarchy is created). Yikes!!
	d->m_raw=X;
	d->m_own_raw=own_it;
	d->m_statistics_widget->setRaw(X);
	d->m_labeled_raw_view->setData(X,false);
}

void MVOverviewWidget::setTimesLabels(const Mda &times, const Mda &labels)
{
	d->m_times=times;
	d->m_labels=labels;
	int NN=d->m_times.totalSize();
	Mda TL; TL.allocate(2,NN);
	for (int ii=0; ii<NN; ii++) {
        TL.setValue(d->m_times.value1(ii),0,ii);
		TL.setValue(d->m_labels.value1(ii),1,ii);
	}

	d->m_statistics_widget->setTimesLabels(times,labels);
	d->m_labeled_raw_view->setLabels(new DiskReadMda(TL),true);

	d->m_cdf_view->setTimesLabels(times,labels);
	d->m_cdf_view->update();
}

void MVOverviewWidget::setCrossCorrelogramsPath(const QString &path)
{
	d->m_cross_correlograms_path=path;
	d->m_cross_correlograms_widget->setCrossCorrelogramsPath(path);
}

void MVOverviewWidget::setClips(DiskArrayModel *X, bool own_it)
{
	if ((d->m_clips)&&(d->m_own_clips)) delete d->m_clips;
	d->m_clips=X;
	d->m_own_clips=own_it;
}

void MVOverviewWidget::setClipsIndex(const Mda &X)
{
	d->m_clips_index=X;
}

void MVOverviewWidget::updateWidgets()
{
	d->update_spike_templates();
	d->m_statistics_widget->updateStatistics();
	d->m_cross_correlograms_widget->updateWidget();
	d->m_electrode_view->update();
}

void MVOverviewWidget::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt);

	d->update_sizes();
}

void MVOverviewWidget::slot_spike_templates_current_x_changed()
{
	int x=d->m_spike_templates_view->currentX();
	int unit_number=x/(d->m_templates.N2()+d->m_template_view_padding)+1;
	d->set_current_unit(unit_number);
}

void MVOverviewWidget::slot_cross_correlograms_current_unit_changed()
{
	int num=d->m_cross_correlograms_widget->currentUnit();
	d->set_current_unit(num);
}

void MVOverviewWidget::slot_cross_correlograms_selected_units_changed()
{
	QList<int> nums=d->m_cross_correlograms_widget->selectedUnits();
	d->m_statistics_widget->setSelectedUnits(nums);
}

void MVOverviewWidget::slot_statistics_widget_current_unit_changed()
{
	int num=d->m_statistics_widget->currentUnit();
	d->set_current_unit(num);

	QList<int> units=d->m_statistics_widget->selectedUnits();
	d->m_cross_correlograms_widget->setSelectedUnits(units);
}

void MVOverviewWidget::slot_statistics_widget_selected_units_changed()
{
	int num=d->m_statistics_widget->currentUnit();
	d->set_current_unit(num);

	QList<int> units=d->m_statistics_widget->selectedUnits();
	d->m_cross_correlograms_widget->setSelectedUnits(units);
}

void MVOverviewWidget::slot_unit_activated(int num)
{
	MVUnitWidget *W=new MVUnitWidget;
	W->setWindowTitle(QString("%1 - Neuron %2").arg(this->windowTitle()).arg(num));
	W->setUnitNumber(num);
	W->setAttribute(Qt::WA_DeleteOnClose);
	W->setElectrodeLocations(d->m_locations);
	W->setTimesLabels(d->m_times,d->m_labels);
	W->setRaw(d->m_raw,false);
	W->setTemplates(d->m_templates);

	DiskArrayModelClipsSubset *clips=new DiskArrayModelClipsSubset;
	clips->setPath(d->m_clips->path());
	int T=d->m_clips->size(1)/d->m_clips->dim3();
	int i1=(int)d->m_clips_index.value1(num-1)*T;
	int i2=(int)d->m_clips_index.value1(num)*T;
	if (num>=d->m_clips_index.totalSize()) i2=d->m_clips->size(1);
	clips->setRange(i1,i2);

	W->setClips(clips,true);
	W->setCrossCorrelogramsPath(d->m_cross_correlograms_path);
	W->resize(width(),height());
	W->show();
	W->setProperty("unit_number",num);
	W->updateWidgets();

	connect(W,SIGNAL(currentClipNumberChanged()),this,SLOT(slot_current_clip_number_changed()));
}

void MVOverviewWidget::slot_current_clip_number_changed()
{
	MVUnitWidget *W=(MVUnitWidget *)sender();
	if (!W) return;
	int unit_num=W->property("unit_number").toInt();
	int clip_num=W->currentClipNumber();
	int jj=0;
	for (int i=0; i<d->m_labels.totalSize(); i++) {
		int label0=(int)d->m_labels.value1(i);
		if (label0==unit_num) {
			if (jj==clip_num) {
				int time0=(int)d->m_times.value1(i);
				d->m_labeled_raw_view->setCurrentTimepoint(time0);
				break;
			}
			jj++;
		}
	}
}

void MVOverviewWidget::slot_cdf_view_current_label_changed()
{
	d->set_current_unit(d->m_cdf_view->currentLabel());
}

void MVOverviewWidget::slot_cdf_view_current_timepoint_changed()
{
	d->m_labeled_raw_view->setCurrentTimepoint(d->m_cdf_view->currentTimepoint());
}

void MVOverviewWidget::slot_current_raw_timepoint_changed()
{
	d->m_cdf_view->setCurrentTimepoint(d->m_labeled_raw_view->currentTimepoint());
}

void MVOverviewWidget::slot_compare_neurons()
{
	QList<int> units=d->m_cross_correlograms_widget->selectedUnits();
	qSort(units);
	if (units.count()<=1) {
		QMessageBox::information(this,"Compare Neurons","You must select more than one neuron in the auto-correlogram view. Use the Control key to select multiple neurons.");
		return;
	}
	d->do_compare_units(units);
}

void MVOverviewWidget::slot_explore_neuron()
{
	int num=d->m_cross_correlograms_widget->currentUnit();
	if (num<=0) {
		QMessageBox::information(this,"Explore Neuron","You must select a neuron in the auto-correlogram view.");
		return;
	}
	slot_unit_activated(num);
}

void MVOverviewWidgetPrivate::update_spike_templates()
{
	SSTimeSeriesView *V=m_spike_templates_view;
	DiskArrayModel *data=new DiskArrayModel;
	int M=m_templates.N1();
	int T=m_templates.N2();
	int K=m_templates.N3();
	int padding=m_template_view_padding;
	Mda templates_formatted; templates_formatted.allocate(M,(T+padding)*K);
	Mda TL; TL.allocate(2,K);
	Mda *templates=&m_templates;
	for (int k=0; k<K; k++) {
		TL.setValue((T+padding)*k+T/2,0,k);
		//TL.setValue(order[k]+1,1,k);
		TL.setValue(k+1,1,k);
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				//templates_formatted.setValue(templates->value(m,t,order[k]),m,t+(T+padding)*k);
				templates_formatted.setValue(templates->value(m,t,k),m,t+(T+padding)*k);
			}
		}
	}
	data->setFromMda(templates_formatted);
	V->plot()->setShowMarkerLines(false);
	V->setData(data,true);
	V->setVerticalZoomFactor(1.2);
	V->setLabels(new DiskReadMda(TL),true);
	V->initialize();
}

void MVOverviewWidgetPrivate::update_sizes()
{
	float W0=q->width();
	float H0=q->height();

	int W1=W0/3; if (W1<300) W1=300; if (W1>500) W1=500;
	int W2=W0-W1;
	if (W2<500) {W2=500; W1=W0-W2;}

	int H1=H0/3;
	int H2=H0/3;
	int H3=H0-H1-H2;
	{
		QList<int> sizes; sizes << W1 << W2;
		m_hsplitter->setSizes(sizes);
	}
	{
		QList<int> sizes; sizes << H1 << H2 << H3;
		m_vsplitter->setSizes(sizes);
	}

	int H1_left=H0/3;
	int H2_left=H0/3;
	int H3_left=H0-H1_left-H2_left;
	{
		QList<int> sizes; sizes << H1_left << H2_left << H3_left;
		m_left_vsplitter->setSizes(sizes);
	}
}

void MVOverviewWidgetPrivate::set_current_unit(int num)
{
	m_current_unit_number=num;
	m_cross_correlograms_widget->setCurrentUnit(num);
	m_statistics_widget->setCurrentUnit(num);

	int x=m_spike_templates_view->currentX();
	int factor=(m_templates.N2()+m_template_view_padding);
	int unit_number=x/factor+1;
	if (unit_number!=num) {
		m_spike_templates_view->setCurrentX(factor*(num-1)+m_templates.N2()/2);
	}

	int M=m_templates.N1();
	int T=m_templates.N2();
	Mda template0; template0.allocate(M,T);
	for (int t=0; t<T; t++) {
		for (int m=0; m<M; m++) {
			template0.setValue(m_templates.value(m,t,num-1),m,t);
		}
	}
	m_electrode_view->setWaveform(template0);

	m_cdf_view->setCurrentLabel(num);
}

void MVOverviewWidgetPrivate::do_compare_units(const QList<int> &unit_numbers)
{
    MVNeuronComparisonWidget *W=new MVNeuronComparisonWidget;
	QString tmp;
	foreach (int num,unit_numbers) {
		if (!tmp.isEmpty()) tmp+=", ";
		tmp+=QString("%1").arg(num);
	}
	W->setWindowTitle(QString("%1 - Neurons %2").arg(q->windowTitle()).arg(tmp));
	W->setUnitNumbers(unit_numbers);
	W->setAttribute(Qt::WA_DeleteOnClose);
	W->setElectrodeLocations(m_locations);
	W->setTimesLabels(m_times,m_labels);
	W->setRaw(m_raw,false);

	QList<DiskArrayModel *> clips;
	for (int i=0; i<unit_numbers.count(); i++) {
		int num=unit_numbers[i];
		DiskArrayModelClipsSubset *clips0=new DiskArrayModelClipsSubset;
		clips0->setPath(m_clips->path());
		int T=m_clips->size(1)/m_clips->dim3();
		int i1=(int)m_clips_index.value1(num-1)*T;
		int i2=(int)m_clips_index.value1(num)*T;
		if (num>=m_clips_index.totalSize()) i2=m_clips->size(1);
		clips0->setRange(i1,i2);
		clips << clips0;
	}

//	DiskArrayModelClipsSubset *clips=new DiskArrayModelClipsSubset;
//	clips->setPath(d->m_clips->path());
//	int T=d->m_clips->size(1)/d->m_clips->dim3();
//	int i1=(int)d->m_clips_index.value1(num-1)*T;
//	int i2=(int)d->m_clips_index.value1(num)*T;
//	if (num>=d->m_clips_index.totalSize()) i2=d->m_clips->size(1);
//	clips->setRange(i1,i2);

	W->setClips(clips,true);
	W->setCrossCorrelogramsPath(m_cross_correlograms_path);
	W->resize(q->width(),q->height());
	W->show();
	//W->setProperty("unit_number",num);
	W->updateWidgets();

	//connect(W,SIGNAL(currentClipNumberChanged()),this,SLOT(slot_current_clip_number_changed()));
}

MVOverviewWidget::~MVOverviewWidget()
{
	if ((d->m_raw)&&(d->m_own_raw)) delete d->m_raw;
	if ((d->m_clips)&&(d->m_own_clips)) delete d->m_clips;
	delete d;
}
