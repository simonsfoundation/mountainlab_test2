/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_across_channels_processor.h"
#include "merge_across_channels.h"

class merge_across_channels_ProcessorPrivate {
public:
    merge_across_channels_Processor* q;
};

merge_across_channels_Processor::merge_across_channels_Processor()
{
    d = new merge_across_channels_ProcessorPrivate;
    d->q = this;

    this->setName("merge_across_channels");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("min_peak_ratio", "max_dt", "min_coinc_frac", "min_coinc_num");
    this->setRequiredParameters("max_corr_stddev", "min_template_corr_coef", "clip_size");
}

merge_across_channels_Processor::~merge_across_channels_Processor()
{
    delete d;
}

bool merge_across_channels_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool merge_across_channels_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    merge_across_channels_opts opts;
    opts.max_corr_stddev = params["max_corr_stddev"].toDouble();
    opts.max_dt = params["max_dt"].toInt();
    opts.min_coinc_frac = params["min_coinc_frac"].toDouble();
    opts.min_coinc_num = params["min_coinc_num"].toInt();
    opts.min_peak_ratio = params["min_peak_ratio"].toDouble();
    opts.min_template_corr_coef = params["min_template_corr_coef"].toDouble();
    opts.clip_size = params["clip_size"].toInt();
    return merge_across_channels(timeseries_path, firings_path, firings_out_path, opts);
}
