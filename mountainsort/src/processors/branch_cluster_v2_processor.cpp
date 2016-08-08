#include "branch_cluster_v2_processor.h"
#include "branch_cluster_v2.h"
#include "omp.h"

class branch_cluster_v2_ProcessorPrivate {
public:
    branch_cluster_v2_Processor* q;
};

branch_cluster_v2_Processor::branch_cluster_v2_Processor()
{
    d = new branch_cluster_v2_ProcessorPrivate;
    d->q = this;

    this->setName("branch_cluster_v2");
    this->setVersion("0.17");
    this->setInputFileParameters("timeseries", "detect", "adjacency_matrix");
    this->setOutputFileParameters("firings_out");
    this->setRequiredParameters("clip_size", "min_shell_size", "shell_increment", "num_features");
    this->setRequiredParameters("detect_interval");
    this->setOptionalParameters("num_pca_representatives", "consolidation_factor");
    this->setOptionalParameters("num_threads");
}

branch_cluster_v2_Processor::~branch_cluster_v2_Processor()
{
    delete d;
}

bool branch_cluster_v2_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool branch_cluster_v2_Processor::run(const QMap<QString, QVariant>& params)
{
    Branch_Cluster_V2_Opts opts;

    QString timeseries_path = params["timeseries"].toString();
    QString detect_path = params["detect"].toString();
    QString adjacency_matrix_path = params["adjacency_matrix"].toString();
    QString firings_path = params["firings_out"].toString();
    opts.clip_size = params["clip_size"].toInt();
    opts.min_shell_size = params["min_shell_size"].toInt();
    opts.shell_increment = params["shell_increment"].toDouble();
    opts.num_features = params["num_features"].toInt();
    opts.detect_interval = params["detect_interval"].toInt();
    opts.num_pca_representatives = params.value("num_pca_representatives", 5000).toLongLong();
    opts.consolidation_factor = params.value("consolidation_factor", 0.9).toDouble();

    int num_threads = params.value("num_threads", 0).toInt();
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    }

    //return branch_cluster_v2(timeseries_path, detect_path, adjacency_matrix_path, firings_path, opts);
    return branch_cluster_v2b(timeseries_path, detect_path, adjacency_matrix_path, firings_path, opts);
}
