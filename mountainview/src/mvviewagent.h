/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#ifndef MVVIEWAGENT_H
#define MVVIEWAGENT_H

#include "clustermerge.h"

#include <QObject>


class MVViewAgentPrivate;
class MVViewAgent : public QObject
{
    Q_OBJECT
public:
    friend class MVViewAgentPrivate;
    MVViewAgent();
    virtual ~MVViewAgent();

    ClusterMerge clusterMerge();

    void setClusterMerge(const ClusterMerge &CM);

signals:
    void clusterMergeChanged();
private:
    MVViewAgentPrivate *d;
};

#endif // MVVIEWAGENT_H
