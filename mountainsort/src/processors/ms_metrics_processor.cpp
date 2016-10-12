/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "ms_metrics_processor.h"
#include "ms_metrics.h"

class ms_metrics_ProcessorPrivate {
public:
    ms_metrics_Processor* q;
};

ms_metrics_Processor::ms_metrics_Processor()
{
    d = new ms_metrics_ProcessorPrivate;
    d->q = this;

    this->setName("ms_metrics");
    this->setVersion("0.58");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("cluster_metrics", "cluster_pair_metrics");
    this->setRequiredParameters("clip_size");
    //this->setRequiredParameters("add_noise_level");
    //this->setOptionalParameters("cluster_numbers", "max_comparisons_per_cluster");
    //this->setOptionalParameters("ms_metrics_only");
}

ms_metrics_Processor::~ms_metrics_Processor()
{
    delete d;
}

bool ms_metrics_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool ms_metrics_Processor::run(const QMap<QString, QVariant>& params)
{
    ms_metrics_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    QStringList cluster_numbers_str = params["cluster_numbers"].toString().split(",", QString::SkipEmptyParts);
    foreach (QString num, cluster_numbers_str) {
        opts.cluster_numbers << num.toInt();
    }
    //opts.add_noise_level = params["add_noise_level"].toDouble();
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString cluster_metrics_path = params["cluster_metrics"].toString();
    QString cluster_pair_metrics_path = params["cluster_pair_metrics"].toString();
    return MSMetrics::ms_metrics(timeseries_path, firings_path, cluster_metrics_path, cluster_pair_metrics_path, opts);
}
