/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/23/2016
*******************************************************/

#include "clustermerge.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QDebug>
#include "msmisc.h"

struct merge_pair {
    merge_pair(int l1, int l2)
    {
        label1 = l1;
        label2 = l2;
    }
    bool operator==(const merge_pair& other) const
    {
        return ((label1 == other.label1) && (label2 == other.label2));
    }

    int label1, label2;
};
uint qHash(const merge_pair& mp)
{
    return qHash(QString("%1:::%2").arg(mp.label1).arg(mp.label2));
}

class ClusterMergePrivate {
public:
    ClusterMerge* q;
    QSet<merge_pair> m_merge_pairs;

    bool is_merged(int label1, int label2);
    void make_transitive();
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

bool ClusterMerge::operator==(const ClusterMerge& other)
{
    if (d->m_merge_pairs.count() != other.d->m_merge_pairs.count())
        return false;
    foreach(merge_pair mp, d->m_merge_pairs)
    if (!other.d->m_merge_pairs.contains(mp))
        return false;
    foreach(merge_pair mp, other.d->m_merge_pairs)
    if (!d->m_merge_pairs.contains(mp))
        return false;
    return true;
}

void ClusterMerge::clear()
{
    d->m_merge_pairs.clear();
}

void ClusterMerge::merge(const QSet<int>& labels)
{
    merge(labels.toList());
}

void ClusterMerge::merge(const QList<int>& labels)
{
    for (int i = 0; i < labels.count(); i++) {
        for (int j = 0; j < labels.count(); j++) {
            if (labels[i] != labels[j]) {
                if (!d->is_merged(labels[i], labels[j])) {
                    d->m_merge_pairs.insert(merge_pair(labels[i], labels[j]));
                    d->m_merge_pairs.insert(merge_pair(labels[j], labels[i]));
                }
            }
        }
    }
    d->make_transitive();
}

void ClusterMerge::unmerge(int label)
{
    QSet<merge_pair> new_pairs;
    foreach(merge_pair mp, d->m_merge_pairs)
    {
        if ((mp.label1 != label) && (mp.label2 != label)) {
            new_pairs.insert(mp);
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
    foreach(merge_pair mp, d->m_merge_pairs)
    {
        if (mp.label1 == label) {
            if (mp.label2 < ret)
                ret = mp.label2;
        }
    }
    return ret;
}

QList<int> ClusterMerge::representativeLabels() const
{
    QSet<int> ret;
    foreach(merge_pair mp, d->m_merge_pairs)
    {
        ret.insert(this->representativeLabel(mp.label1));
    }
    QList<int> ret2 = ret.toList();
    qSort(ret2);
    return ret2;
}

QList<int> ClusterMerge::getMergeGroup(int label) const
{
    QList<int> ret;
    ret << label;
    foreach(merge_pair mp, d->m_merge_pairs)
    {
        if (mp.label1 == label) {
            ret << mp.label2;
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
        if (grp.count() > 1) {
            QJsonArray Y;
            for (int j = 0; j < grp.count(); j++)
                Y.append(grp[j]);
            X.append(Y);
        }
    }
    return QJsonDocument(X).toJson();
}

QString ClusterMerge::clusterLabelText(int label)
{
    QList<int> grp=this->getMergeGroup(label);
    if (grp.isEmpty()) return QString("%1").arg(label);
    if (label!=grp.value(0)) return "";
    QString str;
    for (int j = 0; j < grp.count(); j++) {
        if (!str.isEmpty())
            str += ",";
        str += QString("%1").arg(grp[j]);
    }
    return str;
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
    return m_merge_pairs.contains(merge_pair(label1, label2));
}

void ClusterMergePrivate::make_transitive()
{
    QSet<int> all_labels;
    foreach(merge_pair mp, m_merge_pairs)
    {
        all_labels.insert(mp.label1);
    }
    QList<int> all_labels0 = all_labels.toList();
    bool done = false;
    while (!done) {
        done = true;
        for (int i = 0; i < all_labels0.count(); i++) {
            for (int j = 0; j < all_labels0.count(); j++) {
                for (int k = 0; k < all_labels0.count(); k++) {
                    int label1 = all_labels0[i];
                    int label2 = all_labels0[j];
                    int label3 = all_labels0[k];
                    if ((is_merged(label1, label2)) && (is_merged(label2, label3)) && (!is_merged(label1, label3))) {
                        m_merge_pairs.insert(merge_pair(label1, label3));
                        done = false;
                    }
                }
            }
        }
    }
}
