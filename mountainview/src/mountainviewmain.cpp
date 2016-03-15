#include <QApplication>
#include <QScriptEngine>
#include <QDebug>
#include <qdatetime.h>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QStringList>
#include "textfile.h"
#include "usagetracking.h"
#include "cvcommon.h"
#include "mountainviewwidget.h"
#include "mda.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include "get_command_line_params.h"
#include "diskarraymodel.h"
#include "histogramview.h"
#include "mvoverviewwidget.h"
#include "mvlabelcomparewidget.h"
#include "mvoverview2widget.h"
#include "sstimeserieswidget.h"
#include "sstimeseriesview.h"
#include "mvclusterwidget.h"

/*
 * TO DO:
 * Clean up temporary files
 * */

bool download_file(QString url,QString fname) {
	QStringList args; args << "-o" << fname << url;
	int ret=QProcess::execute("/usr/bin/curl",args);
	if (ret!=0) {
		if (QFile::exists(fname)) {
			QFile::remove(fname);
		}
		return false;
	}
	return QFile::exists(fname);
}

void test_histogramview() {

	int N=100;
	float values[N];
	for (int i=0; i<N; i++) {
		values[i]=(qrand()%10000)*1.0/10000;
	}

	HistogramView *W=new HistogramView;
	W->setData(N,values);
	W->autoSetBins(N/5);
	W->show();
}


int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	printf("MountainView main\n");
	//MainWindow w;
	//w.show();

