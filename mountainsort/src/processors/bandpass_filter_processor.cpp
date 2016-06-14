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
	this->setVersion("0.2");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
    this->setRequiredParameters("samplerate","freq_min","freq_max","freq_wid");
    this->setOptionalParameters("processing_chunk_size", "chunk_overlap_size");
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
    QString input=params["timeseries"].toString();
    QString output=params["timeseries_out"].toString();
    double samplerate=params["samplerate"].toDouble();
	double freq_min=params["freq_min"].toDouble();
	double freq_max=params["freq_max"].toDouble();
	double freq_wid=params["freq_wid"].toDouble();
    const long chunkSize = params.value("processing_chunk_size", -1).toLongLong();
    const long overlapSize = params.value("chunk_overlap_size", -1).toLongLong();
    return bandpass_filter0(input,output,samplerate,freq_min,freq_max,chunkSize,overlapSize);
}
