/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/
#ifndef CLUSTER_SCORES_H
#define CLUSTER_SCORES_H

#include <QList>

struct cluster_scores_opts {
    int clip_size=50;
    double detect_threshold=0;
    int max_comparisons_per_cluster=5;
    QList<int> cluster_numbers;
    double add_noise_level=1;
};

namespace ClusterScores {
    bool cluster_scores(QString timeseries,QString firings,QString cluster_scores,QString cluster_pair_scores,cluster_scores_opts opts);
}

#endif // CLUSTER_SCORES_H
