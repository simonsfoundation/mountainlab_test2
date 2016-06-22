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

/// TODO: (MEDIUM) rename MVViewAgent to MVContext throughout -- for now we use typedef
class MVViewAgent;
typedef MVViewAgent MVContext;

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
    bool isVisible(MVContext* context, int cluster_num) const;

    QSet<QString> invisibility_assessments;
    bool view_merged = true;

private:
    void copy_from(const ClusterVisibilityRule& other);
};

#include "mvmisc.h"

class MVViewAgentPrivate;
class MVViewAgent : public QObject {
    Q_OBJECT
public:
    friend class MVViewAgentPrivate;
    MVViewAgent();
    virtual ~MVViewAgent();

    void clear();

    /////////////////////////////////////////////////
    ClusterMerge clusterMerge() const;
    QMap<int, QJsonObject> clusterAttributes() const;
    void setClusterMerge(const ClusterMerge& CM);
    void setClusterAttributes(const QMap<int, QJsonObject>& A);

    /////////////////////////////////////////////////
    ClusterVisibilityRule visibilityRule();
    void setVisibilityRule(const ClusterVisibilityRule& rule);
    QList<int> visibleClusters(int K);

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
    DiskReadMda currentTimeseries();
    QString currentTimeseriesName();
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

    /////////////////////////////////////////////////
    void copySettingsFrom(MVViewAgent* other);

signals:
    void currentTimeseriesChanged();
    void timeseriesNamesChanged();
    void firingsChanged();
    void eventFilterChanged();
    void filteredFiringsChanged();
    void clusterMergeChanged();
    void clusterAttributesChanged();
    void currentEventChanged();
    void currentClusterChanged();
    void selectedClustersChanged();
    void currentTimepointChanged();
    void currentTimeRangeChanged();
    void optionChanged(QString name);
    void clusterVisibilityChanged();

private:
    MVViewAgentPrivate* d;
};

#endif // MVVIEWAGENT_H
