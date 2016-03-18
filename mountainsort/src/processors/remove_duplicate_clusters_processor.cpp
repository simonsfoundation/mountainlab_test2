#include "remove_duplicate_clusters_processor.h"
#include "remove_duplicate_clusters.h"

class remove_duplicate_clusters_ProcessorPrivate
{
public:
    remove_duplicate_clusters_Processor *q;
};

remove_duplicate_clusters_Processor::remove_duplicate_clusters_Processor() {
    d=new remove_duplicate_clusters_ProcessorPrivate;
	d->q=this;

    this->setName("remove_duplicate_clusters");
	this->setVersion("0.1");
    this->setInputFileParameters("firings");
	this->setOutputFileParameters("firings_out");
	this->setRequiredParameters("max_dt","overlap_threshold");
}

remove_duplicate_clusters_Processor::~remove_duplicate_clusters_Processor() {
	delete d;
}

bool remove_duplicate_clusters_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool remove_duplicate_clusters_Processor::run(const QMap<QString, QVariant> &params)
{
    QString firings_path=params["firings"].toString();
    QString firings_out_path=params["firings_out"].toString();
	int max_dt=params["max_dt"].toInt();
	double overlap_threshold=params["overlap_threshold"].toDouble();
    return remove_duplicate_clusters(firings_path,firings_out_path,max_dt,overlap_threshold);
}
