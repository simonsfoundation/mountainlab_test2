#include "mountainviewwidget.h"
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
#include <QMessageBox>
#include <QTextBrowser>
#include "firetrackwidget.h"
#include "histogramview.h"
#include <math.h>

class MountainViewWidgetPrivate {
public:
	MountainViewWidget *q;

	Mda m_primary_channels;
	Mda m_templates;
	Mda m_locations;
	DiskArrayModel *m_raw,*m_raw_whitened;
    Mda m_times;
    Mda m_labels;
    DiskReadMda *m_times_labels;
    int m_template_view_padding;
    QList<SSTimeSeriesView *> m_spike_template_views;
    QList<SSTimeSeriesView *> m_labeled_raw_data_views;
    QList<SSTimeSeriesView *> m_clips_views;
    int m_current_template_index;
    QString m_cross_correlograms_path;

    void connect_spike_templates_view(SSTimeSeriesView *V);
    void connect_labeled_raw_data_view(SSTimeSeriesView *V);
    void connect_clips_view(SSTimeSeriesView *V);
    void update_clips_view(SSTimeSeriesWidget *W,SSTimeSeriesView *V,int label);
};


MountainViewWidget::MountainViewWidget(QWidget *parent) : QMainWindow(parent)
{
	d=new MountainViewWidgetPrivate;
	d->q=this;

    d->m_raw=0;
    d->m_raw_whitened=0;
    d->m_times_labels=0;
    d->m_template_view_padding=30;

    d->m_current_template_index=0;

    QGridLayout *GL=new QGridLayout;
    int row=0;
    {
        QPushButton *B=new QPushButton("Labeled Raw Data");
        GL->addWidget(B,row,0);
        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_view_labeled_raw_data()));
    }
    {
        QPushButton *B=new QPushButton("Labeled Raw Data (Whitened)");
        B->setProperty("whitened",true);
        GL->addWidget(B,row,1); row++;
        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_view_labeled_raw_data()));
    }
    {
        QPushButton *B=new QPushButton("Spike Templates");
        GL->addWidget(B,row,0);
        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_view_spike_templates()));
    }
    {
        QPushButton *B=new QPushButton("Spike Templates (Whitened)");
        B->setProperty("whitened",true);
        GL->addWidget(B,row,1); row++;
        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_view_spike_templates()));
    }
    {
        QPushButton *B=new QPushButton("Spike Clips");
        GL->addWidget(B,row,0); row++;
        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_view_spike_clips()));
    }
	{
		QPushButton *B=new QPushButton("Statistics");
        GL->addWidget(B,row,0); row++;
		connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_statistics()));
	}
	{
		QPushButton *B=new QPushButton("FireTrack");
        GL->addWidget(B,row,0); row++;
		connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_firetrack()));
	}
    {
        QPushButton *B=new QPushButton("Cross-Correlograms");
        GL->addWidget(B,row,0); row++;
        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_cross_correlograms()));
    }
	{
		QPushButton *B=new QPushButton("Quit");
        GL->addWidget(B,row,0); row++;
		connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_quit()));
	}
//    {
//        QPushButton *B=new QPushButton("Cluster View");
//        GL->addWidget(B,row,0); row++;
//        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_cluster_view()));
//    }
//    {
//        QPushButton *B=new QPushButton("FireTrack");
//        GL->addWidget(B,row,0); row++;
//        connect(B,SIGNAL(clicked(bool)),this,SLOT(slot_firetrack()));
//    }


    QWidget *CW=new QWidget;
    CW->setLayout(GL);
    this->setCentralWidget(CW);

    this->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
    this->setWindowFlags(Qt::Window|Qt::Tool);
    this->setWindowFlags(this->windowFlags()|Qt::CustomizeWindowHint);
    this->setWindowFlags(this->windowFlags() ^ Qt::WindowCloseButtonHint);
}

void MountainViewWidget::setElectrodeLocations(const Mda &L)
{
	d->m_locations=L;
}

void MountainViewWidget::setTemplates(const Mda &X)
{
	d->m_templates=X;
}
void MountainViewWidget::setTemplatesWhitened(const Mda &X)
{
    d->m_templates_whitened=X;
}

