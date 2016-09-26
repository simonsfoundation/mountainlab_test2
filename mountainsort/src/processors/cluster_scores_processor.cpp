/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "cluster_scores_processor.h"
#include "cluster_scores.h"

class cluster_scores_ProcessorPrivate {
public:
    cluster_scores_Processor* q;
};

cluster_scores_Processor::cluster_scores_Processor()
{
    d = new cluster_scores_ProcessorPrivate;
    d->q = this;

    this->setName("cluster_scores");
    this->setVersion("0.13");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("cluster_scores", "cluster_pair_scores");
    this->setRequiredParameters("clip_size", "detect_threshold");
    this->setRequiredParameters("add_noise_level");
    this->setOptionalParameters("cluster_numbers", "max_comparisons_per_cluster");
    this->setOptionalParameters("cluster_scores_only");
}

cluster_scores_Processor::~cluster_scores_Processor()
{
    delete d;
}

bool cluster_scores_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool cluster_scores_Processor::run(const QMap<QString, QVariant>& params)
{
    cluster_scores_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    opts.detect_threshold = params["detect_threshold"].toDouble();
    QStringList cluster_numbers_str = params["cluster_numbers"].toString().split(",", QString::SkipEmptyParts);
    foreach (QString num, cluster_numbers_str) {
        opts.cluster_numbers << num.toInt();
    }
    opts.max_comparisons_per_cluster = params["max_comparisons_per_cluster"].toInt();
    opts.add_noise_level = params["add_noise_level"].toDouble();
    opts.cluster_scores_only = params.value("cluster_scores_only", 0).toInt();
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString cluster_scores_path = params["cluster_scores"].toString();
    QString cluster_pair_scores_path = params["cluster_pair_scores"].toString();
    return ClusterScores::cluster_scores(timeseries_path, firings_path, cluster_scores_path, cluster_pair_scores_path, opts);
}
