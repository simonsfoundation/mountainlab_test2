/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#include "mvviewagent.h"
#include <QAction>
#include <QDebug>

struct TimeseriesStruct {
    QString name;
    DiskReadMda data;
};

struct OptionChangedAction {
    QString option_name;
    QAction* action;
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
    double m_sample_rate = 0;
    QMap<QString, QVariant> m_options;
    QString m_mlproxy_url;
    QMap<QString, QColor> m_colors;
    ClusterVisibilityRule m_visibility_rule;
    QList<OptionChangedAction> m_option_changed_actions;
};

MVViewAgent::MVViewAgent()
{
    d = new MVViewAgentPrivate;
    d->q = this;
    d->m_current_cluster = 0;
    d->m_current_timepoint = 0;

    d->m_options["clip_size"] = 100;
    d->m_options["cc_max_dt_msec"] = 100;

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

    QObject::connect(this, SIGNAL(optionChanged(QString)), this, SLOT(slot_option_changed(QString)));
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

QJsonObject cluster_attributes_to_object(const QMap<int, QJsonObject>& map)
{
    QJsonObject X;
    QList<int> keys = map.keys();
    foreach (int key, keys) {
        X[QString("%1").arg(key)] = map[key];
    }
    return X;
}

QMap<int, QJsonObject> object_to_cluster_attributes(QJsonObject X)
{
    QMap<int, QJsonObject> ret;
    QStringList keys = X.keys();
    foreach (QString key, keys) {
        ret[key.toInt()] = X[key].toObject();
    }
    return ret;
}

QJsonObject timeseries_map_to_object(const QMap<QString, TimeseriesStruct>& TT)
{
    QJsonObject ret;
    QStringList keys = TT.keys();
    foreach (QString key, keys) {
        QJsonObject obj;
        obj["data"] = TT[key].data.makePath();
        obj["name"] = TT[key].name;
        ret[key] = obj;
    }
    return ret;
}

QMap<QString, TimeseriesStruct> object_to_timeseries_map(QJsonObject X)
{
    QMap<QString, TimeseriesStruct> ret;
    QStringList keys = X.keys();
    foreach (QString key, keys) {
        QJsonObject obj = X[key].toObject();
        TimeseriesStruct A;
        A.data = DiskReadMda(obj["data"].toString());
        A.name = obj["name"].toString();
        ret[key] = A;
    }
    return ret;
}

QJsonObject MVViewAgent::toMVFileObject() const
{
    QJsonObject X;
    X["cluster_merge"] = d->m_cluster_merge.toJsonObject();
    X["cluster_attributes"] = cluster_attributes_to_object(d->m_cluster_attributes);
    X["timeseries"] = timeseries_map_to_object(d->m_timeseries);
    X["current_timeseries_name"] = d->m_current_timeseries_name;
    X["firings"] = d->m_firings.makePath();
    X["event_filter"] = d->m_event_filter.toJsonObject();
    X["samplerate"] = d->m_sample_rate;
    X["options"] = QJsonObject::fromVariantMap(d->m_options);
    X["mlproxy_url"] = d->m_mlproxy_url;
    return X;
}

void MVViewAgent::setFromMVFileObject(QJsonObject X)
{
    this->clear();
    d->m_cluster_merge.setFromJsonObject(X["cluster_merge"].toObject());
    d->m_cluster_attributes = object_to_cluster_attributes(X["cluster_attributes"].toObject());
    d->m_timeseries = object_to_timeseries_map(X["timeseries"].toObject());
    d->m_current_timeseries_name = X["current_timeseries_name"].toString();
    d->m_firings = DiskReadMda(X["firings"].toString());
    d->m_event_filter = MVEventFilter::fromJsonObject(X["event_filter"].toObject());
    d->m_sample_rate = X["samplerate"].toDouble();
    d->m_options = X["options"].toObject().toVariantMap();
    d->m_mlproxy_url = X["mlproxy_url"].toString();
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
    if ((!d->m_event_filter.use_event_filter) && (!EF.use_event_filter)) {
        //if we are not using event filter, don't bother to change the other parameters
        return;
    }
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

QJsonObject MVViewAgent::clusterAttributes(int num) const
{
    return d->m_cluster_attributes.value(num);
}

QList<int> MVViewAgent::clusterAttributesKeys() const
{
    return d->m_cluster_attributes.keys();
}

QSet<QString> MVViewAgent::clusterTags(int num) const
{
    return jsonarray2stringset(clusterAttributes(num)["tags"].toArray());
}

QSet<QString> MVViewAgent::allClusterTags() const
{
    QSet<QString> ret;
    ret.insert("accepted");
    ret.insert("rejected");
    QList<int> keys = clusterAttributesKeys();
    foreach (int key, keys) {
        QSet<QString> tags0 = clusterTags(key);
        foreach (QString tag, tags0)
            ret.insert(tag);
    }
    return ret;
}

void MVViewAgent::setClusterMerge(const ClusterMerge& CM)
{
    if (d->m_cluster_merge == CM)
        return;
    d->m_cluster_merge = CM;
    emit this->clusterMergeChanged();
    if (!d->m_visibility_rule.view_merged) {
        emit this->clusterVisibilityChanged();
    }
}

void MVViewAgent::setClusterAttributes(int num, const QJsonObject& obj)
{
    if (d->m_cluster_attributes.value(num) == obj)
        return;
    d->m_cluster_attributes[num] = obj;
    emit this->clusterAttributesChanged(num);
    /// TODO (LOW) only emit this if there really was a change
    emit this->clusterVisibilityChanged();
}

void MVViewAgent::setClusterTags(int num, const QSet<QString>& tags)
{
    QJsonObject obj = clusterAttributes(num);
    obj["tags"] = stringset2jsonarray(tags);
    setClusterAttributes(num, obj);
}

ClusterVisibilityRule MVViewAgent::visibilityRule() const
{
    return d->m_visibility_rule;
}

QList<int> MVViewAgent::visibleClusters(int K) const
{
    QList<int> ret;
    for (int k = 1; k <= K; k++) {
        if (this->clusterIsVisible(k)) {
            ret << k;
        }
    }
    return ret;
}

bool MVViewAgent::clusterIsVisible(int k) const
{
    return d->m_visibility_rule.isVisible(this, k);
}

void MVViewAgent::setVisibilityRule(const ClusterVisibilityRule& rule)
{
    if (d->m_visibility_rule == rule)
        return;
    d->m_visibility_rule = rule;
    emit this->clusterVisibilityChanged();
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

void MVViewAgent::onOptionChanged(QString name, const QObject* receiver, const char* member, Qt::ConnectionType type)
{
    QAction* action = new QAction(this);
    connect(action, SIGNAL(triggered(bool)), receiver, member, type);
    OptionChangedAction X;
    X.action = action;
    X.option_name = name;
    d->m_option_changed_actions << X;
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

void MVViewAgent::slot_option_changed(QString name)
{
    for (int i = 0; i < d->m_option_changed_actions.count(); i++) {
        if (d->m_option_changed_actions[i].option_name == name) {
            d->m_option_changed_actions[i].action->trigger();
        }
    }
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

ClusterVisibilityRule::ClusterVisibilityRule()
{
}

ClusterVisibilityRule::ClusterVisibilityRule(const ClusterVisibilityRule& other)
{
    copy_from(other);
}

ClusterVisibilityRule::~ClusterVisibilityRule()
{
}

void ClusterVisibilityRule::operator=(const ClusterVisibilityRule& other)
{
    copy_from(other);
}

bool ClusterVisibilityRule::operator==(const ClusterVisibilityRule& other) const
{
    if (this->view_merged != other.view_merged)
        return false;
    if (this->view_tags != other.view_tags)
        return false;
    if (this->view_all_tagged != other.view_all_tagged)
        return false;
    if (this->view_all_untagged != other.view_all_untagged)
        return false;
    return true;
}

void ClusterVisibilityRule::copy_from(const ClusterVisibilityRule& other)
{
    this->view_merged = other.view_merged;
    this->view_tags = other.view_tags;
    this->view_all_tagged = other.view_all_tagged;
    this->view_all_untagged = other.view_all_untagged;
}

bool ClusterVisibilityRule::isVisible(const MVContext* context, int cluster_num) const
{
    if (!this->view_merged) {
        if (context->clusterMerge().representativeLabel(cluster_num) != cluster_num)
            return false;
    }

    QSet<QString> tags = context->clusterTags(cluster_num);

    if ((view_all_tagged) && (!tags.isEmpty()))
        return true;
    if ((view_all_untagged) && (tags.isEmpty()))
        return true;

    foreach (QString tag, tags) {
        if (view_tags.contains(tag))
            return true;
    }

    return false;
}

MVEventFilter MVEventFilter::fromJsonObject(QJsonObject obj)
{
    MVEventFilter ret;
    ret.use_event_filter = obj["use_event_filter"].toBool();
    ret.min_detectability_score = obj["min_detectability_score"].toDouble();
    ret.max_outlier_score = obj["max_outlier_score"].toDouble();
    return ret;
}

QJsonObject MVEventFilter::toJsonObject() const
{
    QJsonObject obj;
    obj["use_event_filter"] = this->use_event_filter;
    obj["min_detectability_score"] = this->min_detectability_score;
    obj["max_outlier_score"] = this->max_outlier_score;
    return obj;
}
