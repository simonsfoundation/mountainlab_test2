#include "bandpass_filter_processor.h"
#include "bandpass_filter0.h"

class bandpass_filter_ProcessorPrivate {
public:
    bandpass_filter_Processor* q;
};

bandpass_filter_Processor::bandpass_filter_Processor()
{
    d = new bandpass_filter_ProcessorPrivate;
    d->q = this;

    this->setName("bandpass_filter");
    this->setVersion("0.21");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
    this->setRequiredParameters("samplerate", "freq_min", "freq_max");
    this->setOptionalParameters("freq_wid");
}

bandpass_filter_Processor::~bandpass_filter_Processor()
{
    delete d;
}

bool bandpass_filter_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool bandpass_filter_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["timeseries"].toString();
    QString output = params["timeseries_out"].toString();
    double samplerate = params["samplerate"].toDouble();
    double freq_min = params["freq_min"].toDouble();
    double freq_max = params["freq_max"].toDouble();
    double freq_wid = params.value("freq_wid", 1000).toDouble();
    if (!freq_wid)
        freq_wid = 1000; //added on 6/21/16
    return bandpass_filter0(input, output, samplerate, freq_min, freq_max, freq_wid);
}
