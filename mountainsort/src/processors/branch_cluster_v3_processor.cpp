#include "branch_cluster_v3_processor.h"
#include "branch_cluster_v3.h"
#include "omp.h"

class branch_cluster_v3_ProcessorPrivate {
public:
    branch_cluster_v3_Processor* q;
};

branch_cluster_v3_Processor::branch_cluster_v3_Processor()
{
    d = new branch_cluster_v3_ProcessorPrivate;
    d->q = this;

    this->setName("branch_cluster_v3");
    this->setVersion("0.34");
    this->setInputFileParameters("timeseries", "detect", "adjacency_matrix");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size", "num_features");
    this->setRequiredParameters("detect_interval");
    this->setOptionalParameters("num_pca_representatives", "consolidation_factor");
    this->setOptionalParameters("num_threads", "num_features2");
    this->setOptionalParameters("isocut_threshold");
}

branch_cluster_v3_Processor::~branch_cluster_v3_Processor()
{
    delete d;
}

bool branch_cluster_v3_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool branch_cluster_v3_Processor::run(const QMap<QString, QVariant>& params)
{
    Branch_Cluster_V3_Opts opts;

    QString timeseries_path = params["timeseries"].toString();
    QString detect_path = params["detect"].toString();
    QString adjacency_matrix_path = params["adjacency_matrix"].toString();
    QString firings_path = params["firings_out"].toString();
    opts.clip_size = params["clip_size"].toInt();
    opts.num_features = params["num_features"].toInt();
    opts.num_features2 = params.value("num_features2", 0).toInt();
    opts.detect_interval = params["detect_interval"].toInt();
    opts.num_pca_representatives = (long)params.value("num_pca_representatives", 5000).toDouble();
    opts.consolidation_factor = params.value("consolidation_factor", 0.9).toDouble();
    opts.isocut_threshold = params.value("isocut_threshold", 1.5).toDouble();

    int num_threads = params.value("num_threads", 0).toInt();
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    }

    return branch_cluster_v3(timeseries_path, detect_path, adjacency_matrix_path, firings_path, opts);
}
