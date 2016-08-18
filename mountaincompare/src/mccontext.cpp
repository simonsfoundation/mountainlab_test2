#include "mccontext.h"

#include <QMutex>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "mlcommon.h"

class ConfusionMatrixCalculator {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings1;
    DiskReadMda firings2;

    //output
    DiskReadMda confusion_matrix;
    DiskReadMda optimal_assignments;
    DiskReadMda event_correspondence;

    virtual void compute();

    bool loaded_from_static_output = false;
    QJsonObject exportStaticOutput();
    void loadStaticOutput(const QJsonObject& X);
};


class MCContextPrivate {
public:
    MCContext *q;
    DiskReadMda m_firings2;
    int m_current_cluster2=-1;
    QSet<int> m_selected_clusters2;

    QMutex m_mutex;
    bool m_confusion_matrix_computed=false;
    DiskReadMda m_confusion_matrix;
    DiskReadMda m_optimal_assignments;
    DiskReadMda m_event_correspondence;

    ConfusionMatrixCalculator m_calculator;
};

MCContext::MCContext()
{
    d=new MCContextPrivate;
    d->q=this;
}

MCContext::~MCContext()
{
    delete d;
}


void MCContext::computeConfusionMatrix()
{
    {
        QMutexLocker locker(&d->m_mutex);
        if (d->m_confusion_matrix_computed) return;
    }

    d->m_calculator.mlproxy_url = this->mlProxyUrl();
    d->m_calculator.firings1 = this->firings();
    d->m_calculator.firings2 = this->firings2();
    d->m_calculator.compute();
    d->m_confusion_matrix=d->m_calculator.confusion_matrix;
    d->m_optimal_assignments=d->m_calculator.optimal_assignments;
    d->m_event_correspondence=d->m_calculator.event_correspondence;

    {
        QMutexLocker locker(&d->m_mutex);
        d->m_confusion_matrix_computed=true;
    }
}

DiskReadMda MCContext::confusionMatrix() const
{
    return d->m_confusion_matrix;
}

DiskReadMda MCContext::optimalAssignments() const
{
    return d->m_optimal_assignments;
}

DiskReadMda MCContext::eventCorrespondence() const
{
    return d->m_event_correspondence;
}

DiskReadMda MCContext::firings2()
{
    return d->m_firings2;
}

int MCContext::currentCluster2() const
{
    return d->m_current_cluster2;
}

QList<int> MCContext::selectedClusters2() const
{
    QList<int> ret=d->m_selected_clusters2.toList();
    qSort(ret);
    return ret;
}

void MCContext::setFirings2(const DiskReadMda &F)
{
    d->m_firings2=F;
    emit firings2Changed();
}

void MCContext::setCurrentCluster2(int k)
{
    if (d->m_current_cluster2==k) return;
    d->m_current_cluster2=k;
    emit currentCluster2Changed();
}

void MCContext::setSelectedClusters2(const QList<int> &list)
{
    if (list.toSet()==d->m_selected_clusters2) return;
    d->m_selected_clusters2=list.toSet();
    emit selectedClusters2Changed();
}

void MCContext::clickCluster2(int k, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier) {
        if (k < 0)
            return;
        if (d->m_selected_clusters2.contains(k)) {
            QList<int> tmp = d->m_selected_clusters2.toList();
            tmp.removeAll(k);
            this->setSelectedClusters2(tmp);
        }
        else {
            if (k >= 0) {
                QList<int> tmp = d->m_selected_clusters2.toList();
                tmp << k;
                this->setSelectedClusters2(tmp);
            }
        }
    }
    else {
        //this->setSelectedClusterPairs(QSet<ClusterPair>());
        this->setSelectedClusters2(QList<int>());
        this->setCurrentCluster2(k);
    }
}

void ConfusionMatrixCalculator::compute()
{
    TaskProgress task(TaskProgress::Calculate, "Compute confusion matrix");
    if (this->loaded_from_static_output) {
        task.log("Loaded from static output");
        return;
    }

    QTime timer;
    timer.start();
    task.setProgress(0.1);

    MountainProcessRunner MPR;
    MPR.setProcessorName("confusion_matrix");

    QMap<QString, QVariant> params;
    params["firings1"] = firings1.makePath();
    params["firings2"] = firings2.makePath();
    params["max_matching_offset"] = 6;
    MPR.setInputParameters(params);
    MPR.setMLProxyUrl(mlproxy_url);

    task.log() << "Firings 1/2 dimensions" << firings1.N1() << firings1.N2() << firings2.N1() << firings2.N2();

    QString output_path = MPR.makeOutputFilePath("output");
    QString optimal_assignments_path = MPR.makeOutputFilePath("optimal_assignments");
    QString event_correspondence_path = MPR.makeOutputFilePath("event_correspondence");

    MPR.runProcess();

    if (MLUtil::threadInterruptRequested()) {
        task.error(QString("Halted while running process."));
        return;
    }

    confusion_matrix.setPath(output_path);
    event_correspondence.setPath(event_correspondence_path);

    optimal_assignments.setPath(optimal_assignments_path);
    /*
    {
        DiskReadMda tmp(optimal_assignments_path);
        for (int i=0; i<tmp.totalSize(); i++) {
            optimal_assignments << tmp.value(i);
        }
    }
    */

    task.log() << "Output path:" << output_path;
    task.log() << "Optimal assignments path:" << optimal_assignments_path;
    task.log() << "Confusion matrix dimensions:" << confusion_matrix.N1() << confusion_matrix.N2();
    task.log() << "Event correspondence dimensions:" << event_correspondence.N1() << event_correspondence.N2();
}
