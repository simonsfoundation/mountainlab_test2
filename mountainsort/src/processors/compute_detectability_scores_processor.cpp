/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "compute_detectability_scores_processor.h"
#include "compute_detectability_scores.h"

class compute_detectability_scores_ProcessorPrivate {
public:
    compute_detectability_scores_Processor* q;
};

compute_detectability_scores_Processor::compute_detectability_scores_Processor()
{
    d = new compute_detectability_scores_ProcessorPrivate;
    d->q = this;

    this->setName("compute_detectability_scores");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size", "shell_increment", "min_shell_size");
}

compute_detectability_scores_Processor::~compute_detectability_scores_Processor()
{
    delete d;
}

bool compute_detectability_scores_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool compute_detectability_scores_Processor::run(const QMap<QString, QVariant>& params)
{
    compute_detectability_scores_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    opts.min_shell_size = params["min_shell_size"].toInt();
    opts.shell_increment = params["shell_increment"].toDouble();
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    return compute_detectability_scores(timeseries_path, firings_path, firings_out_path, opts);
}