//	{
//		test_histogramview();
//		return a.exec();
//	}


    QStringList required;
    QStringList optional;
    CLParams CLP=get_command_line_params(argc,argv,required);

    printf("testing\n");

	QStringList args;
	for (int i=1; i<argc; i++) {
		args << QString(argv[i]);
	}

	qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());

	QString working_path=CLP.named_parameters.value("working_path");
	QString output_path=CLP.named_parameters.value("output_path");

    QString mode=CLP.named_parameters.value("mode","overview");
    QString templates_path=CLP.named_parameters.value("templates");
    QString locations_path=CLP.named_parameters.value("locations");
    QString raw_path=CLP.named_parameters.value("raw");
    QString filt_path=CLP.named_parameters.value("filt");
    QString pre_path=CLP.named_parameters.value("pre");
    QString times_path=CLP.named_parameters.value("times");
    QString labels_path=CLP.named_parameters.value("labels");
	QString firings_path=CLP.named_parameters.value("firings");
	QString window_title=CLP.named_parameters.value("title");
	if (firings_path.isEmpty()) firings_path=CLP.named_parameters.value("clusters"); //historical compatibility
	if (firings_path.isEmpty()) firings_path=CLP.named_parameters.value("cluster"); //historical compatibility
    QString primary_channels_path=CLP.named_parameters.value("primary-channels");
    QString cross_correlograms_path=CLP.named_parameters.value("cross-correlograms");
	if (cross_correlograms_path.isEmpty()) cross_correlograms_path=CLP.named_parameters.value("cross_correlograms");
	QString clips_path=CLP.named_parameters.value("clips");
	QString clips_index_path=CLP.named_parameters.value("clips-index");
	if (clips_index_path.isEmpty()) clips_index_path=CLP.named_parameters.value("clips_index");

	QString firings2_path=CLP.named_parameters.value("firings2"); //for mode=compare_labels
	if (firings2_path.isEmpty()) firings2_path=CLP.named_parameters.value("clusters2"); //historical compatibility
	if (firings2_path.isEmpty()) firings2_path=CLP.named_parameters.value("cluster2"); //historical compatibility

	float sampling_freq=CLP.named_parameters.value("sampling_freq","0").toFloat();

    if (mode=="overview") {
        //OBSOLETE!!! use overview2
        MVOverviewWidget *W=new MVOverviewWidget;
        W->setWindowTitle(CLP.named_parameters.value("window_title","MountainView"));
        W->show();
        W->move(QApplication::desktop()->screen()->rect().topLeft()+QPoint(200,200));
        W->resize(1800,1200);
        if (!templates_path.isEmpty()) {
            Mda X; X.read(templates_path);
            W->setTemplates(X);
        }
        if (!locations_path.isEmpty()) {
            Mda X; X.read(locations_path);
            W->setElectrodeLocations(X);
        }
        if (!raw_path.isEmpty()) {
            DiskArrayModel *X=new DiskArrayModel;
            X->setPath(raw_path);
            W->setRaw(X,true);
        }
        if (!clips_path.isEmpty()) {
            DiskArrayModel *X=new DiskArrayModel;
            X->setPath(clips_path);
            W->setClips(X,true);
        }
        if (!clips_index_path.isEmpty()) {
            Mda X; X.read(clips_index_path);
            W->setClipsIndex(X);
        }
        if (!times_path.isEmpty()) {
            Mda T; T.read(times_path);
            Mda L;
            if (!labels_path.isEmpty()) {
                L.read(labels_path);
            }
            else {
                L.allocate(T.N1(),T.N2());
                for (int ii=0; ii<L.totalSize(); ii++) L.setValue1(1,ii);
            }
            W->setTimesLabels(T,L);
        }
		if (!firings_path.isEmpty()) {
			Mda CC; CC.read(firings_path);
            int num_events=CC.N2();
            Mda T,L;
            T.allocate(1,num_events);
            L.allocate(1,num_events);
            for (int i=0; i<num_events; i++) {
                T.setValue(CC.value(1,i),0,i);
                L.setValue(CC.value(2,i),0,i);
            }
            W->setTimesLabels(T,L);
        }
        {
            Mda PC; PC.read(primary_channels_path);
            W->setPrimaryChannels(PC);
        }
        {
            W->setCrossCorrelogramsPath(cross_correlograms_path);
        }
        W->updateWidgets();
    }
	else if (mode=="overview2") {
		printf("overview2...\n");
		MVOverview2Widget *W=new MVOverview2Widget;
        if (!pre_path.isEmpty()) {
            W->addRawPath("Preprocessed Data",pre_path);
        }
        if (!filt_path.isEmpty()) {
            W->addRawPath("Filtered Data",filt_path);
        }
        if (!raw_path.isEmpty()) {
            W->addRawPath("Raw Data",raw_path);
        }
        QString epochs_path=CLP.named_parameters["epochs"];
        if (!epochs_path.isEmpty()) {
            QList<Epoch> epochs=read_epochs(epochs_path);
            W->setEpochs(epochs);
        }
		if (window_title.isEmpty()) window_title=pre_path;
		if (window_title.isEmpty()) window_title=filt_path;
		if (window_title.isEmpty()) window_title=raw_path;
		W->setFiringsPath(firings_path);
		W->show();
		W->setSamplingFrequency(sampling_freq);
		W->move(QApplication::desktop()->screen()->rect().topLeft()+QPoint(200,200));
		int W0=1300,H0=400;
		QRect geom=QApplication::desktop()->geometry();
		if ((geom.width()-100<W0)||(geom.height()-100<H0)) {
			//W->showMaximized();
		}
		else {
			W->resize(W0,H0);
		}
        W->setDefaultInitialization();
		W->setWindowTitle(window_title);

	}
    else if (mode=="view_clusters") {
        MVClusterWidget *W=new MVClusterWidget;
        QString data_path=CLP.named_parameters.value("data");
        QString labels_path=CLP.named_parameters.value("labels");
        Mda data0; data0.read(data_path);
        W->setData(data0);
        if (~labels_path.isEmpty()) {
            Mda labels0; labels0.read(labels_path);
            int NN=labels0.totalSize();
            QList<int> labels; for (int i=0; i<NN; i++) labels << labels0.value1(i);
            W->setLabels(labels);
        }
        W->resize(1000,500);
        W->show();
    }
	else if (mode=="spikespy") {
		printf("spikespy...\n");
		SSTimeSeriesWidget *W=new SSTimeSeriesWidget;
        W->hideMenu();
		SSTimeSeriesView *V=new SSTimeSeriesView;
		V->setSamplingFrequency(sampling_freq);
		DiskArrayModel *DAM=new DiskArrayModel;
		DAM->setPath(raw_path);
		V->setData(DAM,true);
		Mda firings; firings.read(firings_path);
		QList<long> times,labels;
		for (int i=0; i<firings.N2(); i++) {
			times << (long)firings.value(1,i);
			labels << (long)firings.value(2,i);
		}
		V->setTimesLabels(times,labels);
		W->addView(V);
		W->show();
		W->move(QApplication::desktop()->screen()->rect().topLeft()+QPoint(200,200));
		W->resize(1800,1200);
	}
    else if (mode=="compare_labels") {
        printf("compare_labels...\n");
        MVLabelCompareWidget *W=new MVLabelCompareWidget;
        W->setWindowTitle(CLP.named_parameters.value("window_title","MountainView - Compare Labels"));
        W->show();
        W->move(QApplication::desktop()->screen()->rect().topLeft()+QPoint(200,200));
        W->resize(1800,1200);
        if (!locations_path.isEmpty()) {
            Mda X; X.read(locations_path);
            W->setElectrodeLocations(X);
        }
        if (!raw_path.isEmpty()) {
            DiskArrayModel *X=new DiskArrayModel;
            X->setPath(raw_path);
            W->setRaw(X,true);
        }
		if ((!firings_path.isEmpty())&&(!firings2_path.isEmpty())) {
            Mda T1,L1,T2,L2;
            {
				Mda CC; CC.read(firings_path);
                int num_events=CC.N2();
                Mda T,L;
                T.allocate(1,num_events);
                L.allocate(1,num_events);
                for (int i=0; i<num_events; i++) {
                    T.setValue(CC.value(1,i),0,i);
                    L.setValue(CC.value(2,i),0,i);
                }
                T1=T; L1=L;
            }
            {
				Mda CC; CC.read(firings2_path);
                int num_events=CC.N2();
                Mda T,L;
                T.allocate(1,num_events);
                L.allocate(1,num_events);
                for (int i=0; i<num_events; i++) {
                    T.setValue(CC.value(1,i),0,i);
                    L.setValue(CC.value(2,i),0,i);
                }
                T2=T; L2=L;
            }
            W->setTimesLabels(T1,L1,T2,L2);
        }
        W->updateWidgets();
    }

	int ret=a.exec();

	printf("Number of files open: %d, number of unfreed mallocs: %d, number of unfreed megabytes: %g\n",jnumfilesopen(),jmalloccount(),(int)jbytesallocated()*1.0/1000000);

	return ret;
}