void MountainViewWidget::setPrimaryChannels(const Mda &X)
{
	d->m_primary_channels=X;
}

void MountainViewWidget::setRaw(DiskArrayModel *X)
{
    if (d->m_raw) delete d->m_raw;
    d->m_raw=X;
}

void MountainViewWidget::setRawWhitened(DiskArrayModel *X)
{
    if (d->m_raw_whitened) delete d->m_raw_whitened;
    d->m_raw_whitened=X;
}

void MountainViewWidget::setTimesLabels(const Mda &times, const Mda &labels)
{
    d->m_times=times;
    d->m_labels=labels;
    int NN=d->m_times.totalSize();
    Mda TL; TL.allocate(2,NN);
    for (int ii=0; ii<NN; ii++) {
        TL.setValue(d->m_times.value1(ii),0,ii);
        TL.setValue(d->m_labels.value1(ii),1,ii);
    }

    d->m_times_labels=new DiskReadMda(TL);
}

void MountainViewWidget::setCrossCorrelogramsPath(const QString &path)
{
    d->m_cross_correlograms_path=path;
}

void MountainViewWidget::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt);
	{
		float W0=this->width()*0.15;
		W0=qMax(W0,80.0F);
		W0=qMin(W0,300.0F);
        //d->m_waveform_list->setFixedWidth(W0);
	}
	{
		float W0=this->width()*0.2;
		W0=qMax(W0,150.0F);
		W0=qMin(W0,350.0F);
        //d->m_plot->setFixedWidth(W0);
    }
}

void MountainViewWidget::slot_view_labeled_raw_data()
{
    if (!d->m_raw) return;
    if (sender()->property("whitened").toBool()) {
        if (!d->m_raw_whitened) return;
    }
    SSTimeSeriesWidget *W=new SSTimeSeriesWidget;
    SSTimeSeriesView *V=new SSTimeSeriesView;
    W->addView(V);
    if (sender()->property("whitened").toBool()) {
        V->setData(d->m_raw_whitened,false);
    }
    else {
        V->setData(d->m_raw,false);
    }
    if (d->m_times_labels) {
        V->setLabels(d->m_times_labels,false);
    }
    V->initialize();

    W->show();
    W->setAttribute(Qt::WA_DeleteOnClose);
    W->move(this->topLevelWidget()->geometry().bottomLeft()+QPoint(0,50));
    W->resize(1600,700);

    d->connect_labeled_raw_data_view(V);
}

struct SortRec {
	double val;
	int index;
};
bool caseInsensitiveLessThan(const SortRec &R1, const SortRec &R2)
{
  return R1.val<R2.val;
}
QList<int> get_sort_order(const QList<double> &values) {
	QList<SortRec> list;
	for (int i=0; i<values.count(); i++) {
		SortRec RR; RR.val=values[i]; RR.index=i;
		list << RR;
	}
	qSort(list.begin(),list.end(),caseInsensitiveLessThan);
	QList<int> ret;
	for (int i=0; i<list.count(); i++) {
		ret << list[i].index;
	}
	return ret;
}

QList<int> get_template_sort_order(const Mda &X) {
	int K=X.N3();
	QList<double> values;
	for (int k=0; k<K; k++) {
		double sum1=0;
		double sum2=0;
		for (int t=0; t<X.N2(); t++) {
			for (int m=0; m<X.N1(); m++) {
				double val0=X.value(m,t,k);
				sum1+=val0*val0*m;
				sum2+=val0*val0;
			}
		}
		if (sum2>0) sum1/=sum2;
		values << sum1;
	}
	QList<int> inds=get_sort_order(values);
	return inds;
}

