/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/
#ifndef ms_metrics_H
#define ms_metrics_H

#include <QList>
#include <diskreadmda.h>
#include <diskreadmda32.h>

struct ms_metrics_opts {
    QList<int> cluster_numbers;
    int clip_size = 50;
    double min_peak_ratio_to_consider = 0.7;
    int num_features = 10;
    int K_nearest = 6;
    int exhaustive_search_num = 300;
    int max_num_to_use = 500;
    int min_num_to_use = 100;
};

namespace MSMetrics {
bool ms_metrics(QString timeseries, QString firings, QString cluster_metrics, QString cluster_pair_metrics_path, ms_metrics_opts opts);
}

#endif // ms_metrics_H
