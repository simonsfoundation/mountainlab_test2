#include "mvlabelcomparewidget.h"
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
#include <math.h>
#include <QSplitter>
#include "mvutils.h"

class MVLabelCompareWidgetPrivate {
public:
    MVLabelCompareWidget *q;

	Mda m_locations;
	DiskArrayModel *m_raw;
	bool m_own_raw;
    Mda m_templates_1;
    Mda m_templates_2;
    Mda m_times_1;
    Mda m_labels_1;
    Mda m_TL_1;
    Mda m_times_2;
    Mda m_labels_2;
    Mda m_TL_2;

	SSTimeSeriesView *m_labeled_raw_view;

	QSplitter *m_vsplitter;

	void update_sizes();
};


MVLabelCompareWidget::MVLabelCompareWidget(QWidget *parent) : QWidget(parent)
{
    d=new MVLabelCompareWidgetPrivate;
	d->q=this;

	d->m_raw=0;
	d->m_own_raw=false;

	d->m_labeled_raw_view=new SSTimeSeriesView;
	d->m_labeled_raw_view->initialize();
	//connect(d->m_labeled_raw_view,SIGNAL(currentXChanged()),this,SLOT(slot_current_raw_timepoint_changed()));

	{
		QSplitter *vsplitter=new QSplitter(Qt::Vertical);
		vsplitter->setHandleWidth(15);
		vsplitter->addWidget(d->m_labeled_raw_view);
		d->m_vsplitter=vsplitter;
	}

	QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(d->m_vsplitter);
	this->setLayout(hlayout);
}

void MVLabelCompareWidget::setElectrodeLocations(const Mda &L)
{
	d->m_locations=L;
}

void MVLabelCompareWidget::setTemplates1(const Mda &X)
{
    d->m_templates_1=X;
}

void MVLabelCompareWidget::setTemplates2(const Mda &X)
{
    d->m_templates_2=X;
}

void MVLabelCompareWidget::setRaw(DiskArrayModel *X,bool own_it)
{
	if ((d->m_raw)&&(d->m_own_raw)) delete d->m_raw;
	d->m_raw=X;
	d->m_own_raw=own_it;

	d->m_labeled_raw_view->setData(X,false);
}

void MVLabelCompareWidget::setTimesLabels(const Mda &times1, const Mda &labels1,const Mda &times2, const Mda &labels2)
{
    d->m_times_1=times1;
    d->m_labels_1=labels1;
    d->m_times_2=times2;
    d->m_labels_2=labels2;


    {
        int NN=times1.totalSize();
        Mda TL; TL.allocate(2,NN);
        int jj=0;
        for (int ii=0; ii<times1.totalSize(); ii++) {
            TL.setValue(times1.value1(ii),0,jj);
            TL.setValue(labels1.value1(ii),1,jj);
            jj++;
        }
        d->m_TL_1=TL;
    }
    {
        int NN=times2.totalSize();
        Mda TL; TL.allocate(2,NN);
        int jj=0;
        for (int ii=0; ii<times2.totalSize(); ii++) {
            TL.setValue(times2.value1(ii)-1,0,jj); //convert to zero-based indexing
            TL.setValue(labels2.value1(ii),1,jj);
            jj++;
        }
        d->m_TL_2=TL;
    }

    d->m_labeled_raw_view->setLabels(new DiskReadMda(d->m_TL_1),true);
    d->m_labeled_raw_view->setCompareLabels(new DiskReadMda(d->m_TL_2),true);
}


void MVLabelCompareWidget::updateWidgets()
{

}

void MVLabelCompareWidget::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt);

	d->update_sizes();
}



void MVLabelCompareWidgetPrivate::update_sizes()
{
    /*
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
    */

}

MVLabelCompareWidget::~MVLabelCompareWidget()
{
	if ((d->m_raw)&&(d->m_own_raw)) delete d->m_raw;
	delete d;
}
