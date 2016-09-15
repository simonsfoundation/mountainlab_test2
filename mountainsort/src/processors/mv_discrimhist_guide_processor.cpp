#include "mv_discrimhist_guide_processor.h"
#include "mv_discrimhist_guide.h"
#include "mlcommon.h"

class mv_discrimhist_guide_ProcessorPrivate {
public:
    mv_discrimhist_guide_Processor* q;
};

mv_discrimhist_guide_Processor::mv_discrimhist_guide_Processor()
{
    d = new mv_discrimhist_guide_ProcessorPrivate;
    d->q = this;

    this->setName("mv_discrimhist_guide");
    this->setVersion("0.156");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("num_histograms", "clusters_to_exclude");
    this->setOptionalParameters("method");
}

mv_discrimhist_guide_Processor::~mv_discrimhist_guide_Processor()
{
    delete d;
}

bool mv_discrimhist_guide_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool mv_discrimhist_guide_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString output_path = params["output"].toString();
    int num_histograms = params["num_histograms"].toInt();
    QStringList strlist = params["clusters_to_exclude"].toString().split(",", QString::SkipEmptyParts);
    QSet<int> clusters_to_exclude = MLUtil::stringListToIntList(strlist).toSet();

    mv_discrimhist_guide_opts opts;
    opts.num_histograms = num_histograms;
    opts.clusters_to_exclude = clusters_to_exclude;
    opts.method = params.value("method").toString();
    if (opts.method.isEmpty())
        opts.method = "centroid";
    printf("Using discrim method: %s\n", opts.method.toUtf8().data());
    return mv_discrimhist_guide(timeseries_path, firings_path, output_path, opts);
}
