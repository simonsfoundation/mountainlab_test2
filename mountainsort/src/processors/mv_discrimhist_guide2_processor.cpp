#include "mv_discrimhist_guide2_processor.h"
#include "mv_discrimhist_guide2.h"
#include "mlcommon.h"

class mv_discrimhist_guide2_ProcessorPrivate {
public:
    mv_discrimhist_guide2_Processor* q;
};

mv_discrimhist_guide2_Processor::mv_discrimhist_guide2_Processor()
{
    d = new mv_discrimhist_guide2_ProcessorPrivate;
    d->q = this;

    this->setName("mv_discrimhist_guide2");
    this->setVersion("0.19");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("clip_size");
    this->setRequiredParameters("add_noise_level");
    this->setOptionalParameters("cluster_numbers", "max_comparisons_per_cluster");
}

mv_discrimhist_guide2_Processor::~mv_discrimhist_guide2_Processor()
{
    delete d;
}

bool mv_discrimhist_guide2_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool mv_discrimhist_guide2_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString output_path = params["output"].toString();

    mv_discrimhist_guide2_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    opts.add_noise_level = params["add_noise_level"].toDouble();
    QStringList strlist = params["cluster_numbers"].toString().split(",", QString::SkipEmptyParts);
    opts.cluster_numbers = MLUtil::stringListToIntList(strlist);
    opts.max_comparisons_per_cluster = params.value("max_comparisons_per_cluster", 4).toInt();

    return mv_discrimhist_guide2(timeseries_path, firings_path, output_path, opts);
}
