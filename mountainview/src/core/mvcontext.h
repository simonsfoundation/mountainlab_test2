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
    double min, max;
};

struct MVEventFilter {
    bool use_event_filter = false;
    double min_detectability_score = 0;
    double max_outlier_score = 0;
    bool operator==(const MVEventFilter& other) const
    {
        if (use_event_filter != other.use_event_filter)
            return false;
        if (min_detectability_score != other.min_detectability_score)
            return false;
        if (max_outlier_score != other.max_outlier_score)
            return false;
        return true;
    }
    static MVEventFilter fromJsonObject(QJsonObject obj);
    QJsonObject toJsonObject() const;
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
    bool view_merged = true;

    bool use_subset = false;
    QSet<int> subset;

private:
    void copy_from(const ClusterVisibilityRule& other);
};

struct ClusterPair {
    ClusterPair(int k1_in = 0, int k2_in = 0)
    {
        k1 = k1_in;
        k2 = k2_in;
    }

    int k1, k2;
    bool operator==(const ClusterPair& other) const;
};
uint qHash(const ClusterPair& pair);

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
    QJsonObject toMVFileObject() const;

    /////////////////////////////////////////////////
    ClusterMerge clusterMerge() const;
    QJsonObject clusterAttributes(int num) const;
    QList<int> clusterAttributesKeys() const;
    QSet<QString> clusterTags(int num) const; //part of attributes
    QSet<QString> allClusterTags() const;
    void setClusterMerge(const ClusterMerge& CM);
    void setClusterAttributes(int num, const QJsonObject& obj);
    void setClusterTags(int num, const QSet<QString>& tags); //part of attributes

    /////////////////////////////////////////////////
    ClusterVisibilityRule visibilityRule() const;
    QList<int> visibleClusters(int Kmax) const;
    QList<int> visibleClustersIncludingMerges(int Kmax) const;
    bool clusterIsVisible(int k) const;
    void setVisibilityRule(const ClusterVisibilityRule& rule);

    /////////////////////////////////////////////////
    MVEvent currentEvent() const;
    int currentCluster() const;
    QList<int> selectedClusters() const;
    QList<int> selectedClustersIncludingMerges() const;
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
    QString currentTimeseriesName() const;
    QStringList timeseriesNames() const;
    void addTimeseries(QString name, DiskReadMda timeseries);
    void setCurrentTimeseriesName(QString name);

    /////////////////////////////////////////////////
    DiskReadMda firings();
    void setFirings(const DiskReadMda& F);
    MVEventFilter eventFilter();
    void setEventFilter(const MVEventFilter& EF);

    /////////////////////////////////////////////////
    // these should be set once at beginning
    double sampleRate() const;
    void setSampleRate(double sample_rate);
    QString mlProxyUrl() const;
    void setMLProxyUrl(QString url);

    /////////////////////////////////////////////////
    QVariant option(QString name, QVariant default_val = QVariant());
    void setOption(QString name, QVariant value);
    void onOptionChanged(QString name, const QObject* receiver, const char* member, Qt::ConnectionType type = Qt::DirectConnection);

    /////////////////////////////////////////////////
    void copySettingsFrom(MVContext* other);

signals:
    void currentTimeseriesChanged();
    void timeseriesNamesChanged();
    void firingsChanged();
    void eventFilterChanged();
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

private slots:
    void slot_option_changed(QString name);

private:
    MVContextPrivate* d;
};

#endif // MVCONTEXT_H
