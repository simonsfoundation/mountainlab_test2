#include "mv_discrimhist_sherpa_processor.h"
#include "mv_discrimhist_sherpa.h"
#include "mlcommon.h"

class mv_discrimhist_sherpa_ProcessorPrivate {
public:
    mv_discrimhist_sherpa_Processor* q;
};

mv_discrimhist_sherpa_Processor::mv_discrimhist_sherpa_Processor()
{
    d = new mv_discrimhist_sherpa_ProcessorPrivate;
    d->q = this;

    this->setName("mv_discrimhist_sherpa");
    this->setVersion("0.152");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("num_histograms", "clusters_to_exclude");
}

mv_discrimhist_sherpa_Processor::~mv_discrimhist_sherpa_Processor()
{
    delete d;
}

bool mv_discrimhist_sherpa_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool mv_discrimhist_sherpa_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString output_path = params["output"].toString();
    int num_histograms = params["num_histograms"].toInt();
    QStringList strlist = params["clusters_to_exclude"].toString().split(",", QString::SkipEmptyParts);
    QSet<int> clusters_to_exclude = MLUtil::stringListToIntList(strlist).toSet();

    mv_discrimhist_sherpa_opts opts;
    opts.num_histograms = num_histograms;
    opts.clusters_to_exclude = clusters_to_exclude;
    return mv_discrimhist_sherpa(timeseries_path, firings_path, output_path, opts);
}
