#include "detect_processor.h"
#include "detect.h"

class detect_ProcessorPrivate
{
public:
	detect_Processor *q;
};

detect_Processor::detect_Processor() {
	d=new detect_ProcessorPrivate;
	d->q=this;

	this->setName("detect");
	this->setVersion("0.1");
	this->setInputFileParameters("signal");
    this->setOutputFileParameters("detect_out");
	this->setRequiredParameters("clip_size","detect_interval","detect_threshold");
	this->setOptionalParameters("sign");
}

detect_Processor::~detect_Processor() {
	delete d;
}

bool detect_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool detect_Processor::run(const QMap<QString, QVariant> &params)
{
	QString signal_path=params["signal"].toString();
    QString detect_path=params["detect_out"].toString();
	Detect_Opts opts;
	opts.clip_size=params["clip_size"].toInt();
	opts.detect_interval=params["detect_interval"].toInt();
	opts.detect_threshold=params["detect_threshold"].toDouble();
	opts.sign=params.value("sign",0).toInt();
	return detect(signal_path,detect_path,opts);
}
