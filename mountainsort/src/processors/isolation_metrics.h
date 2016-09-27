/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/
#ifndef isolation_metrics_H
#define isolation_metrics_H

#include <QList>
#include <diskreadmda.h>
#include <diskreadmda32.h>

struct isolation_metrics_opts {
    QList<int> cluster_numbers;
};

namespace IsolationMetrics {
bool isolation_metrics(QString timeseries, QString firings, QString cluster_metrics, QString cluster_pair_metrics, isolation_metrics_opts opts);
}

#endif // isolation_metrics_H
