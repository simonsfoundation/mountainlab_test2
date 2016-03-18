#include "bandpass_filter_processor.h"
#include "bandpass_filter0.h"

class bandpass_filter_ProcessorPrivate
{
public:
	bandpass_filter_Processor *q;
};

bandpass_filter_Processor::bandpass_filter_Processor() {
	d=new bandpass_filter_ProcessorPrivate;
	d->q=this;

	this->setName("bandpass_filter");
	this->setVersion("0.1");
    this->setInputFileParameters("signal");
    this->setOutputFileParameters("signal_out");
    this->setRequiredParameters("samplerate","freq_min","freq_max");
}

bandpass_filter_Processor::~bandpass_filter_Processor() {
	delete d;
}

bool bandpass_filter_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool bandpass_filter_Processor::run(const QMap<QString, QVariant> &params)
{
    QString input=params["signal"].toString();
    QString output=params["signal_out"].toString();
    double samplerate=params["samplerate"].toDouble();
	double freq_min=params["freq_min"].toDouble();
	double freq_max=params["freq_max"].toDouble();
    return bandpass_filter0(input,output,samplerate,freq_min,freq_max);
}