void MountainViewWidget::slot_view_spike_templates()
{
    SSTimeSeriesWidget *W=new SSTimeSeriesWidget;
    SSTimeSeriesView *V=new SSTimeSeriesView;
    W->addView(V);
    DiskArrayModel *data=new DiskArrayModel;
    int M=d->m_templates.N1();
    int T=d->m_templates.N2();
    int K=d->m_templates.N3();
    int padding=d->m_template_view_padding;
    Mda templates_formatted; templates_formatted.allocate(M,(T+padding)*K);
    Mda TL; TL.allocate(2,K);
    Mda *templates=&d->m_templates;
    if (sender()->property("whitened").toBool()) {
        templates=&d->m_templates_whitened;
    }
	//QList<int> order=get_template_sort_order(*templates);
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
	V->setVerticalZoomFactor(0.6);
    V->setLabels(new DiskReadMda(TL),true);
    V->initialize();

    W->show();
	W->setAttribute(Qt::WA_DeleteOnClose);
	W->move(this->topLevelWidget()->geometry().topRight()+QPoint(300,-100));
    W->resize(800,400);

    d->connect_spike_templates_view(V);
}

void MountainViewWidget::slot_view_spike_clips()
{
    if (!d->m_raw) return;

    QString txt=QInputDialog::getText(this,"View Spike Clips","Label number:",QLineEdit::Normal,"1");
    int label=txt.toInt();
    if (label<=0) return;

    SSTimeSeriesWidget *W=new SSTimeSeriesWidget;
    SSTimeSeriesView *V=new SSTimeSeriesView;
    V->setClipMode(true);
    V->setProperty("fixed_clipsize",true);
	V->setVerticalZoomFactor(0.5);
    W->addView(V);
	d->update_clips_view(W,V,label);

    W->show();
    W->setAttribute(Qt::WA_DeleteOnClose);
    W->move(this->topLevelWidget()->geometry().topRight()+QPoint(500,400));
    W->resize(800,400);

    d->connect_clips_view(V);
}

void MountainViewWidget::slot_firetrack() {
	FireTrackWidget *W=new FireTrackWidget;
	W->setElectrodeLocations(d->m_locations);
	W->setWaveforms(d->m_templates);
	W->electrodeArrayWidget()->setShowChannelNumbers(true);

	W->show();
	W->setAttribute(Qt::WA_DeleteOnClose);
	W->move(this->topLevelWidget()->geometry().topRight()+QPoint(500,400));
	W->resize(800,400);
}

void MountainViewWidget::slot_cluster_view()
{

}

struct SpikeStats {
	int count;
};

QString read_text_file(QString path) {
	QFile FF(path);
	if (!FF.open(QFile::Text|QFile::ReadOnly)) {
		return "";
	}
	QString ret=QString(FF.readAll());
	FF.close();
	return ret;
}

void MountainViewWidget::slot_statistics()
{
	QList<SpikeStats> spike_stats;
	int num=d->m_times.totalSize();
	for (int ii=0; ii<num; ii++) {
		//int t0=d->m_times.value1(ii);
		int label0=d->m_labels.value1(ii);
		while (label0>=spike_stats.count()) {
			SpikeStats X;
			X.count=0;
			spike_stats << X;
		}
		spike_stats[label0].count++;
	}

	QString statistics_css=read_text_file(":/statistics.css");

	QString html;
	html+="<html>\n";
	html+=QString("<head>\n");
	html+=QString("<style>\n");
	html+=QString("%1\n\n").arg(statistics_css);
	html+=QString("</style>\n");
	html+=QString("</head>\n");
	html+=QString("<table>\n");
	html+=QString("<tr><th>Template</th><th>Primary Channel</th><th># Spikes</th><th>Spikes per minute</th></tr>");
	for (int k=1; k<spike_stats.count(); k++) {
		SpikeStats X=spike_stats[k];
		int primary_channel=(int)d->m_primary_channels.value1(k-1);
		if (X.count!=0) {
			float frequency=X.count*1.0/(d->m_raw->size(1)*1.0/30000/60);
			html+=QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>\n")
					.arg(k)
					.arg(primary_channel)
					.arg(X.count)
					.arg(frequency,0,'f',1,' ');
		}
	}
	html+=QString("</table>\n");
	html+="</html>\n";

	{
		QTextBrowser *W=new QTextBrowser;
		W->setAttribute(Qt::WA_DeleteOnClose);
		W->setHtml(html);
		W->show();

		W->resize(600,800);
		W->move(this->geometry().topRight()+QPoint(100,50));
    }
}

