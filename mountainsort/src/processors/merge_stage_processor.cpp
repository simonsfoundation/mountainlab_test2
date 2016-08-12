/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_stage_processor.h"
#include "merge_stage.h"

class merge_stage_ProcessorPrivate {
public:
    merge_stage_Processor* q;
};

merge_stage_Processor::merge_stage_Processor()
{
    d = new merge_stage_ProcessorPrivate;
    d->q = this;

    this->setName("merge_stage");
    this->setVersion("0.11");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("min_peak_ratio");
    this->setRequiredParameters("min_template_corr_coef", "clip_size");
}

merge_stage_Processor::~merge_stage_Processor()
{
    delete d;
}

bool merge_stage_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool merge_stage_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    merge_stage_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    opts.min_peak_ratio = params["min_peak_ratio"].toDouble();
    opts.min_template_corr_coef = params["min_template_corr_coef"].toDouble();
    return merge_stage(timeseries_path, firings_path, firings_out_path, opts);
}
