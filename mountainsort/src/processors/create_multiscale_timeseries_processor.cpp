#include "create_multiscale_timeseries_processor.h"
#include "create_multiscale_timeseries.h"

class create_multiscale_timeseries_ProcessorPrivate {
public:
    create_multiscale_timeseries_Processor* q;
};

create_multiscale_timeseries_Processor::create_multiscale_timeseries_Processor()
{
    d = new create_multiscale_timeseries_ProcessorPrivate;
    d->q = this;

    this->setName("create_multiscale_timeseries");
    this->setVersion("0.17");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
    //this->setRequiredParameters();
}

create_multiscale_timeseries_Processor::~create_multiscale_timeseries_Processor()
{
    delete d;
}

bool create_multiscale_timeseries_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool create_multiscale_timeseries_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString timeseries_out_path = params["timeseries_out"].toString();
    return create_multiscale_timeseries(timeseries_path, timeseries_out_path);
}
