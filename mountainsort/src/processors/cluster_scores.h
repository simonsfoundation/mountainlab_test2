/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/
#ifndef CLUSTER_SCORES_H
#define CLUSTER_SCORES_H

#include <QList>
#include <diskreadmda.h>
#include <diskreadmda32.h>

struct cluster_scores_opts {
    int clip_size = 50;
    double detect_threshold = 0;
    int max_comparisons_per_cluster = 5;
    QList<int> cluster_numbers;
    double add_noise_level = 1;
    int cluster_scores_only = 0; //0 or 1
};

namespace ClusterScores {
bool cluster_scores(QString timeseries, QString firings, QString cluster_scores, QString cluster_pair_scores, cluster_scores_opts opts);
void find_pairs_to_compare(QList<int>& k1s, QList<int>& k2s, const DiskReadMda32& timeseries, const DiskReadMda& firings, cluster_scores_opts opts);
QVector<double> compute_cluster_pair_scores(DiskReadMda32 timeseries, const Mda32& clips1, const Mda32& clips2, cluster_scores_opts opts, QVector<double>* out_proj_data1 = 0, QVector<double>* out_proj_data2 = 0);
}

#endif // CLUSTER_SCORES_H
