/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#include "mvviewagent.h"
#include <QDebug>

class MVViewAgentPrivate {
public:
    MVViewAgent* q;
    ClusterMerge m_cluster_merge;
    QMap<int,QJsonObject> m_cluster_attributes;
};

MVViewAgent::MVViewAgent()
{
    d = new MVViewAgentPrivate;
    d->q = this;
}

MVViewAgent::~MVViewAgent()
{
    delete d;
}

QMap<int, QJsonObject> MVViewAgent::clusterAttributes() const
{
    return d->m_cluster_attributes;
}

ClusterMerge MVViewAgent::clusterMerge() const
{
    return d->m_cluster_merge;
}

void MVViewAgent::setClusterMerge(const ClusterMerge &CM)
{
    if (d->m_cluster_merge==CM) return;
    d->m_cluster_merge=CM;
    emit this->clusterMergeChanged();
}

void MVViewAgent::setClusterAttributes(const QMap<int, QJsonObject> &A)
{
    if (d->m_cluster_attributes==A) return;
    d->m_cluster_attributes=A;
    emit this->clusterAttributesChanged();
}

