#include "mask_out_artifacts_processor.h"
#include "mask_out_artifacts.h"

class mask_out_artifacts_ProcessorPrivate
{
public:
	mask_out_artifacts_Processor *q;
};

mask_out_artifacts_Processor::mask_out_artifacts_Processor() {
	d=new mask_out_artifacts_ProcessorPrivate;
	d->q=this;

	this->setName("mask_out_artifacts");
	this->setVersion("0.1");
    this->setInputFileParameters("raw");
	this->setOutputFileParameters("raw_out");
	this->setRequiredParameters("threshold");
	this->setOptionalParameters("interval_size");
}

mask_out_artifacts_Processor::~mask_out_artifacts_Processor() {
	delete d;
}

bool mask_out_artifacts_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool mask_out_artifacts_Processor::run(const QMap<QString, QVariant> &params)
{
    QString raw_in_path=params["raw"].toString();
	QString raw_out_path=params["raw_out"].toString();
	double threshold=params["threshold"].toDouble();
	int interval_size=params["interval_size"].toInt();
	return mask_out_artifacts(raw_in_path,raw_out_path,threshold,interval_size);
}



