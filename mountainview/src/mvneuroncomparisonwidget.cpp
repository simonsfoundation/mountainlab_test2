#include "mvneuroncomparisonwidget.h"
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QProgressDialog>
#include <QPushButton>
#include "sstimeserieswidget.h"
#include "sstimeseriesview.h"
#include "diskreadmda.h"
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QTextBrowser>
#include <QTime>
#include <QTimer>
#include "firetrackwidget.h"
#include "histogramview.h"
#include <math.h>
#include "mvcrosscorrelogramswidget.h"
#include <QSplitter>
#include "cvwidget.h"
#include "get_principal_components.h"
#include "mvutils.h"

class MVNeuronComparisonWidgetPrivate {
public:
    MVNeuronComparisonWidget *q;

	Mda m_locations;
	DiskArrayModel *m_raw;
	bool m_own_raw;
	Mda m_times;
	Mda m_labels;
	QString m_cross_correlograms_path;
	int m_current_clip_number;
	Mda m_TL;

	QList<DiskArrayModel *> m_clips;
	bool m_own_clips;
	QList<int> m_unit_numbers;

	MVCrossCorrelogramsWidget *m_cross_correlograms_widget;
	SSTimeSeriesView *m_clips_view;
	SSTimeSeriesView *m_template_view;
	CVWidget *m_cluster_widget;
	SSTimeSeriesView *m_labeled_raw_view;

	QSplitter *m_hsplitter;
	QSplitter *m_vsplitter;
	QSplitter *m_left_vsplitter;
	QLabel *m_status_label;

	void update_sizes();
	void set_status_text(const QString &txt);
	void set_current_clip_number(int num);
};


MVNeuronComparisonWidget::MVNeuronComparisonWidget(QWidget *parent) : QWidget(parent)
{
    d=new MVNeuronComparisonWidgetPrivate;
	d->q=this;

	d->m_raw=0;
	d->m_own_raw=false;

	d->m_own_clips=false;
	d->m_current_clip_number=-1;

	d->m_clips_view=new SSTimeSeriesView;
	d->m_clips_view->initialize();
	connect(d->m_clips_view,SIGNAL(currentXChanged()),this,SLOT(slot_clips_view_current_x_changed()));

	d->m_template_view=new SSTimeSeriesView;
	d->m_template_view->initialize();

	d->m_cluster_widget=new CVWidget;
	d->m_cluster_widget->setNumDataPointsToSelect(1);
	connect(d->m_cluster_widget,SIGNAL(selectedDataPointsChanged()),this,SLOT(slot_selected_data_points_changed()));

	d->m_cross_correlograms_widget=new MVCrossCorrelogramsWidget;

	d->m_status_label=new QLabel;

	d->m_labeled_raw_view=new SSTimeSeriesView;
	d->m_labeled_raw_view->initialize();
	//connect(d->m_labeled_raw_view,SIGNAL(currentXChanged()),this,SLOT(slot_current_raw_timepoint_changed()));

	{
		QSplitter *vsplitter=new QSplitter(Qt::Vertical);
		vsplitter->setHandleWidth(15);
		vsplitter->addWidget(d->m_clips_view);
		vsplitter->addWidget(d->m_cross_correlograms_widget);
		vsplitter->addWidget(d->m_labeled_raw_view);
		d->m_vsplitter=vsplitter;
	}

	{
		QSplitter *vsplitter=new QSplitter(Qt::Vertical);
		vsplitter->setHandleWidth(15);
		vsplitter->addWidget(d->m_template_view);
		vsplitter->addWidget(d->m_cluster_widget);
		d->m_left_vsplitter=vsplitter;
	}

	QWidget *left_panel=new QWidget;
	QVBoxLayout *left_panel_layout=new QVBoxLayout;
	left_panel->setLayout(left_panel_layout);
	left_panel_layout->addWidget(d->m_left_vsplitter);
	left_panel_layout->addWidget(d->m_status_label);

	QSplitter *hsplitter=new QSplitter;
	hsplitter->setHandleWidth(15);
	hsplitter->addWidget(left_panel);
	hsplitter->addWidget(d->m_vsplitter);
	d->m_hsplitter=hsplitter;

	QHBoxLayout *hlayout=new QHBoxLayout;
	hlayout->addWidget(hsplitter);
	this->setLayout(hlayout);

	//this->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	//this->setWindowFlags(Qt::Window|Qt::Tool);
	//this->setWindowFlags(this->windowFlags()|Qt::CustomizeWindowHint);
	//this->setWindowFlags(this->windowFlags() ^ Qt::WindowCloseButtonHint);
}

