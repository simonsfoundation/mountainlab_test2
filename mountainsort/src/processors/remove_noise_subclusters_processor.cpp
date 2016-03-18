#include "remove_noise_subclusters_processor.h"
#include "remove_noise_subclusters.h"

class remove_noise_subclusters_ProcessorPrivate
{
public:
	remove_noise_subclusters_Processor *q;
};

remove_noise_subclusters_Processor::remove_noise_subclusters_Processor() {
	d=new remove_noise_subclusters_ProcessorPrivate;
	d->q=this;

	this->setName("remove_noise_subclusters");
	this->setVersion("0.1");
    this->setInputFileParameters("signal","firings");
	this->setOutputFileParameters("firings_out");
	this->setRequiredParameters("clip_size","detectability_threshold","shell_increment","min_shell_size");
}

remove_noise_subclusters_Processor::~remove_noise_subclusters_Processor() {
	delete d;
}

bool remove_noise_subclusters_Processor::check(const QMap<QString, QVariant> &params)
{
	if (!this->checkParameters(params)) return false;
	return true;
}

bool remove_noise_subclusters_Processor::run(const QMap<QString, QVariant> &params)
{

	Remove_noise_subclusters_opts opts;
	QString signal_path=params["signal"].toString();
    QString firings_path=params["firings"].toString();
	QString firings_out_path=params["firings_out"].toString();
	opts.clip_size=params["clip_size"].toInt();
	opts.detectability_threshold=params["detectability_threshold"].toDouble();
	opts.shell_increment=params["shell_increment"].toDouble();
	opts.min_shell_size=params["min_shell_size"].toInt();
	return remove_noise_subclusters(signal_path,firings_path,firings_out_path,opts);
}
