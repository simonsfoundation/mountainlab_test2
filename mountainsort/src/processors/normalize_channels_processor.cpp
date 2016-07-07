#include "normalize_channels_processor.h"
#include "normalize_channels.h"

class normalize_channels_ProcessorPrivate {
public:
    normalize_channels_Processor* q;
};

normalize_channels_Processor::normalize_channels_Processor()
{
    d = new normalize_channels_ProcessorPrivate;
    d->q = this;

    this->setName("normalize_channels");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
}

normalize_channels_Processor::~normalize_channels_Processor()
{
    delete d;
}

bool normalize_channels_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool normalize_channels_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["timeseries"].toString();
    QString output = params["timeseries_out"].toString();
    return normalize_channels(input, output);
}
