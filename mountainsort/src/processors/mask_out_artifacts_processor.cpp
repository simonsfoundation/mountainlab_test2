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
    this->setInputFileParameters("timeseries");
	this->setOutputFileParameters("timeseries_out");
    this->setRequiredParameters("threshold","interval_size");
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
    QString timeseries_path=params["timeseries"].toString();
	QString timeseries_out_path=params["timeseries_out"].toString();
	double threshold=params["threshold"].toDouble();
	int interval_size=params["interval_size"].toInt();
	return mask_out_artifacts(timeseries_path,timeseries_out_path,threshold,interval_size);
}
