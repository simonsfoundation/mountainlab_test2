/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#include "fit_stage_processor.h"
#include "fit_stage.h"

class fit_stage_ProcessorPrivate {
public:
    fit_stage_Processor* q;
};

fit_stage_Processor::fit_stage_Processor()
{
    d = new fit_stage_ProcessorPrivate;
    d->q = this;

    this->setName("fit_stage");
    this->setVersion("0.17");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size", "shell_increment", "min_shell_size");
    this->setOptionalParameters("use_old");
    this->setOptionalParameters("neglogprior");
}

fit_stage_Processor::~fit_stage_Processor()
{
    delete d;
}

bool fit_stage_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool fit_stage_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    int clip_size = params["clip_size"].toInt();
    fit_stage_opts opts;
    opts.clip_size = clip_size;
    opts.min_shell_size = params["min_shell_size"].toInt();
    opts.shell_increment = params["shell_increment"].toDouble();
    opts.neglogprior = params.value("neglogprior", 30).toDouble();
    //if (params["use_old"].toBool()) {
    //    return fit_stage(timeseries_path, firings_path, firings_out_path, opts);
    //}
    //else {
    return fit_stage_new(timeseries_path, firings_path, firings_out_path, opts);
    //}
}
