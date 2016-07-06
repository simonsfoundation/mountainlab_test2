#include "mv_discrimhist_sherpa_processor.h"
#include "mv_discrimhist_sherpa.h"

class mv_discrimhist_sherpa_ProcessorPrivate {
public:
    mv_discrimhist_sherpa_Processor* q;
};

mv_discrimhist_sherpa_Processor::mv_discrimhist_sherpa_Processor()
{
    d = new mv_discrimhist_sherpa_ProcessorPrivate;
    d->q = this;

    this->setName("mv_discrimhist_sherpa");
    this->setVersion("0.11");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("output");
    this->setRequiredParameters("num_histograms");
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

    mv_discrimhist_sherpa_opts opts;
    opts.num_histograms = num_histograms;
    return mv_discrimhist_sherpa(timeseries_path, firings_path, output_path, opts);
}
