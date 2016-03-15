#include "compute_outlier_scores_processor.h"
#include "compute_outlier_scores.h"

class compute_outlier_scores_ProcessorPrivate
{
public:
	compute_outlier_scores_Processor *q;
};

compute_outlier_scores_Processor::compute_outlier_scores_Processor() {
	d=new compute_outlier_scores_ProcessorPrivate;
	d->q=this;

	this->setName("compute_outlier_scores");
	this->setVersion("0.1");
	this->setInputFileParameters("raw","firings_in");
	this->setOutputFileParameters("firings_out");
	this->setRequiredParameters("clip_size","shell_increment","min_shell_size");
}

compute_outlier_scores_Processor::~compute_outlier_scores_Processor() {
	delete d;
}

bool compute_outlier_scores_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool compute_outlier_scores_Processor::run(const QMap<QString, QVariant> &params)
{
	Compute_Outlier_Scores_Opts opts;
	QString raw_path=params["raw"].toString();
	QString firings_in_path=params["firings_in"].toString();
	QString firings_out_path=params["firings_out"].toString();
	opts.clip_size=params["clip_size"].toInt();
	opts.shell_increment=params["shell_increment"].toDouble();
	opts.min_shell_size=params["min_shell_size"].toInt();
	return compute_outlier_scores(raw_path,firings_in_path,firings_out_path,opts);
}


