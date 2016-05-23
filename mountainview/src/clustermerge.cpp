/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#include "clustermerge.h"

#include <QJsonArray>
#include <QJsonDocument>

struct merge_pair {
    merge_pair(int l1, int l2)
    {
        label1 = l1;
        label2 = l2;
    }
    int label1, label2;
};

class ClusterMergePrivate {
public:
    ClusterMerge* q;
    QList<merge_pair> m_merge_pairs;

    bool is_merged(int label1, int label2);
};

ClusterMerge::ClusterMerge()
{
    d = new ClusterMergePrivate;
    d->q = this;
}

ClusterMerge::ClusterMerge(const ClusterMerge& other)
{
    d = new ClusterMergePrivate;
    d->q = this;

    d->m_merge_pairs = other.d->m_merge_pairs;
}

ClusterMerge::~ClusterMerge()
{
    delete d;
}

void ClusterMerge::operator=(const ClusterMerge& other)
{
    d->m_merge_pairs = other.d->m_merge_pairs;
}

void ClusterMerge::merge(int label1, int label2)
{
    if (d->is_merged(label1, label2))
        return;
    d->m_merge_pairs << merge_pair(label1, label2) << merge_pair(label2, label2);
}

void ClusterMerge::merge(const QSet<int>& labels)
{
    merge(labels.toList());
}

void ClusterMerge::merge(const QList<int>& labels)
{
    for (int i = 0; i < labels.count(); i++) {
        for (int j = 0; j < labels.count(); i++) {
            merge(labels[i], labels[j]);
        }
    }
}

void ClusterMerge::unmerge(int label)
{
    QList<merge_pair> new_pairs;
    for (int i = 0; i < d->m_merge_pairs.count(); i++) {
        if ((d->m_merge_pairs[i].label1 != label) && (d->m_merge_pairs[i].label2 != label)) {
            new_pairs << d->m_merge_pairs[i];
        }
    }
    d->m_merge_pairs = new_pairs;
}

void ClusterMerge::unmerge(const QSet<int>& labels)
{
    unmerge(labels.toList());
}

void ClusterMerge::unmerge(const QList<int>& labels)
{
    for (int i = 0; i < labels.count(); i++) {
        unmerge(labels[i]);
    }
}

int ClusterMerge::representativeLabel(int label) const
{
    int ret = label;
    for (int i = 0; i < d->m_merge_pairs.count(); i++) {
        if (d->m_merge_pairs[i].label1 == label) {
            if (d->m_merge_pairs[i].label2 < ret)
                ret = d->m_merge_pairs[i].label2;
        }
    }
    return ret;
}

QList<int> ClusterMerge::representativeLabels() const
{
    QSet<int> ret;
    for (int i = 0; i < d->m_merge_pairs.count(); i++) {
        ret.insert(this->representativeLabel(d->m_merge_pairs[i].label1));
    }
    QList<int> ret2 = ret.toList();
    qSort(ret2);
    return ret2;
}

QList<int> ClusterMerge::getMergeGroup(int label) const
{
    QList<int> ret;
    for (int i = 0; i < d->m_merge_pairs.count(); i++) {
        if (d->m_merge_pairs[i].label1 == label) {
            ret << d->m_merge_pairs[i].label2;
        }
    }
    qSort(ret);
    return ret;
}

QString ClusterMerge::toJson() const
{
    QJsonArray X;
    QList<int> rep_labels = this->representativeLabels();
    for (int i = 0; i < rep_labels.count(); i++) {
        QList<int> grp = this->getMergeGroup(rep_labels[i]);
        QJsonArray Y;
        for (int j = 0; j < grp.count(); j++)
            Y.append(grp[j]);
        X.append(Y);
    }
    return QJsonDocument(X).toJson();
}

ClusterMerge ClusterMerge::fromJson(const QString& json)
{
    ClusterMerge ret;
    QJsonArray X = QJsonDocument::fromJson(json.toLatin1()).array();
    for (int i = 0; i < X.count(); i++) {
        QJsonArray Y = X[i].toArray();
        QList<int> grp;
        for (int j = 0; j < Y.count(); j++) {
            grp << Y[j].toInt();
        }
        ret.merge(grp);
    }
    return ret;
}

bool ClusterMergePrivate::is_merged(int label1, int label2)
{
    if (label1 == label2)
        return true;
    for (int i = 0; i < m_merge_pairs.count(); i++) {
        if ((m_merge_pairs[i].label1 == label1) && (m_merge_pairs[i].label2 == label2))
            return true;
    }
    return false;
}
