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

/// Witold is there a Qt struct that captures this?
struct MVRange {
    MVRange(double min0 = 0, double max0 = 1)
    {
        min = min0;
        max = max0;
    }
    bool operator==(const MVRange& other);
    MVRange operator+(double offset);
    MVRange operator*(double scale);
    double min, max;
};

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
    double currentTimepoint() const;
    MVRange currentTimeRange() const;
    QColor clusterColor(int k);
    QColor channelColor(int m);
    DiskReadMda timeseries();
    DiskReadMda firings();
    DiskReadMda filteredFirings();
    QVariant option(QString name, QVariant default_val = QVariant());

    void setTimeseries(const DiskReadMda& X);
    void setFirings(const DiskReadMda& F);
    void setClusterMerge(const ClusterMerge& CM);
    void setClusterAttributes(const QMap<int, QJsonObject>& A);
    void setCurrentEvent(const MVEvent& evt);
    void setCurrentCluster(int k);
    void setSelectedClusters(const QList<int>& ks);
    void setCurrentTimepoint(double tp);
    void setCurrentTimeRange(const MVRange& range);
    void setClusterColors(const QList<QColor>& colors);
    void setChannelColors(const QList<QColor>& colors);
    void setOption(QString name, QVariant value);

    void clickCluster(int k, Qt::KeyboardModifiers modifiers);

signals:
    void timeseriesChanged();
    void firingsChanged();
    void clusterMergeChanged();
    void clusterAttributesChanged();
    void currentEventChanged();
    void currentClusterChanged();
    void selectedClustersChanged();
    void currentTimepointChanged();
    void currentTimeRangeChanged();
    void optionChanged(QString name);

private:
    MVViewAgentPrivate* d;
};

#endif // MVVIEWAGENT_H
