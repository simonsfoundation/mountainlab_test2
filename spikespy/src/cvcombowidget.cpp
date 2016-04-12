#include "cvcombowidget.h"
#include "cvwidget.h"
#include "sstimeseriesview_prev.h"
#include "sstimeserieswidget_prev.h"
#include <QDir>

#include <QSplitter>
#include <qboxlayout.h>
#include "diskreadmdaold.h"
#include "diskwritemdaold.h"
#include "cvcommon.h"

class CVComboWidgetPrivate {
public:
	CVComboWidget *q;

	CVWidget *m_cv_widget;
	SSTimeSeriesWidget *m_ts_widget;
	SSTimeSeriesView *m_ts_view;
    DiskReadMdaOld m_clips;
    DiskReadMdaOld m_timepoint_mapping;
	QSplitter *m_splitter;
};

CVComboWidget::CVComboWidget(QWidget *parent) : QWidget(parent)
{
	d=new CVComboWidgetPrivate;
	d->q=this;

	d->m_cv_widget=new CVWidget;
	d->m_ts_widget=new SSTimeSeriesWidget;
	d->m_ts_view=new SSTimeSeriesView;
	d->m_ts_view->setClipMode(true);
	d->m_ts_widget->addView(d->m_ts_view);

	QVBoxLayout *vlayout=new QVBoxLayout;
	setLayout(vlayout);

	QSplitter *splitter=new QSplitter;
	d->m_splitter=splitter;
	splitter->setOrientation(Qt::Vertical);
	vlayout->addWidget(splitter);

	splitter->addWidget(d->m_cv_widget);
	splitter->addWidget(d->m_ts_widget);
	splitter->setStretchFactor(0,1);
	splitter->setStretchFactor(1,1);
	d->m_ts_widget->setMinimumHeight(30);

	connect(d->m_cv_widget,SIGNAL(selectedDataPointsChanged()),this,SLOT(slot_update_selected_clips()));
}

CVComboWidget::~CVComboWidget()
{
	delete d;
}

void CVComboWidget::setClips(DiskReadMdaOld X)
{
	d->m_clips=X;
	d->m_cv_widget->setClips(X);
	d->m_cv_widget->autoSetRange();
	d->m_cv_widget->refresh();
}

void CVComboWidget::setTimepointMapping(DiskReadMdaOld TM)
{
	d->m_timepoint_mapping=TM;
}

SSTimeSeriesView *CVComboWidget::timeSeriesView()
{
	return d->m_ts_view;
}

void CVComboWidget::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt)
	QList<int> sizes=d->m_splitter->sizes(); while (sizes.count()<2) sizes << 0;
	int tot=0;
	for (int i=0; i<sizes.count(); i++) tot+=sizes[i];
	sizes[0]=(int)(tot*0.6); sizes[1]=tot-sizes[0];
	d->m_splitter->setSizes(sizes);
}

void CVComboWidget::slot_update_selected_clips()
{
	QList<int> inds=d->m_cv_widget->selectedDataPointIndices();
	int N1=d->m_clips.N1();
	int N2=d->m_clips.N2();

	QString path=QString(ssTempPath()+"/spikespy-tmp-cvcombowidget-%1.mda").arg(qAbs(qrand()));;
	removeOnClose(path);

    DiskWriteMdaOld X(path);
	X.allocate(N1,N2,inds.count());
	for (int c=0; c<inds.count(); c++) {
		for (int t=0; t<N2; t++) {
			for (int k=0; k<N1; k++) {
				X.setValue(d->m_clips.value(k,t,inds[c]),k,t,c);
			}
		}
	}

	if (d->m_timepoint_mapping.totalSize()!=1) {
        DiskWriteMdaOld TM;
		TM.allocate(1,N2,inds.count());
		for (int c=0; c<inds.count(); c++) {
			for (int t=0; t<N2; t++) {
				float val=d->m_timepoint_mapping.value(0,t,inds[c]);
				TM.setValue(val,0,t+N2*c);
			}
		}
		d->m_ts_view->setTimepointMapping(TM.toReadMda());
	}
	else {
        d->m_ts_view->setTimepointMapping(DiskReadMdaOld());
	}

	DiskArrayModel *MM=new DiskArrayModel();
	MM->setPath(path);
	d->m_ts_view->setData(MM,true);

	d->m_ts_view->initialize();
	d->m_ts_view->setCurrentX(-1); d->m_ts_view->setCurrentX(N2/2); //a hack trigger sync
}

