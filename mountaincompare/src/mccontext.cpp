#include "mccontext.h"

class MCContextPrivate {
public:
    MCContext *q;
    DiskReadMda m_firings2;
    int m_current_cluster2=-1;
    QSet<int> m_selected_clusters2;
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
