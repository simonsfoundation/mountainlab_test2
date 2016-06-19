/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#include "mvviewagent.h"
#include <QDebug>

struct TimeseriesStruct {
    QString name;
    DiskReadMda data;
};

class MVViewAgentPrivate {
public:
    MVViewAgent* q;
    ClusterMerge m_cluster_merge;
    QMap<int, QJsonObject> m_cluster_attributes;
    MVEvent m_current_event;
    int m_current_cluster;
    QList<int> m_selected_clusters;
    double m_current_timepoint;
    MVRange m_current_time_range;
    QList<QColor> m_cluster_colors;
    QList<QColor> m_channel_colors;
    QMap<QString, TimeseriesStruct> m_timeseries;
    QString m_current_timeseries_name;
    DiskReadMda m_firings;
    MVEventFilter m_event_filter;
    double m_sample_rate;
    QMap<QString, QVariant> m_options;
    QString m_mlproxy_url;
    QMap<QString, QColor> m_colors;
};

MVViewAgent::MVViewAgent()
{
    d = new MVViewAgentPrivate;
    d->q = this;
    d->m_current_cluster = 0;
    d->m_current_timepoint = 0;

    // default colors
    d->m_colors["background"] = QColor(240, 240, 240);
    d->m_colors["frame1"] = QColor(245, 245, 245);
    d->m_colors["info_text"] = QColor(80, 80, 80);
    d->m_colors["view_background"] = QColor(245, 245, 245);
    d->m_colors["view_background_highlighted"] = QColor(210, 230, 250);
    d->m_colors["view_background_selected"] = QColor(220, 240, 250);
    d->m_colors["view_background_hovered"] = QColor(240, 245, 240);
    d->m_colors["view_frame_selected"] = QColor(50, 20, 20);
    d->m_colors["divider_line"] = QColor(255, 100, 150);
    d->m_colors["calculation-in-progress"] = QColor(130, 130, 140, 50);
}

MVViewAgent::~MVViewAgent()
{
    delete d;
}

void MVViewAgent::clear()
{
    d->m_cluster_merge.clear();
    d->m_cluster_attributes.clear();
    d->m_timeseries.clear();
    d->m_firings = DiskReadMda();
    d->m_options.clear();
}

QMap<int, QJsonObject> MVViewAgent::clusterAttributes() const
{
    return d->m_cluster_attributes;
}

MVEvent MVViewAgent::currentEvent() const
{
    return d->m_current_event;
}

int MVViewAgent::currentCluster() const
{
    return d->m_current_cluster;
}

QList<int> MVViewAgent::selectedClusters() const
{
    return d->m_selected_clusters;
}

double MVViewAgent::currentTimepoint() const
{
    return d->m_current_timepoint;
}

MVRange MVViewAgent::currentTimeRange() const
{
    return d->m_current_time_range;
}

QList<QColor> MVViewAgent::channelColors() const
{
    return d->m_channel_colors;
}

QList<QColor> MVViewAgent::clusterColors() const
{
    return d->m_cluster_colors;
}

DiskReadMda MVViewAgent::currentTimeseries()
{
    return d->m_timeseries.value(d->m_current_timeseries_name).data;
}

QString MVViewAgent::currentTimeseriesName()
{
    return d->m_current_timeseries_name;
}

QColor MVViewAgent::clusterColor(int k) const
{
    if (k <= 0)
        return Qt::black;
    if (d->m_cluster_colors.isEmpty())
        return Qt::black;
    return d->m_cluster_colors[(k - 1) % d->m_cluster_colors.count()];
}

QColor MVViewAgent::channelColor(int m) const
{
    if (m < 0)
        return Qt::black;
    if (d->m_channel_colors.isEmpty())
        return Qt::black;
    return d->m_channel_colors[m % d->m_channel_colors.count()];
}

QColor MVViewAgent::color(QString name, QColor default_color) const
{
    return d->m_colors.value(name, default_color);
}

QMap<QString, QColor> MVViewAgent::colors() const
{
    return d->m_colors;
}

QStringList MVViewAgent::timeseriesNames() const
{
    return d->m_timeseries.keys();
}

void MVViewAgent::addTimeseries(QString name, DiskReadMda timeseries)
{
    TimeseriesStruct X;
    X.data = timeseries;
    X.name = name;
    d->m_timeseries[name] = X;
    emit this->timeseriesNamesChanged();
    if (name == d->m_current_timeseries_name)
        emit this->currentTimeseriesChanged();
}

DiskReadMda MVViewAgent::firings()
{
    return d->m_firings;
}

double MVViewAgent::sampleRate() const
{
    return d->m_sample_rate;
}

QVariant MVViewAgent::option(QString name, QVariant default_val)
{
    return d->m_options.value(name, default_val);
}

void MVViewAgent::setCurrentTimeseriesName(QString name)
{
    if (d->m_current_timeseries_name == name)
        return;
    d->m_current_timeseries_name = name;
    emit this->currentTimeseriesChanged();
}

