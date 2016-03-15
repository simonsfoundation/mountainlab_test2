#include "remove_duplicates_v1_processor.h"
#include "remove_duplicates_v1.h"

class remove_duplicates_v1_ProcessorPrivate
{
public:
	remove_duplicates_v1_Processor *q;
};

remove_duplicates_v1_Processor::remove_duplicates_v1_Processor() {
	d=new remove_duplicates_v1_ProcessorPrivate;
	d->q=this;

	this->setName("remove_duplicates_v1");
	this->setVersion("0.1");
	this->setInputFileParameters("firings_in");
	this->setOutputFileParameters("firings_out");
	this->setRequiredParameters("max_dt","overlap_threshold");
}

remove_duplicates_v1_Processor::~remove_duplicates_v1_Processor() {
	delete d;
}

bool remove_duplicates_v1_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool remove_duplicates_v1_Processor::run(const QMap<QString, QVariant> &params)
{
	QString firings_in_path=params["firings_in"].toString();
	QString firings_out_path=params["firings_in"].toString();
	int max_dt=params["max_dt"].toInt();
	double overlap_threshold=params["overlap_threshold"].toDouble();
	return remove_duplicates_v1(firings_in_path,firings_out_path,max_dt,overlap_threshold);
}


