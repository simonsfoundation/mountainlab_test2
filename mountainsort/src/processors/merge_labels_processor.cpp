/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#include "merge_labels_processor.h"
#include "merge_labels.h"

class merge_labels_ProcessorPrivate {
public:
    merge_labels_Processor* q;
};

merge_labels_Processor::merge_labels_Processor()
{
    d = new merge_labels_ProcessorPrivate;
    d->q = this;

    this->setName("merge_labels");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("merge_threshold", "clip_size");
}

merge_labels_Processor::~merge_labels_Processor()
{
    delete d;
}

bool merge_labels_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool merge_labels_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    merge_labels_opts opts;
    opts.merge_threshold = params["merge_threshold"].toDouble();
    opts.clip_size = params["clip_size"].toInt();
    return merge_labels(timeseries_path, firings_path, firings_out_path, opts);
}
