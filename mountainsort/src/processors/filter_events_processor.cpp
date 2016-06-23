/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#include "filter_events_processor.h"
#include "filter_events.h"

class filter_events_ProcessorPrivate {
public:
    filter_events_Processor* q;
};

filter_events_Processor::filter_events_Processor()
{
    d = new filter_events_ProcessorPrivate;
    d->q = this;

    this->setName("filter_events");
    this->setVersion("0.1");
    this->setInputFileParameters("firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("detectability_threshold", "outlier_threshold");
}

filter_events_Processor::~filter_events_Processor()
{
    delete d;
}

bool filter_events_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool filter_events_Processor::run(const QMap<QString, QVariant>& params)
{
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();
    filter_events_opts opts;
    opts.detectability_threshold = params["detectability_threshold"].toDouble();
    opts.outlier_threshold = params["outlier_threshold"].toDouble();
    return filter_events(firings_path, firings_out_path, opts);
}
