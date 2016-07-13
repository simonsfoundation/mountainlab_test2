/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/13/2016
*******************************************************/

#include "mvmergecontrol.h"

#include <QToolButton>
#include <flowlayout.h>

class MVMergeControlPrivate {
public:
    MVMergeControl* q;

    void do_merge_or_unmerge(bool merge);
};

MVMergeControl::MVMergeControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVMergeControlPrivate;
    d->q = this;

    QHBoxLayout* layout1 = new QHBoxLayout;
    {
        QToolButton* B = this->createToolButtonControl("merge_selected", "Merge selected", this, SLOT(slot_merge_selected()));
        layout1->addWidget(B);
    }
    {
        QToolButton* B = this->createToolButtonControl("unmerge_selected", "Unmerge selected", this, SLOT(slot_unmerge_selected()));
        layout1->addWidget(B);
    }
    layout1->addStretch();

    QHBoxLayout* layout2 = new QHBoxLayout;
    {
        QCheckBox* CB = this->createCheckBoxControl("view_merged", "View merged");
        layout2->addWidget(CB);
    }
    layout2->addStretch();

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addLayout(layout1);
    vlayout->addLayout(layout2);

    this->setLayout(vlayout);

    this->updateControlsOn(context, SIGNAL(viewMergedChanged()));
    this->updateControlsOn(context, SIGNAL(selectedClusterPairsChanged()));
    this->updateControlsOn(context, SIGNAL(selectedClustersChanged()));
}

MVMergeControl::~MVMergeControl()
{
    delete d;
}

QString MVMergeControl::title() const
{
    return "Merge";
}

void MVMergeControl::updateContext()
{
    mvContext()->setViewMerged(this->controlValue("view_merged").toBool());
}

void MVMergeControl::updateControls()
{
    bool can_merge_or_unmerge = false;
    if (!mvContext()->selectedClusterPairs().isEmpty()) {
        can_merge_or_unmerge = true;
    }
    if (mvContext()->selectedClusters().count() >= 2) {
        can_merge_or_unmerge = true;
    }
    this->setControlEnabled("merge_selected", can_merge_or_unmerge);
    this->setControlEnabled("unmerge_selected", can_merge_or_unmerge);

    this->setControlValue("view_merged", mvContext()->viewMerged());
}

void MVMergeControl::slot_merge_selected()
{
    d->do_merge_or_unmerge(true);
    mvContext()->setViewMerged(true);
}

void MVMergeControl::slot_unmerge_selected()
{
    d->do_merge_or_unmerge(false);
}

void MVMergeControlPrivate::do_merge_or_unmerge(bool merge)
{
    QSet<ClusterPair> selected_cluster_pairs = q->mvContext()->selectedClusterPairs();
    if (selected_cluster_pairs.count() >= 1) {
        foreach (ClusterPair pair, selected_cluster_pairs) {
            QSet<QString> tags = q->mvContext()->clusterPairTags(pair);
            if (merge)
                tags.insert("merged");
            else
                tags.remove("merged");
            q->mvContext()->setClusterPairTags(pair, tags);
        }
    }
    else {
        QList<int> selected_clusters = q->mvContext()->selectedClusters();
        //we need to do something special since it is overkill to merge every pair -- and expensive for large # clusters
        if (merge) {
            ClusterMerge CM = q->mvContext()->clusterMerge();
            for (int i1 = 0; i1 < selected_clusters.count() - 1; i1++) {
                int i2 = i1 + 1;
                ClusterPair pair(selected_clusters[i1], selected_clusters[i2]);
                if (CM.representativeLabel(pair.kmin()) != CM.representativeLabel(pair.kmax())) {
                    //not already merged
                    QSet<QString> tags = q->mvContext()->clusterPairTags(pair);
                    tags.insert("merged");
                    q->mvContext()->setClusterPairTags(pair, tags);
                    QSet<int> tmp;
                    tmp.insert(pair.kmin());
                    tmp.insert(pair.kmax());
                    CM.merge(tmp);
                }
            }
        }
        else {
            QSet<int> selected_clusters_set = selected_clusters.toSet();
            QList<ClusterPair> keys = q->mvContext()->clusterPairAttributesKeys();
            foreach (ClusterPair pair, keys) {
                if ((selected_clusters_set.contains(pair.kmin())) || (selected_clusters_set.contains(pair.kmax()))) {
                    QSet<QString> tags = q->mvContext()->clusterPairTags(pair);
                    tags.remove("merged");
                    q->mvContext()->setClusterPairTags(pair, tags);
                }
            }
        }
    }
}