void MVNeuronComparisonWidget::setElectrodeLocations(const Mda &L)
{
	d->m_locations=L;
}

void MVNeuronComparisonWidget::setRaw(DiskArrayModel *X,bool own_it)
{
	if ((d->m_raw)&&(d->m_own_raw)) delete d->m_raw;
	d->m_raw=X;
	d->m_own_raw=own_it;

	d->m_labeled_raw_view->setData(X,false);
}

void MVNeuronComparisonWidget::setTimesLabels(const Mda &times, const Mda &labels)
{
	QSet<int> the_set=d->m_unit_numbers.toSet();

	d->m_times=times;
	d->m_labels=labels;
//	int NN=d->m_times.totalSize();
//	Mda TL; TL.allocate(2,NN);
//	for (int ii=0; ii<NN; ii++) {
//		TL.setValue(d->m_times.value1(ii),0,ii);
//		TL.setValue(d->m_labels.value1(ii),1,ii);
//	}

//	d->m_clips_view->setLabels(new DiskReadMda(TL),true);

	int NN=d->m_times.totalSize();
	int N0=0;
	for (int ii=0; ii<NN; ii++) {
		if (the_set.contains(d->m_labels.value1(ii))) {
			N0++;
		}
	}
	Mda TL; TL.allocate(2,N0);
	int jj=0;
	for (int ii=0; ii<NN; ii++) {
		if (the_set.contains(d->m_labels.value1(ii))) {
            TL.setValue(d->m_times.value1(ii),0,jj);
			TL.setValue(d->m_labels.value1(ii),1,jj);
			jj++;
		}
	}
	d->m_TL=TL;

	d->m_labeled_raw_view->setLabels(new DiskReadMda(TL),true);
}

void MVNeuronComparisonWidget::setCrossCorrelogramsPath(const QString &path)
{
	d->m_cross_correlograms_path=path;
	d->m_cross_correlograms_widget->setCrossCorrelogramsPath(path);
}

void MVNeuronComparisonWidget::setClips(const QList<DiskArrayModel *> &C, bool own_it)
{
	if (d->m_own_clips) qDeleteAll(d->m_clips);
	d->m_clips=C;
	d->m_own_clips=own_it;

	if (C.isEmpty()) return;

	/*
	 * //the following takes too long!!
	int NC=0;
	for (int i=0; i<C.count(); i++) NC+=C[i]->dim3();
	int M=C[0]->size(0);
	int T=C[0]->size(1)/C[0]->dim3();
	Mda X; X.allocate(M,T,NC);
	int jj=0;
	for (int i=0; i<C.count(); i++) {
		for (int n=0; n<C[i]->dim3(); n++) {
			for (int t=0; t<T; t++) {
				for (int m=0; m<M; m++) {
					X.setValue1(C[i]->value(m,t+T*n),jj);
					jj++;
				}
			}
		}
	}
	DiskArrayModel *X0=new DiskArrayModel;
	X0->setFromMda(X);
	d->m_clips_view->setData(X0,true);
	int max0=10000;
	if (X0->size(1)>max0) {
		d->m_clips_view->setXRange(vec2(0,max0-1));
	}
	*/



}

void MVNeuronComparisonWidget::setUnitNumbers(const QList<int> &numbers)
{
	d->m_unit_numbers=numbers;
	d->m_cross_correlograms_widget->setUnitNumbers(numbers);
}

void MVNeuronComparisonWidget::updateWidgets()
{
	d->m_cross_correlograms_widget->updateWidget();
	QTimer::singleShot(100,this,SLOT(slot_compute_templates()));
}

void MVNeuronComparisonWidget::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt);

	d->update_sizes();
}

