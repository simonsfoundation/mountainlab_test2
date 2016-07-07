#include "whiten_processor.h"
#include "whiten.h"

class whiten_ProcessorPrivate {
public:
    whiten_Processor* q;
};

whiten_Processor::whiten_Processor()
{
    d = new whiten_ProcessorPrivate;
    d->q = this;

    this->setName("whiten");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
}

whiten_Processor::~whiten_Processor()
{
    delete d;
}

bool whiten_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool whiten_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["timeseries"].toString();
    QString output = params["timeseries_out"].toString();
    return whiten(input, output);
}
