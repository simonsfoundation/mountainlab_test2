#include "detect3_processor.h"
#include "detect3.h"

class detect3_ProcessorPrivate {
public:
    detect3_Processor* q;
};

detect3_Processor::detect3_Processor()
{
    d = new detect3_ProcessorPrivate;
    d->q = this;

    this->setName("detect3");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("detect_out");
    this->setRequiredParameters("clip_size", "detect_interval", "detect_threshold");
    this->setOptionalParameters("sign");
    this->setOptionalParameters("beta");
}

detect3_Processor::~detect3_Processor()
{
    delete d;
}

bool detect3_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool detect3_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString detect_path = params["detect_out"].toString();
    Detect3_Opts opts;
    opts.clip_size = params["clip_size"].toInt();
    opts.detect_interval = params["detect_interval"].toInt();
    opts.detect_threshold = params["detect_threshold"].toDouble();
    opts.sign = params.value("sign", 0).toInt();
    opts.beta = params.value("beta", 1).toInt();
    return detect3(timeseries_path, detect_path, opts);
}
