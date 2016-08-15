/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#include "merge_across_channels_v2_processor.h"
#include "merge_across_channels_v2.h"

class merge_across_channels_v2_ProcessorPrivate {
public:
    merge_across_channels_v2_Processor* q;
};

merge_across_channels_v2_Processor::merge_across_channels_v2_Processor()
{
    d = new merge_across_channels_v2_ProcessorPrivate;
    d->q = this;

    this->setName("merge_across_channels_v2");
    this->setVersion("0.19");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size");
}

merge_across_channels_v2_Processor::~merge_across_channels_v2_Processor()
{
    delete d;
}

bool merge_across_channels_v2_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool merge_across_channels_v2_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    merge_across_channels_v2_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    return merge_across_channels_v2(timeseries_path, firings_path, firings_out_path, opts);
}
