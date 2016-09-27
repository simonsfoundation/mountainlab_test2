/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "basic_metrics_processor.h"
#include "basic_metrics.h"

class basic_metrics_ProcessorPrivate {
public:
    basic_metrics_Processor* q;
};

basic_metrics_Processor::basic_metrics_Processor()
{
    d = new basic_metrics_ProcessorPrivate;
    d->q = this;

    this->setName("basic_metrics");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("cluster_metrics", "cluster_pair_metrics");
    this->setRequiredParameters("samplerate");
    this->setOptionalParameters("cluster_numbers");
}

basic_metrics_Processor::~basic_metrics_Processor()
{
    delete d;
}

bool basic_metrics_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool basic_metrics_Processor::run(const QMap<QString, QVariant>& params)
{
    basic_metrics_opts opts;
    opts.samplerate = params["samplerate"].toDouble();
    QStringList cluster_numbers_str = params["cluster_numbers"].toString().split(",", QString::SkipEmptyParts);
    foreach (QString num, cluster_numbers_str) {
        opts.cluster_numbers << num.toInt();
    }
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString cluster_metrics_path = params["cluster_metrics"].toString();
    QString cluster_pair_metrics_path = params["cluster_pair_metrics"].toString();
    return BasicMetrics::basic_metrics(timeseries_path, firings_path, cluster_metrics_path, cluster_pair_metrics_path, opts);
}
