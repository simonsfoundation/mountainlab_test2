/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#ifndef MVCONTEXT_H
#define MVCONTEXT_H

#include "clustermerge.h"

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include "mvutils.h"

class MVContext;

struct MVRange {
    MVRange(double min0 = 0, double max0 = 1)
    {
        min = min0;
        max = max0;
    }
    bool operator==(const MVRange& other) const;
    MVRange operator+(double offset);
    MVRange operator*(double scale);
    double range() const { return max - min; }
    double min, max;
};

class ClusterVisibilityRule {
public:
    ClusterVisibilityRule();
    ClusterVisibilityRule(const ClusterVisibilityRule& other);
    virtual ~ClusterVisibilityRule();
    void operator=(const ClusterVisibilityRule& other);
    bool operator==(const ClusterVisibilityRule& other) const;
    bool isVisible(const MVContext* context, int cluster_num) const;
    QJsonObject toJsonObject() const;
    static ClusterVisibilityRule fromJsonObject(const QJsonObject& X);

    QSet<QString> view_tags;
    bool view_all_tagged = true;
    bool view_all_untagged = true;
    bool hide_rejected = true;

    bool use_subset = false;
    QSet<int> subset;

private:
    void copy_from(const ClusterVisibilityRule& other);
};

struct ClusterPair {
    ClusterPair(int k1 = 0, int k2 = 0);
    ClusterPair(const ClusterPair& other);

    void set(int k1, int k2);
    int kmin() const;
    int kmax() const;
    void operator=(const ClusterPair& other);
    bool operator==(const ClusterPair& other) const;
    bool operator<(const ClusterPair& other) const;
    QString toString() const;
    static ClusterPair fromString(const QString& str);

private:
    int m_kmin = 0, m_kmax = 0;
};
uint qHash(const ClusterPair& pair);

struct ElectrodeGeometry {
    QList<QVector<double> > coordinates;
    QJsonObject toJsonObject() const;
    bool operator==(const ElectrodeGeometry& other);
    static ElectrodeGeometry fromJsonObject(const QJsonObject& obj);
    static ElectrodeGeometry loadFromGeomFile(const QString& path);
};

#include "mvmisc.h"

class MVContextPrivate;
class MVContext : public QObject {
    Q_OBJECT
public:
    friend class MVContextPrivate;
    MVContext();
    virtual ~MVContext();

    void clear();
    void setFromMVFileObject(QJsonObject obj);
    void setFromMV2FileObject(QJsonObject obj);
    QJsonObject toMVFileObject() const;
    QJsonObject toMV2FileObject() const;

    /////////////////////////////////////////////////
    ClusterMerge clusterMerge() const;
    bool viewMerged() const;
    void setViewMerged(bool val);

    /////////////////////////////////////////////////
    QJsonObject clusterAttributes(int num) const;
    QList<int> clusterAttributesKeys() const;
    void setClusterAttributes(int num, const QJsonObject& obj);
    QSet<QString> clusterTags(int num) const; //part of attributes
    QList<QString> clusterTagsList(int num) const;
    QSet<QString> allClusterTags() const;
    void setClusterTags(int num, const QSet<QString>& tags); //part of attributes

    /////////////////////////////////////////////////
    QList<int> clusterOrder(int max_K = 0) const; //max_K is used in case the cluster order is empty, in which case it will return 1,2,...,max_K
    QString clusterOrderScoresName() const;
    QList<double> clusterOrderScores() const;
    void setClusterOrderScores(QString scores_name, const QList<double>& scores);

    /////////////////////////////////////////////////
    QJsonObject clusterPairAttributes(const ClusterPair& pair) const;
    QList<ClusterPair> clusterPairAttributesKeys() const;
    void setClusterPairAttributes(const ClusterPair& pair, const QJsonObject& obj);
    QSet<QString> clusterPairTags(const ClusterPair& pair) const; //part of attributes
    QList<QString> clusterPairTagsList(const ClusterPair& pair) const;
    QSet<QString> allClusterPairTags() const;
    void setClusterPairTags(const ClusterPair& pair, const QSet<QString>& tags); //part of attributes

