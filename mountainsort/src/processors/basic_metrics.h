/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/
#ifndef basic_metrics_H
#define basic_metrics_H

#include <QList>
#include <diskreadmda.h>
#include <diskreadmda32.h>

struct basic_metrics_opts {
    QList<int> cluster_numbers;
    double samplerate = 0;
};

namespace BasicMetrics {
bool basic_metrics(QString timeseries, QString firings, QString cluster_metrics, QString cluster_pair_metrics, basic_metrics_opts opts);
}

#endif // basic_metrics_H
