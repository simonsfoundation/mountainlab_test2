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
#include "commandlineparams.h"
#include "diskarraymodel_new.h"
#include "histogramview.h"
#include "mvlabelcomparewidget.h"
#include "mvoverview2widget.h"
#include "sstimeserieswidget.h"
#include "sstimeseriesview.h"
#include "mvclusterwidget.h"
#include "closemehandler.h"
#include "remotereadmda.h"
#include "taskprogress.h"
#include "mvtimeseriesview.h" //for unit test

#include <QRunnable>
#include <QThreadPool>
#include <QtConcurrentRun>

/// TODO option to turn on/off 8-bit quantization per view
/// TODO update doc
/// TODO ctrl+a to select all clusters
/// TODO event filter to be computed on client

class TaskProgressViewThread : public QRunnable {
public:
    TaskProgressViewThread(int idx)
        : QRunnable()
        , m_idx(idx)
    {
        setAutoDelete(true);
    }
    void run()
    {
        qsrand(QDateTime::currentDateTime().currentMSecsSinceEpoch());
        QThread::msleep(qrand() % 1000);
        TaskProgress TP1(QString("Test task %1").arg(m_idx));
        if (m_idx % 3 == 0)
            TP1.addTag(TaskProgress::Download);
        else if (m_idx % 3 == 1)
            TP1.addTag(TaskProgress::Calculate);
        TP1.setDescription("The description of the task. This should complete on destruct.");
        for (int i = 0; i <= 100; ++i) {
            TP1.setProgress(i * 1.0 / 100.0);
            TP1.setLabel(QString("Test task %1 (%2)").arg(m_idx).arg(i * 1.0 / 100.0));
            TP1.log(QString("Log #%1").arg(i + 1));
            int rand = 1 + (qrand() % 10);
            QThread::msleep(100 * rand);
        }
    }

private:
    int m_idx;
};

void test_taskprogressview()
{
    int num_jobs = 30; //can increase to test a large number of jobs
    qsrand(QDateTime::currentDateTime().currentMSecsSinceEpoch());
    for (int i = 0; i < num_jobs; ++i) {
        QThreadPool::globalInstance()->releaseThread();
        QThreadPool::globalInstance()->start(new TaskProgressViewThread(i + 1));
        QThread::msleep(qrand() % 10);
    }
    QApplication::instance()->exec();
}
////////////////////////////////////////////////////////////////////////////////

//void run_export_instructions(MVOverview2Widget* W, const QStringList& instructions);

/// TODO provide mountainview usage information
/// TODO auto correlograms for selected clusters
/// TODO figure out what to do when #channels and/or #clusters is huge
/// TODO 0.9.1 -- make sure to handle merging with other views, such as clips etc. Make elegant way

QColor brighten(QColor col, int amount);