    /////////////////////////////////////////////////
    ClusterVisibilityRule clusterVisibilityRule() const;
    QList<int> visibleClusters(int Kmax) const;
    bool clusterIsVisible(int k) const;
    void setClusterVisibilityRule(const ClusterVisibilityRule& rule);

    /////////////////////////////////////////////////
    QList<int> visibleChannels() const; //1-based indexing
    void setVisibleChannels(const QList<int>& channels);

    /////////////////////////////////////////////////
    ElectrodeGeometry electrodeGeometry() const;
    void setElectrodeGeometry(const ElectrodeGeometry& geom);

    /////////////////////////////////////////////////
    QSet<int> clustersSubset() const;
    void setClustersSubset(const QSet<int>& clusters_subset);

    /////////////////////////////////////////////////
    MVEvent currentEvent() const;
    int currentCluster() const;
    QList<int> selectedClusters() const;
    double currentTimepoint() const;
    MVRange currentTimeRange() const;
    void setCurrentEvent(const MVEvent& evt);
    void setCurrentCluster(int k);
    void setSelectedClusters(const QList<int>& ks);
    void setCurrentTimepoint(double tp);
    void setCurrentTimeRange(const MVRange& range);
    void clickCluster(int k, Qt::KeyboardModifiers modifiers);

    /////////////////////////////////////////////////
    QSet<ClusterPair> selectedClusterPairs() const;
    void setSelectedClusterPairs(const QSet<ClusterPair>& pairs);
    void clickClusterPair(const ClusterPair& pair, Qt::KeyboardModifiers modifiers);

    /////////////////////////////////////////////////
    QColor clusterColor(int k) const;
    QColor channelColor(int m) const;
    QColor color(QString name, QColor default_color = Qt::black) const;
    QMap<QString, QColor> colors() const;
    QList<QColor> channelColors() const;
    QList<QColor> clusterColors() const;
    void setClusterColors(const QList<QColor>& colors);
    void setChannelColors(const QList<QColor>& colors);
    void setColors(const QMap<QString, QColor>& colors);

    /////////////////////////////////////////////////
    DiskReadMda currentTimeseries() const;
    DiskReadMda timeseries(QString name) const;
    QString currentTimeseriesName() const;
    QStringList timeseriesNames() const;
    void addTimeseries(QString name, DiskReadMda timeseries);
    void setCurrentTimeseriesName(QString name);

    /////////////////////////////////////////////////
    DiskReadMda firings();
    void setFirings(const DiskReadMda& F);

    /////////////////////////////////////////////////
    // these should be set once at beginning
    double sampleRate() const;
    void setSampleRate(double sample_rate);
    QString mlProxyUrl() const;
    void setMLProxyUrl(QString url);

    /////////////////////////////////////////////////
    QVariant option(QString name, QVariant default_val = QVariant()) const;
    void setOption(QString name, QVariant value);
    void onOptionChanged(QString name, const QObject* receiver, const char* member, Qt::ConnectionType type = Qt::DirectConnection);

    /////////////////////////////////////////////////
    void copySettingsFrom(MVContext* other);

    /////////////////////////////////////////////////
    bool createAllPrvFiles(QStringList& paths_ret);

signals:
    void currentTimeseriesChanged();
    void timeseriesNamesChanged();
    void firingsChanged();
    void filteredFiringsChanged();
    void clusterMergeChanged();
    void clusterAttributesChanged(int cluster_number);
    void currentEventChanged();
    void currentClusterChanged();
    void selectedClustersChanged();
    void currentTimepointChanged();
    void currentTimeRangeChanged();
    void optionChanged(QString name);
    void clusterVisibilityChanged();
    void selectedClusterPairsChanged();
    void clusterPairAttributesChanged(ClusterPair pair);
    void electrodeGeometryChanged();
    void viewMergedChanged();
    void visibleChannelsChanged();
    void clusterOrderChanged();

private slots:
    void slot_option_changed(QString name);
    void slot_firings_subset_calculator_finished();

private:
    MVContextPrivate* d;
};

#endif // MVCONTEXT_H
