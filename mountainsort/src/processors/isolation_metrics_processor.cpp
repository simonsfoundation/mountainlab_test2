/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "isolation_metrics_processor.h"
#include "isolation_metrics.h"

class isolation_metrics_ProcessorPrivate {
public:
    isolation_metrics_Processor* q;
};

isolation_metrics_Processor::isolation_metrics_Processor()
{
    d = new isolation_metrics_ProcessorPrivate;
    d->q = this;

    this->setName("isolation_metrics");
    this->setVersion("0.11");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("cluster_metrics", "cluster_pair_metrics");
    this->setOptionalParameters("cluster_numbers");
}

isolation_metrics_Processor::~isolation_metrics_Processor()
{
    delete d;
}

bool isolation_metrics_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool isolation_metrics_Processor::run(const QMap<QString, QVariant>& params)
{
    isolation_metrics_opts opts;
    QStringList cluster_numbers_str = params["cluster_numbers"].toString().split(",", QString::SkipEmptyParts);
    foreach (QString num, cluster_numbers_str) {
        opts.cluster_numbers << num.toInt();
    }
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString cluster_metrics_path = params["cluster_metrics"].toString();
    QString cluster_pair_metrics_path = params["cluster_pair_metrics"].toString();
    return IsolationMetrics::isolation_metrics(timeseries_path, firings_path, cluster_metrics_path, cluster_pair_metrics_path, opts);
}
