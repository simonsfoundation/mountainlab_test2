#include "mccontext.h"

#include <QMutex>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "mlcommon.h"

class MergeFiringsCalculator {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings1;
    DiskReadMda firings2;

    //output
    DiskReadMda firings_merged;
    DiskReadMda confusion_matrix;
    DiskReadMda optimal_label_map;

    virtual void compute();
};

class MCContextPrivate {
public:
    MCContext* q;
    DiskReadMda m_firings2;
    int m_current_cluster2 = -1;
    QSet<int> m_selected_clusters2;

    QMutex m_mutex;
    bool m_merged_firings_computed = false;
    DiskReadMda m_confusion_matrix;
    DiskReadMda m_optimal_label_map;
    DiskReadMda m_firings_merged;

    MergeFiringsCalculator m_calculator;
};

MCContext::MCContext()
{
    d = new MCContextPrivate;
    d->q = this;
}

MCContext::~MCContext()
{
    delete d;
}

void MCContext::computeMergedFirings()
{
    {
        QMutexLocker locker(&d->m_mutex);
        if (d->m_merged_firings_computed)
            return;
    }

    d->m_calculator.mlproxy_url = this->mlProxyUrl();
    d->m_calculator.firings1 = this->firings();
    d->m_calculator.firings2 = this->firings2();
    d->m_calculator.compute();
    d->m_confusion_matrix = d->m_calculator.confusion_matrix;
    d->m_optimal_label_map = d->m_calculator.optimal_label_map;
    d->m_firings_merged = d->m_calculator.firings_merged;

    {
        QMutexLocker locker(&d->m_mutex);
        d->m_merged_firings_computed = true;
    }
}

Mda MCContext::confusionMatrix() const
{
    Mda ret;
    d->m_confusion_matrix.readChunk(ret, 0, 0, d->m_confusion_matrix.N1(), d->m_confusion_matrix.N2());
    return ret;
}

QList<int> MCContext::optimalLabelMap() const
{
    QList<int> ret;
    for (int i = 0; i < d->m_optimal_label_map.totalSize(); i++) {
        ret << d->m_optimal_label_map.value(i);
    }
    return ret;
}

DiskReadMda MCContext::firingsMerged() const
{
    return d->m_firings_merged;
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
    QList<int> ret = d->m_selected_clusters2.toList();
    qSort(ret);
    return ret;
}

void MCContext::setFirings2(const DiskReadMda& F)
{
    d->m_firings2 = F;
    emit firings2Changed();
}

void MCContext::setCurrentCluster2(int k)
{
    if (d->m_current_cluster2 == k)
        return;
    d->m_current_cluster2 = k;
    emit currentCluster2Changed();
}

void MCContext::setSelectedClusters2(const QList<int>& list)
{
    if (list.toSet() == d->m_selected_clusters2)
        return;
    d->m_selected_clusters2 = list.toSet();
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

void MergeFiringsCalculator::compute()
{
    TaskProgress task(TaskProgress::Calculate, "Compute confusion matrix");
    task.setProgress(0.1);

    MountainProcessRunner MPR;
    MPR.setProcessorName("merge_firings");

    QMap<QString, QVariant> params;
    params["firings1"] = firings1.makePath();
    params["firings2"] = firings2.makePath();
    params["max_matching_offset"] = 4;
    MPR.setInputParameters(params);
    MPR.setMLProxyUrl(mlproxy_url);

    task.log() << "Firings 1/2 dimensions" << firings1.N1() << firings1.N2() << firings2.N1() << firings2.N2();

    QString firings_merged_path = MPR.makeOutputFilePath("firings_merged");
    QString optimal_label_map_path = MPR.makeOutputFilePath("optimal_label_map");
    QString confusion_matrix_path = MPR.makeOutputFilePath("confusion_matrix");

    MPR.runProcess();

    if (MLUtil::threadInterruptRequested()) {
        task.error(QString("Halted while running process."));
        return;
    }

    firings_merged.setPath(firings_merged_path);
    confusion_matrix.setPath(confusion_matrix_path);
    optimal_label_map.setPath(optimal_label_map_path);
}