void MVNeuronComparisonWidget::slot_compute_templates()
{
	if (d->m_clips.isEmpty()) return;

    {
        d->set_status_text("Computing Templates...");
        Mda template0=compute_mean_waveform(d->m_clips[0]);
        Mda template1; template1.allocate(template0.N1(),template0.N2(),d->m_clips.count());
		QList<long> times0,labels0;
        int jj=0;
        for (int i=0; i<d->m_clips.count(); i++) {
            Mda tmp=compute_mean_waveform(d->m_clips[i]);
            int ii=0;
            for (int t=0; t<template0.N2(); t++) {
                for (int m=0; m<template0.N1(); m++) {
                    template1.setValue1(tmp.value1(ii),jj);
                    ii++;
                    jj++;
                }
            }
			times0 << (long)(template0.N2()*(i+0.5));
			labels0 << d->m_unit_numbers.value(i);
        }
        d->set_status_text("Ready.");
        DiskArrayModel *template_data=new DiskArrayModel;
        template_data->setFromMda(template1);
        d->m_template_view->setClipMode(true);
        d->m_template_view->setData(template_data,true);
        d->m_template_view->setTimesLabels(times0,labels0);
        d->m_template_view->setMarkerLinesVisible(false);
    }

    {
        d->set_status_text("Computing Features...");
        Mda features0=compute_features(d->m_clips);
        Mda labels0; labels0.allocate(1,features0.N2());
        QStringList label_strings; //for legend
        int ii=0;
        for (int j=0; j<d->m_clips.count(); j++) {
            for (int k=0; k<d->m_clips[j]->dim3(); k++) {
                labels0.setValue1(j+1,ii);
                ii++;
            }
            label_strings << QString("%1").arg(d->m_unit_numbers.value(j));
        }
        d->set_status_text("Ready.");
        d->m_cluster_widget->setFeatures(features0);
        d->m_cluster_widget->setLabels(labels0);
        d->m_cluster_widget->setLabelStrings(label_strings);
        d->m_cluster_widget->autoSetRange();
        d->m_cluster_widget->refresh();
    }
}

void MVNeuronComparisonWidget::slot_clips_view_current_x_changed()
{
	/*
	if (!d->m_clips->dim3()) return;
	//int M=d->m_clips->size(0);
	int T=d->m_clips->size(1)/d->m_clips->dim3();
	int NC=d->m_clips->dim3();
	if (!NC) return;

	int x=d->m_clips_view->currentX();
	int clip_number=x/T;
	d->set_current_clip_number(clip_number);

	d->m_labeled_raw_view->setCurrentTimepoint((int)d->m_TL.value(0,clip_number));
	*/
}

void MVNeuronComparisonWidget::slot_selected_data_points_changed()
{
	QList<int> L=d->m_cluster_widget->selectedDataPointIndices();
	if (L.count()!=1) return;
	d->set_current_clip_number(L[0]);
}



void MVNeuronComparisonWidgetPrivate::update_sizes()
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

	int H1_left=H0/2;
	int H2_left=H0/2;
	{
		QList<int> sizes; sizes << H1_left << H2_left;
		m_left_vsplitter->setSizes(sizes);
	}

}

void MVNeuronComparisonWidgetPrivate::set_status_text(const QString &txt)
{
	m_status_label->setText(txt);
}

void MVNeuronComparisonWidgetPrivate::set_current_clip_number(int num)
{
    Q_UNUSED(num)
	/*
	if (m_current_clip_number==num) return;

	m_current_clip_number=num;
	QList<int> L; L << m_current_clip_number;
	m_cluster_widget->setSelectedDataPointIndices(L);

	{
		if (!m_clips->dim3()) return;
		//int M=d->m_clips->size(0);
		int T=m_clips->size(1)/m_clips->dim3();
		int NC=m_clips->dim3();
		if (!NC) return;

		int x=m_clips_view->currentX();
		int clip_number2=x/T;
		if (clip_number2!=num) {
			m_clips_view->setCurrentX(num*T+T/2);
		}
	}

	emit q->currentClipNumberChanged();
	*/
}

MVNeuronComparisonWidget::~MVNeuronComparisonWidget()
{
	if ((d->m_raw)&&(d->m_own_raw)) delete d->m_raw;
	if (d->m_own_clips) qDeleteAll(d->m_clips);
	delete d;
}
