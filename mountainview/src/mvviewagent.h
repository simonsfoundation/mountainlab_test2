/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#ifndef MVVIEWAGENT_H
#define MVVIEWAGENT_H

#include "clustermerge.h"

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include "mvutils.h"

class MVViewAgentPrivate;
class MVViewAgent : public QObject {
    Q_OBJECT
public:
    friend class MVViewAgentPrivate;
    MVViewAgent();
    virtual ~MVViewAgent();

    ClusterMerge clusterMerge() const;
    QMap<int, QJsonObject> clusterAttributes() const;
    MVEvent currentEvent() const;
    int currentCluster() const;
    QList<int> selectedClusters() const;

    void setClusterMerge(const ClusterMerge& CM);
    void setClusterAttributes(const QMap<int, QJsonObject>& A);
    void setCurrentEvent(const MVEvent& evt);
    void setCurrentCluster(int k);
    void setSelectedClusters(const QList<int>& ks);

    void clickCluster(int k, Qt::KeyboardModifiers modifiers);

signals:
    void clusterMergeChanged();
    void clusterAttributesChanged();
    void currentEventChanged();
    void currentClusterChanged();
    void selectedClustersChanged();

private:
    MVViewAgentPrivate* d;
};

#endif // MVVIEWAGENT_H
