#include "remove_duplicate_clusters_processor.h"
#include "remove_duplicate_clusters.h"

class remove_duplicate_clusters_ProcessorPrivate {
public:
    remove_duplicate_clusters_Processor* q;
};

remove_duplicate_clusters_Processor::remove_duplicate_clusters_Processor()
{
    d = new remove_duplicate_clusters_ProcessorPrivate;
    d->q = this;

    this->setName("remove_duplicate_clusters");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size");
}

remove_duplicate_clusters_Processor::~remove_duplicate_clusters_Processor()
{
    delete d;
}

bool remove_duplicate_clusters_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool remove_duplicate_clusters_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString firings_out_path = params["firings_out"].toString();

    remove_duplicate_clusters_Opts opts;
    opts.clip_size = params["clip_size"].toInt();

    return remove_duplicate_clusters(timeseries_path, firings_path, firings_out_path, opts);
}
