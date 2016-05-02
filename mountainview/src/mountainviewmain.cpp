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
#include <QDir>
#include <QImageWriter>
#include "get_command_line_params.h"
#include "diskarraymodel_new.h"
#include "histogramview.h"
#include "mvlabelcomparewidget.h"
#include "mvoverview2widget.h"
#include "sstimeserieswidget.h"
#include "sstimeseriesview.h"
#include "mvclusterwidget.h"
#include "run_mountainview_script.h"
#include "closemehandler.h"
#include "remotereadmda.h"
#include "taskprogress.h"

/*
 * TO DO:
 * Clean up temporary files
 * */

bool download_file(QString url, QString fname)
{
    QStringList args;
    args << "-o" << fname << url;
    int ret = QProcess::execute("/usr/bin/curl", args);
    if (ret != 0) {
        if (QFile::exists(fname)) {
            QFile::remove(fname);
        }
        return false;
    }
    return QFile::exists(fname);
}

void test_histogramview()
{

    int N = 100;
    float values[N];
    for (int i = 0; i < N; i++) {
        values[i] = (qrand() % 10000) * 1.0 / 10000;
    }

    HistogramView* W = new HistogramView;
    W->setData(N, values);
    W->autoSetBins(N / 5);
    W->show();
}

void run_export_instructions(MVOverview2Widget* W, const QStringList& instructions);

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    CloseMeHandler::start();

    /// Witold I don't want to do this here! It should be in the taskprogress.h. What can I do?
    qRegisterMetaType<TaskInfo>();

    CLParams CLP = get_command_line_params(argc, argv);

    if (CLP.unnamed_parameters.value(0).endsWith(".js")) {
        QString script = read_text_file(CLP.unnamed_parameters.value(0));
        return run_mountainview_script(script, CLP.named_parameters);
    }

    if (CLP.unnamed_parameters.value(0) == "unit_test") {
        QString arg2 = CLP.unnamed_parameters.value(1);
        if (arg2 == "remotereadmda") {
            unit_test_remote_read_mda();
        }
        else if (arg2 == "remotereadmda2") {
            QString arg3 = CLP.unnamed_parameters.value(2, "http://localhost:8000/firings.mda");
            unit_test_remote_read_mda_2(arg3);
        }
        else {
            qWarning() << "No such unit test: " + arg2;
        }
        return 0;
    }

    QString mode = CLP.named_parameters.value("mode", "overview2").toString();
    if ((mode == "overview2") || (mode == "export_image") || (mode == "export_images")) {
        printf("overview2...\n");
        QString raw_path = CLP.named_parameters["raw"].toString();
        QString pre_path = CLP.named_parameters["pre"].toString();
        QString filt_path = CLP.named_parameters["filt"].toString();
        QString firings_path = CLP.named_parameters["firings"].toString();
        double samplerate = CLP.named_parameters.value("samplerate", 20000).toDouble();
        QString epochs_path = CLP.named_parameters["epochs"].toString();
        QString window_title = CLP.named_parameters["window_title"].toString();
        MVOverview2Widget* W = new MVOverview2Widget;
        if (mode == "overview2") {
            W->setWindowTitle(window_title);
            W->show();
            W->move(QApplication::desktop()->screen()->rect().topLeft() + QPoint(200, 200));

            int W0 = 1400, H0 = 1000;
            QRect geom = QApplication::desktop()->geometry();
            if ((geom.width() - 100 < W0) || (geom.height() - 100 < H0)) {
                //W->showMaximized();
                W->resize(geom.width() - 100, geom.height() - 100);
            }
            else {
                W->resize(W0, H0);
            }

            qApp->processEvents();
        }

        W->setMscmdServerUrl(CLP.named_parameters.value("mscmdserver_url", "").toString());
        if (!pre_path.isEmpty()) {
            W->addTimeseriesPath("Preprocessed Data", pre_path);
        }
        if (!filt_path.isEmpty()) {
            W->addTimeseriesPath("Filtered Data", filt_path);
        }
        if (!raw_path.isEmpty()) {
            W->addTimeseriesPath("Raw Data", raw_path);
        }

        if (!epochs_path.isEmpty()) {
            QList<Epoch> epochs = read_epochs(epochs_path);
            W->setEpochs(epochs);
        }
        if (window_title.isEmpty())
            window_title = pre_path;
        if (window_title.isEmpty())
            window_title = filt_path;
        if (window_title.isEmpty())
            window_title = raw_path;
        W->setFiringsPath(firings_path);
        W->setSampleRate(samplerate);

        QStringList keys = CLP.named_parameters.keys();
        foreach (QString key, keys) {
            if (key.startsWith("P")) {
                QString pname = key.mid(1);
                QVariant pvalue = CLP.named_parameters[key];
                W->setParameterValue(pname, pvalue);
            }
        }

        W->setDefaultInitialization();

        if (mode == "export_image") {
            QString output_fname = CLP.named_parameters.value("output").toString();
            if (output_fname.isEmpty()) {
                printf("Missing --output parameter.\n");
                return -1;
            }
            QImage img = W->generateImage(CLP.named_parameters);
            QImageWriter IW(output_fname);
            printf("Writing image %s... ", output_fname.toLatin1().data());
            if (!IW.write(img)) {
                printf("Error writing image.\n");
            }
            else {
                printf("OK.\n");
            }

            return 0;
        }
        else if (mode == "export_images") {
            QString instructions_fname = CLP.named_parameters.value("instructions").toString();
            QString instructions = read_text_file(instructions_fname);
            run_export_instructions(W, instructions.split("\n"));
            return 0;
        }
    }
    /*
    else if (mode == "view_clusters") {
        MVClusterWidget* W = new MVClusterWidget;
        QString data_path = CLP.named_parameters.value("data").toString();
        QString labels_path = CLP.named_parameters.value("labels").toString();
        Mda data0;
        data0.read(data_path);
        W->setData(data0);
        if (~labels_path.isEmpty()) {
            Mda labels0;
            labels0.read(labels_path);
            int NN = labels0.totalSize();
            QList<int> labels;
            for (int i = 0; i < NN; i++)
                labels << labels0.get(i);
            W->setLabels(labels);
        }
        W->resize(1000, 500);
        W->show();
    }
    */
    else if (mode == "spikespy") {
        printf("spikespy...\n");
        QString timeseries_path = CLP.named_parameters["timeseries"].toString();
        QString firings_path = CLP.named_parameters["firings"].toString();
        double samplerate = CLP.named_parameters["samplerate"].toDouble();
        SSTimeSeriesWidget* W = new SSTimeSeriesWidget;
        SSTimeSeriesView* V = new SSTimeSeriesView;
        V->setSampleRate(samplerate);
        DiskArrayModel_New* DAM = new DiskArrayModel_New;
        DAM->setPath(timeseries_path);
        V->setData(DAM, true);
        Mda firings;
        firings.read(firings_path);
        QList<long> times, labels;
        for (int i = 0; i < firings.N2(); i++) {
            times << (long)firings.value(1, i);
            labels << (long)firings.value(2, i);
        }
        V->setTimesLabels(times, labels);
        W->addView(V);
        W->show();
        W->move(QApplication::desktop()->screen()->rect().topLeft() + QPoint(200, 200));
        W->resize(1800, 1200);
    }

    int ret = a.exec();

    printf("Number of files open: %d, number of unfreed mallocs: %d, number of unfreed megabytes: %g\n", jnumfilesopen(), jmalloccount(), (int)jbytesallocated() * 1.0 / 1000000);

    return ret;
}

void run_export_instructions(MVOverview2Widget* W, const QStringList& instructions)
{
    foreach (QString instruction, instructions) {
        QStringList vals = instruction.split(QRegExp("\\s"));
        CLParams params = get_command_line_params(vals);
        QString val0 = params.unnamed_parameters.value(0);
        if (val0 == "EXPORT") {
            QImage img = W->generateImage(params.named_parameters);
            QString output_fname = params.named_parameters["output"].toString();
            QImageWriter IW(output_fname);
            printf("Writing image %s... ", output_fname.toLatin1().data());
            if (!IW.write(img)) {
                printf("Error writing image.\n");
            }
            else {
                printf("OK.\n");
            }
        }
        else if (val0 == "SET") {
            QStringList keys = params.named_parameters.keys();
            foreach (QString key, keys) {
                W->setParameterValue(key, params.named_parameters[key]);
            }
        }
    }
}