typedef QList<float> FloatList;

QList<float> get_cross_correlogram_data(DiskReadMda &X,int k1,int k2) {
    QList<float> ret;
    for (int i=0; i<X.N2(); i++) {
        if ((X.value(0,i)==k1)&&(X.value(1,i)==k2)) ret << X.value(2,i);
    }
    return ret;
}

QList<FloatList> get_cross_correlogram_datas(DiskReadMda &X,int k,int K) {
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

void MountainViewWidget::slot_cross_correlograms(int k0)
{
    QWidgetList widgets=qApp->allWidgets();
    foreach (QWidget *widget,widgets) {
        if (widget->property("slot_cross_correlograms").toBool()) {
            int kk=widget->property("slot_cross_correlograms_k0").toInt();
            if (kk==k0) {
                widget->raise();
                return;
            }
        }
    }

    if (d->m_cross_correlograms_path.isEmpty()) {
        QMessageBox::information(this,"Unable to open cross-correlograms view","There was a problem opening the cross-correlograms view (empty path).");
        return;
    }
    DiskReadMda X;
    X.setPath(d->m_cross_correlograms_path);

    int K=d->m_templates.N3();
    int num_rows=(int)sqrt(K); if (num_rows<1) num_rows=1;
    int num_cols=(K+num_rows-1)/num_rows;

    QProgressDialog dlg;
    dlg.show();
    dlg.setLabelText("Loading cross correlograms...");
    dlg.repaint(); qApp->processEvents();
    QList<FloatList> data0=get_cross_correlogram_datas(X,k0,K);

    QWidget *W=new QWidget;
    W->setAttribute(Qt::WA_DeleteOnClose);
    QGridLayout *GL=new QGridLayout;
    W->setLayout(GL);

    for (int k1=1; k1<=K; k1++) {
        HistogramView *HV=new HistogramView;
        HV->setData(data0[k1]);
        HV->autoSetBins(50);
        int k2=k1; if (k0>=1) k2=k0;
        QString title0=QString("%1/%2").arg(k1).arg(k2);
        HV->setTitle(title0);
        GL->addWidget(HV,(k1-1)/num_cols,(k1-1)%num_cols);
        HV->setProperty("cross-correlogram-k",k1);
        connect(HV,SIGNAL(clicked()),this,SLOT(slot_cross_correlogram_clicked()));
    }
    W->show();
	//int W0=num_rows*150; if (W0>1500) W0=1500;
	//int H0=num_cols*150; if (H0>1500) H0=1500;
	int W0=1500,H0=1500;
    W->resize(W0,H0);
    if (k0==0) {
        W->move(this->topLevelWidget()->geometry().topRight()+QPoint(300,-100));
    }
    else {
        W->move(this->topLevelWidget()->geometry().bottomLeft()+QPoint(100,100));
    }

    if (k0==0) {
        W->setWindowTitle("Cross-Correlograms (Diagonal)");
    }
    else {
        W->setWindowTitle(QString("Cross-Correlograms (Neuron %1)").arg(k0));
    }

    W->setProperty("slot_cross_correlograms",true);
    W->setProperty("slot_cross_correlograms_k0",k0);
}

void MountainViewWidget::slot_quit()
{
    qApp->quit();
}

void MountainViewWidget::slot_cross_correlogram_clicked()
{
    int k=sender()->property("cross-correlogram-k").toInt();
    slot_cross_correlograms(k);
}

void MountainViewWidget::slot_spike_templates_x_changed()
{
    SSTimeSeriesView *V=(SSTimeSeriesView *)sender();
    int x=V->currentX();
    int T=d->m_templates.N2();
    int k=x/(T+d->m_template_view_padding) + 1;
    d->m_current_template_index=k;

    //if (d->m_clips_widget->isVisible()) {
    //    d->update_clips_view();
    //}
}

void MountainViewWidget::slot_object_destroyed(QObject *obj)
{
    d->m_spike_template_views.removeOne((SSTimeSeriesView *)obj);
    d->m_labeled_raw_data_views.removeOne((SSTimeSeriesView *)obj);
    d->m_clips_views.removeOne((SSTimeSeriesView *)obj);
}

MountainViewWidget::~MountainViewWidget()
{
    if (d->m_raw) delete d->m_raw;
    if (d->m_raw_whitened) delete d->m_raw_whitened;
    if (d->m_times_labels) delete d->m_times_labels;
	delete d;
}

void MountainViewWidgetPrivate::connect_spike_templates_view(SSTimeSeriesView *V)
{
    QObject::connect(V,SIGNAL(currentXChanged()),q,SLOT(slot_spike_templates_x_changed()));
    QObject::connect(V,SIGNAL(destroyed(QObject*)),q,SLOT(slot_object_destroyed(QObject*)));
    m_spike_template_views << V;
}

void MountainViewWidgetPrivate::connect_labeled_raw_data_view(SSTimeSeriesView *V)
{
    QObject::connect(V,SIGNAL(destroyed(QObject*)),q,SLOT(slot_object_destroyed(QObject*)));
    m_labeled_raw_data_views << V;
}

void MountainViewWidgetPrivate::connect_clips_view(SSTimeSeriesView *V)
{
    QObject::connect(V,SIGNAL(destroyed(QObject*)),q,SLOT(slot_object_destroyed(QObject*)));
    m_clips_views << V;
}

Mda extract_clips(DiskArrayModel *X,const Mda &times,const Mda &labels,int label) {

	//TO DO: replace call to X->value with X->loadData (should be way faster)
	Mda empty;
	return empty;
	/*
    Mda clips;

    QList<int> times0;
    for (int i=0; i<labels.totalSize(); i++) {
        if (labels.value1(i)==label) {
            times0 << (int)times.value1(i);
        }
    }

    int M=X->size(0);
    int N=X->size(1);
    int T=100;
    int NC=times0.count();

    int dtmin=-T/2;
    int dtmax=dtmin+T-1;

    clips.allocate(M,T,NC);
    for (int ii=0; ii<NC; ii++) {
        int t0=times0[ii];
        if ((t0+dtmin>=0)&&(t0+dtmax<N)) {
            for (int t=0; t<T; t++) {
                int t1=t0+dtmin+t;
                for (int m=0; m<M; m++) {
                    clips.setValue(X->value(m,t1),m,t,ii);
                }
            }
        }
    }
    return clips;
	*/
}

Mda format_clips(const Mda &clips,int padding) {
    int M=clips.N1();
    int T=clips.N2();
    int NC=clips.N3();
    Mda clips2;
    clips2.allocate(M,(T+padding)*NC);
    for (int i=0; i<NC; i++) {
        for (int t=0; t<T; t++) {
            for (int m=0; m<M; m++) {
                clips2.setValue(clips.value(m,t,i),m,t+(T+padding)*i);
            }
        }
    }
    return clips2;
}

void MountainViewWidgetPrivate::update_clips_view(SSTimeSeriesWidget *W,SSTimeSeriesView *V,int label)
{
    QProgressDialog dlg;
	dlg.setWindowTitle(QString("Extracting clips for template %1").arg(label));
    dlg.setRange(0,100);
    dlg.show();
	dlg.setLabelText(QString("Extracting clips for template %1...").arg(label));
    dlg.setValue(0); dlg.repaint(); qApp->processEvents();
    Mda clips=extract_clips(m_raw,m_times,m_labels,label);
    dlg.setLabelText("Formatting clips...");
    dlg.setValue(50); dlg.repaint(); qApp->processEvents();
    Mda clips2=format_clips(clips,m_template_view_padding);
    DiskArrayModel *MM=new DiskArrayModel;
    MM->setFromMda(clips2);
    dlg.setLabelText("Initializing...");
    dlg.setValue(75); dlg.repaint(); qApp->processEvents();
    V->setData(MM,true);
    V->initialize();
    W->setClipData(clips);
    W->setWindowTitle(QString("Spike Clips -- template %1 -- %2 spikes").arg(label).arg(clips.N3()));
}
