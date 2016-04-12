#include "cvview.h"
#include "cvwidget.h"
#include "diskreadmdaold.h"
#include <QVBoxLayout>
#include <math.h>
#include <QList>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>
#include "cvcommon.h"
#include "sscommon.h"
#include <QApplication>

class CVWidgetPrivate {
public:
	CVWidget *q;

	CVView *m_view;
    DiskReadMdaOld m_features;
    DiskReadMdaOld m_clips;
    DiskReadMdaOld m_labels;
	float m_xmin,m_xmax,m_ymin,m_ymax,m_zmin,m_zmax;

	QColor get_label_color(int label);
	void generate_features_from_clips();
};

CVWidget::CVWidget(QWidget *parent) : QWidget(parent)
{
	d=new CVWidgetPrivate;
	d->q=this;

	d->m_xmin=d->m_ymin=d->m_zmin=-1;
	d->m_xmax=d->m_ymax=d->m_zmax=1;

	QVBoxLayout *L=new QVBoxLayout;
	L->setContentsMargins(0,0,0,0);
	L->setSpacing(0);
	setLayout(L);
	resize(600,600); //apparently needed if not set in main()

	d->m_view=new CVView;
	L->addWidget(d->m_view);
	d->m_view->setFocus();

	connect(d->m_view,SIGNAL(selectedDataPointsChanged()),this,SIGNAL(selectedDataPointsChanged()));
}

CVWidget::~CVWidget()
{
	delete d;
}

void CVWidget::setFeatures(const DiskReadMdaOld &X)
{
	d->m_features=X;
}

void CVWidget::setClips(const DiskReadMdaOld &X)
{
	d->m_clips=X;
	d->generate_features_from_clips();
}

void CVWidget::setLabels(const DiskReadMdaOld &L)
{
	d->m_labels=L;
}

void CVWidget::setRange(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	d->m_xmin=xmin; d->m_xmax=xmax;
	d->m_ymin=ymin; d->m_ymax=ymax;
	d->m_zmin=zmin; d->m_zmax=zmax;
}

void CVWidget::autoSetRange()
{
	if (d->m_features.totalSize()==1) return;
    DiskReadMdaOld X=d->m_features;
	int NN=X.N2();
	float xmin=0,xmax=0,ymin=0,ymax=0,zmin=0,zmax=0;
	for (int i=0; i<NN; i++) {
		float xval=X.value(0,i);
		float yval=X.value(1,i);
		float zval=X.value(2,i);
		if ((i==0)||(xval<xmin)) xmin=xval;
		if ((i==0)||(xval>xmax)) xmax=xval;
		if ((i==0)||(yval<ymin)) ymin=yval;
		if ((i==0)||(yval>ymax)) ymax=yval;
		if ((i==0)||(zval<zmin)) zmin=zval;
		if ((i==0)||(zval>zmax)) zmax=zval;
	}
	this->setRange(xmin,xmax,ymin,ymax,zmin,zmax);
}

void CVWidget::refresh()
{

	if (d->m_features.totalSize()==1) return;

	int M=d->m_features.N1();
	int N=d->m_features.N2();

	if (M!=3) {
		qWarning() << "Incorrect data dimensions:" << M << N;
		return;
	}

	if (d->m_labels.totalSize()>1) {
		if (d->m_labels.totalSize()!=N) {
			qWarning() << "Incorrect label dimensions" << d->m_labels.totalSize() << N;
			return;
		}
	}

	QList<QColor> colors;
	colors << QColor(100,30,30) << QColor(30,100,30) << QColor(30,30,100) << QColor(100,100,30) << QColor(100,30,100) << QColor(30,100,100);
	d->m_view->setLabelColors(colors);

	QList<CVDataPoint> pts;
	for (long i=0; i<N; i++) {
		CVDataPoint dp;
		float xval=d->m_features.value(0,i); xval=(xval-d->m_xmin)/(d->m_xmax-d->m_xmin)*2-1;
		float yval=d->m_features.value(1,i); yval=(yval-d->m_ymin)/(d->m_ymax-d->m_ymin)*2-1;
		float zval=d->m_features.value(2,i); zval=(zval-d->m_zmin)/(d->m_zmax-d->m_zmin)*2-1;
		dp.p=cvpoint(xval,yval,zval);
		if (d->m_labels.totalSize()>1) {
			int label=d->m_labels.value1(i);
			dp.label=label;
		}
		else dp.label=1;
		pts << dp;
//		for (int k=0; k<100; k++) {
//			CVDataPoint dp_test=dp;
//			dp_test.p.x+=(qrand()%10000)*1.0/10000*0.1;
//			dp_test.p.y+=(qrand()%10000)*1.0/10000*0.1;
//			dp_test.p.z+=(qrand()%10000)*1.0/10000*0.1;
//			pts << dp_test;
//		}
	}

	printf("adding %d points\n",pts.count());

	d->m_view->addDataPoints(pts);
}

void CVWidget::setSelectedDataPointIndices(const QList<int> &L)
{
	d->m_view->setSelectedDataPointIndices(L);
}

QList<int> CVWidget::selectedDataPointIndices()
{
	return d->m_view->selectedDataPointIndices();
}

void CVWidget::setNumDataPointsToSelect(int num)
{
    d->m_view->setNumDataPointsToSelect(num);
}

void CVWidget::setLabelStrings(const QStringList &strings)
{
    d->m_view->setLabelStrings(strings);
}


QColor CVWidgetPrivate::get_label_color(int label)
{
	if (label==1) return Qt::red;
	else return Qt::blue;
}


void CVWidgetPrivate::generate_features_from_clips()
{
	//TO DO -- this should be replaced by a procedure that does not involve system call
	/*
    if (m_clips.totalSize()<=1) {
		QMessageBox::critical(q,"Problem generating features.","Problem generating features. m_clips is null.");
		return;
	}

	QProgressDialog dlg("Generating features","Cancel",0,100);
	dlg.show();
	qApp->processEvents();

	QString inpath=ssTempPath()+"/spikespy_"+make_random_id(10)+".mda";
	QString outpath=ssTempPath()+"/spikespy_"+make_random_id(10)+".mda";
	removeOnClose(inpath);
	removeOnClose(outpath);

	m_clips.write(inpath);

	QString exe=qApp->applicationDirPath()+"/ssfeatures";
    if (!QFile::exists(exe)) {
        QString exe2=qApp->applicationDirPath()+"/../src/spikespy/bin/ssfeatures";
        if (QFile::exists(exe2)) {
            exe=exe2;
        }
    }
	QStringList args; args << inpath << outpath << "--method=pca" << "--nfeatures=3";
	int ret=QProcess::execute(exe,args);

	if (ret<0) {
		QMessageBox::critical(q,"Problem generating features.","Problem generating features.");
		return;
	}

    m_features=DiskReadMdaOld(outpath);
	*/
}
