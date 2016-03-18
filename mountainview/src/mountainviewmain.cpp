#include <QApplication>
#include <QDebug>
#include <qdatetime.h>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QStringList>
#include "textfile.h"
#include "usagetracking.h"
#include "mda.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include "get_command_line_params.h"
#include "diskarraymodel.h"
#include "histogramview.h"
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

	CLParams CLP=get_command_line_params(argc,argv);
	QString mode=CLP.named_parameters.value("mode","overview2").toString();

	if (mode=="overview2") {
		printf("overview2...\n");
		QString pre_path=CLP.named_parameters["pre"].toString();
		QString filt_path=CLP.named_parameters["filt"].toString();
        QString signal_path=CLP.named_parameters["signal"].toString();
		QString firings_path=CLP.named_parameters["firings"].toString();
        double samplerate=CLP.named_parameters["samplerate"].toDouble();
		QString epochs_path=CLP.named_parameters["epochs"].toString();
		QString window_title=CLP.named_parameters["window_title"].toString();
		MVOverview2Widget *W=new MVOverview2Widget;
        if (!pre_path.isEmpty()) {
            W->addSignalPath("Preprocessed Data",pre_path);
        }
        if (!filt_path.isEmpty()) {
            W->addSignalPath("Filtered Data",filt_path);
        }
        if (!signal_path.isEmpty()) {
            W->addSignalPath("Raw Data",signal_path);
        }

        if (!epochs_path.isEmpty()) {
            QList<Epoch> epochs=read_epochs(epochs_path);
            W->setEpochs(epochs);
        }
		if (window_title.isEmpty()) window_title=pre_path;
		if (window_title.isEmpty()) window_title=filt_path;
        if (window_title.isEmpty()) window_title=signal_path;
		W->setFiringsPath(firings_path);
		W->show();
        W->setSamplingFrequency(samplerate);
		W->move(QApplication::desktop()->screen()->rect().topLeft()+QPoint(200,200));
		int W0=1400,H0=600;
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
		QString data_path=CLP.named_parameters.value("data").toString();
		QString labels_path=CLP.named_parameters.value("labels").toString();
        Mda data0; data0.read(data_path);
        W->setData(data0);
        if (~labels_path.isEmpty()) {
            Mda labels0; labels0.read(labels_path);
            int NN=labels0.totalSize();
			QList<int> labels; for (int i=0; i<NN; i++) labels << labels0.get(i);
            W->setLabels(labels);
        }
        W->resize(1000,500);
        W->show();
    }
	else if (mode=="spikespy") {
		printf("spikespy...\n");
		QString signal_path=CLP.named_parameters["signal"].toString();
		QString firings_path=CLP.named_parameters["firings"].toString();
        double samplerate=CLP.named_parameters["samplerate"].toDouble();
		SSTimeSeriesWidget *W=new SSTimeSeriesWidget;
		SSTimeSeriesView *V=new SSTimeSeriesView;
        V->setSamplingFrequency(samplerate);
		DiskArrayModel *DAM=new DiskArrayModel;
		DAM->setPath(signal_path);
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

	int ret=a.exec();

	printf("Number of files open: %d, number of unfreed mallocs: %d, number of unfreed megabytes: %g\n",jnumfilesopen(),jmalloccount(),(int)jbytesallocated()*1.0/1000000);

	return ret;
}