void MVViewAgent::setFirings(const DiskReadMda& F)
{
    d->m_firings = F;
    emit firingsChanged();
    emit filteredFiringsChanged();
}

MVEventFilter MVViewAgent::eventFilter()
{
    return d->m_event_filter;
}

void MVViewAgent::setEventFilter(const MVEventFilter& EF)
{
    if (d->m_event_filter == EF)
        return;
    d->m_event_filter = EF;
    emit eventFilterChanged();
    emit filteredFiringsChanged();
}

void MVViewAgent::setSampleRate(double sample_rate)
{
    d->m_sample_rate = sample_rate;
}

QString MVViewAgent::mlProxyUrl() const
{
    return d->m_mlproxy_url;
}

void MVViewAgent::setMLProxyUrl(QString url)
{
    d->m_mlproxy_url = url;
}

ClusterMerge MVViewAgent::clusterMerge() const
{
    return d->m_cluster_merge;
}

void MVViewAgent::setClusterMerge(const ClusterMerge& CM)
{
    if (d->m_cluster_merge == CM)
        return;
    d->m_cluster_merge = CM;
    emit this->clusterMergeChanged();
}

void MVViewAgent::setClusterAttributes(const QMap<int, QJsonObject>& A)
{
    if (d->m_cluster_attributes == A)
        return;
    d->m_cluster_attributes = A;
    emit this->clusterAttributesChanged();
}

void MVViewAgent::setCurrentEvent(const MVEvent& evt)
{
    if (evt == d->m_current_event)
        return;
    d->m_current_event = evt;
    emit currentEventChanged();
    this->setCurrentTimepoint(evt.time);
}

void MVViewAgent::setCurrentCluster(int k)
{
    if (k == d->m_current_cluster)
        return;
    d->m_current_cluster = k;

    QList<int> tmp = selectedClusters();
    if (!tmp.contains(k)) {
        tmp << k;
        this->setSelectedClusters(tmp); //think about this
    }
    emit currentClusterChanged();
}

void MVViewAgent::setSelectedClusters(const QList<int>& ks)
{
    QList<int> ks2 = QList<int>::fromSet(ks.toSet()); //remove duplicates and -1
    ks2.removeAll(-1);
    qSort(ks2);
    if (d->m_selected_clusters == ks2)
        return;
    d->m_selected_clusters = ks2;
    if (!d->m_selected_clusters.contains(d->m_current_cluster)) {
        this->setCurrentCluster(-1);
    }
    emit selectedClustersChanged();
}

void MVViewAgent::setCurrentTimepoint(double tp)
{
    if (d->m_current_timepoint == tp)
        return;
    d->m_current_timepoint = tp;
    emit currentTimepointChanged();
}

void MVViewAgent::setCurrentTimeRange(const MVRange& range_in)
{
    MVRange range = range_in;
    if (range.min < 0) {
        range = range + (0 - range.min);
    }
    if (range.max >= this->currentTimeseries().N2()) {
        range.max = this->currentTimeseries().N2() - 1;
    }
    if (range.max - range.min < 30) { //don't allow range to be too small
        range.max = range.min + 30;
    }
    if (d->m_current_time_range == range)
        return;
    d->m_current_time_range = range;
    emit currentTimeRangeChanged();
}

void MVViewAgent::setClusterColors(const QList<QColor>& colors)
{
    d->m_cluster_colors = colors;
}

void MVViewAgent::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
}

void MVViewAgent::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
}

void MVViewAgent::setOption(QString name, QVariant value)
{
    if (d->m_options[name] == value)
        return;
    d->m_options[name] = value;
    emit optionChanged(name);
}

void MVViewAgent::copySettingsFrom(MVViewAgent* other)
{
    this->setChannelColors(other->channelColors());
    this->setClusterColors(other->clusterColors());
    this->setColors(other->colors());
    this->setMLProxyUrl(other->mlProxyUrl());
    this->setSampleRate(other->sampleRate());
    this->d->m_options = other->d->m_options;
}

void MVViewAgent::clickCluster(int k, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier) {
        if (k < 0)
            return;
        if (d->m_selected_clusters.contains(k)) {
            QList<int> tmp = d->m_selected_clusters;
            tmp.removeAll(k);
            this->setSelectedClusters(tmp);
        }
        else {
            if (k >= 0) {
                QList<int> tmp = d->m_selected_clusters;
                tmp << k;
                this->setSelectedClusters(tmp);
            }
        }
    }
    else {
        this->setSelectedClusters(QList<int>());
        this->setCurrentCluster(k);
    }
}

bool MVRange::operator==(const MVRange& other) const
{
    return ((other.min == min) && (other.max == max));
}

MVRange MVRange::operator+(double offset)
{
    return MVRange(min + offset, max + offset);
}

MVRange MVRange::operator*(double scale)
{
    double center = (min + max) / 2;
    double span = (max - min);
    return MVRange(center - span / 2 * scale, center + span / 2 * scale);
}