#include "multiscaletimeseries.h"
#include "spikespywidget.h"
#include "taskprogressview.h"
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    // make sure task progress monitor is instantiated in the main thread
    TaskManager::TaskProgressMonitor* monitor = TaskManager::TaskProgressMonitor::globalInstance();
    Q_UNUSED(monitor);
    CloseMeHandler::start();

    setbuf(stdout, 0);

    /// Witold I don't want to do this here! It should be in the taskprogress.h. What can I do?
    qRegisterMetaType<TaskInfo>();

    CLParams CLP = commandlineparams(argc, argv);

    QString mv_fname;
    if (CLP.unnamed_parameters.value(0).endsWith(".mv")) {
        mv_fname = CLP.unnamed_parameters.value(0);
    }

    QList<QColor> channel_colors;
    QStringList color_strings;
    color_strings
        << "#282828"
        << "#402020"
        << "#204020"
        << "#202070";
    for (int i = 0; i < color_strings.count(); i++)
        channel_colors << QColor(brighten(color_strings[i], 80));

    if (CLP.unnamed_parameters.value(0) == "unit_test") {
        QString arg2 = CLP.unnamed_parameters.value(1);
        if (arg2 == "remotereadmda") {
            unit_test_remote_read_mda();
            return 0;
        }
        else if (arg2 == "remotereadmda2") {
            QString arg3 = CLP.unnamed_parameters.value(2, "http://localhost:8000/firings.mda");
            unit_test_remote_read_mda_2(arg3);
            return 0;
        }
        else if (arg2 == "taskprogressview") {
            MVOverview2Widget* W = new MVOverview2Widget(new MVViewAgent); //not that the view agent does not get deleted. :(
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
            test_taskprogressview();
            qWarning() << "No such unit test: " + arg2;
            return 0;
        }
        else if (arg2 == "multiscaletimeseries") {
            MultiScaleTimeSeries::unit_test(3, 10);
            return 0;
        }
        else if (arg2 == "mvtimeseriesview") {
            MVTimeSeriesView::unit_test();
        }
    }

    QString mode = CLP.named_parameters.value("mode", "overview2").toString();
    if (mode == "overview2") {
        printf("overview2...\n");
        QString raw_path = CLP.named_parameters["raw"].toString();
        QString pre_path = CLP.named_parameters["pre"].toString();
        QString filt_path = CLP.named_parameters["filt"].toString();
        QString firings_path = CLP.named_parameters["firings"].toString();
        double samplerate = CLP.named_parameters.value("samplerate", 20000).toDouble();
        QString epochs_path = CLP.named_parameters["epochs"].toString();
        QString window_title = CLP.named_parameters["window_title"].toString();
        MVOverview2Widget* W = new MVOverview2Widget(new MVViewAgent); //not that the view agent does not get deleted. :(
        W->setChannelColors(channel_colors);
        W->setMLProxyUrl(CLP.named_parameters.value("mlproxy_url", "").toString());
        {
            W->setWindowTitle(window_title);
            W->show();
            //W->move(QApplication::desktop()->screen()->rect().topLeft() + QPoint(200, 200));

            int W0 = 1400, H0 = 1000;
            QRect geom = QApplication::desktop()->geometry();
            if ((geom.width() - 100 < W0) || (geom.height() - 100 < H0)) {
                //W->showMaximized();
                W->resize(geom.width() - 100, geom.height() - 100);
            }
            else {
                W->resize(W0, H0);
            }

            W->move(QApplication::desktop()->screen()->rect().bottomRight() - QPoint(W0, H0));

            qApp->processEvents();
        }

        if (!mv_fname.isEmpty()) {
            W->loadMVFile(mv_fname);
        }

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
        if (!firings_path.isEmpty()) {
            W->setFiringsPath(firings_path);
        }
        if (samplerate) {
            W->setSampleRate(samplerate);
        }

        W->setDefaultInitialization();
    }
    else if (mode == "spikespy") {
        printf("spikespy...\n");
        QStringList timeseries_paths = CLP.named_parameters["timeseries"].toString().split(",");
        QStringList firings_paths = CLP.named_parameters["firings"].toString().split(",");
        double samplerate = CLP.named_parameters["samplerate"].toDouble();

        SpikeSpyWidget* W = new SpikeSpyWidget(new MVViewAgent); //not that the view agent will not get deleted. :(
        W->setChannelColors(channel_colors);
        W->setSampleRate(samplerate);
        for (int i = 0; i < timeseries_paths.count(); i++) {
            QString tsp = timeseries_paths.value(i);
            if (tsp.isEmpty())
                tsp = timeseries_paths.value(0);
            QString fp = firings_paths.value(i);
            if (fp.isEmpty())
                fp = firings_paths.value(0);
            SpikeSpyViewData view;
            view.timeseries = DiskReadMda(tsp);
            view.firings = DiskReadMda(fp);
            W->addView(view);
        }
        W->show();
        W->move(QApplication::desktop()->screen()->rect().topLeft() + QPoint(200, 200));
        W->resize(1800, 1200);

        /*
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
        */
    }

    int ret = a.exec();

    printf("Number of files open: %d, number of unfreed mallocs: %d, number of unfreed megabytes: %g\n", jnumfilesopen(), jmalloccount(), (int)jbytesallocated() * 1.0 / 1000000);

    return ret;
}

QColor brighten(QColor col, int amount)
{
    int r = col.red() + amount;
    int g = col.green() + amount;
    int b = col.blue() + amount;
    if (r > 255)
        r = 255;
    if (r < 0)
        r = 0;
    if (g > 255)
        g = 255;
    if (g < 0)
        g = 0;
    if (b > 255)
        b = 255;
    if (b < 0)
        b = 0;
    return QColor(r, g, b, col.alpha());
}
