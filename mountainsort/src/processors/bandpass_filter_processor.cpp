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
	this->setInputFileParameters("input");
	this->setOutputFileParameters("output");
}

bandpass_filter_Processor::~bandpass_filter_Processor() {
	delete d;
}

bool bandpass_filter_Processor::check(const QMap<QString, QVariant> &params)
{
	QStringList required; required << "sampling_freq" << "freq_min" << "freq_max";
	QStringList optional;
	if (!this->checkParameters(params,required,optional)) return false;
	return true;
}

bool bandpass_filter_Processor::run(const QMap<QString, QVariant> &params)
{
	QString input=params["input"].toString();
	QString output=params["output"].toString();
	double sampling_freq=params["sampling_freq"].toDouble();
	double freq_min=params["freq_min"].toDouble();
	double freq_max=params["freq_max"].toDouble();
	return bandpass_filter0(input,output,sampling_freq,freq_min,freq_max);
}

