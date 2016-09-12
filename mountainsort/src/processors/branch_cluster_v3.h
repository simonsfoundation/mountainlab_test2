/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef BRANCH_CLUSTER_V3_H
#define BRANCH_CLUSTER_V3_H

#include <QString>

struct Branch_Cluster_V3_Opts {
    int clip_size = 50;
    int num_features = 10;
    int num_features2 = 0;
    int detect_interval = 10; //timepoints
    long num_pca_representatives; //??
    double consolidation_factor = 0.9;
    bool split_clusters_at_end = true; //??
    double isocut_threshold = 1.5; //For Hartigan test
};

bool branch_cluster_v3(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const Branch_Cluster_V3_Opts& opts);

#endif // BRANCH_CLUSTER_V3_H
