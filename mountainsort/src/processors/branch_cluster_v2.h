/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef BRANCH_CLUSTER_V2_H
#define BRANCH_CLUSTER_V2_H

#include <QString>

struct Branch_Cluster_V2_Opts {
    int clip_size;
    int min_shell_size;
    double shell_increment;
    int num_features;
    int detect_interval;
    long num_pca_representatives;
    double consolidation_factor;
};

bool branch_cluster_v2(const QString& timeseries_path, const QString& detect_path, const QString& adjacency_matrix_path, const QString& output_firings_path, const Branch_Cluster_V2_Opts& opts);

#endif // BRANCH_CLUSTER_V2_H
