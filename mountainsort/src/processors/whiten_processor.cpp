#include "whiten_processor.h"
#include "whiten.h"

class whiten_ProcessorPrivate
{
public:
	whiten_Processor *q;
};

whiten_Processor::whiten_Processor() {
	d=new whiten_ProcessorPrivate;
	d->q=this;

	this->setName("whiten");
	this->setVersion("0.1");
    this->setInputFileParameters("raw");
    this->setOutputFileParameters("raw_out");
}

whiten_Processor::~whiten_Processor() {
	delete d;
}

bool whiten_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool whiten_Processor::run(const QMap<QString, QVariant> &params)
{
    QString input=params["raw"].toString();
    QString output=params["raw_out"].toString();
	return whiten(input,output);
}


