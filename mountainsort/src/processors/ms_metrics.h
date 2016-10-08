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
    double add_noise_level = 0.25;
};

namespace MSMetrics {
bool ms_metrics(QString timeseries, QString firings, QString cluster_metrics, QString cluster_pair_metrics_path, ms_metrics_opts opts);
}

#endif // ms_metrics_H
