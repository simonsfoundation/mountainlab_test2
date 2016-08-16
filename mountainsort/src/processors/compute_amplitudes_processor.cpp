#include "compute_amplitudes_processor.h"

#include "compute_amplitudes_processor.h"
#include "compute_amplitudes.h"

class compute_amplitudes_ProcessorPrivate {
public:
    compute_amplitudes_Processor* q;
};

compute_amplitudes_Processor::compute_amplitudes_Processor()
{
    d = new compute_amplitudes_ProcessorPrivate;
    d->q = this;

    this->setName("compute_amplitudes");
    this->setVersion("0.21");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("firings_out");
}

compute_amplitudes_Processor::~compute_amplitudes_Processor()
{
    delete d;
}

bool compute_amplitudes_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool compute_amplitudes_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries = params["timeseries"].toString();
    QString firings = params["firings"].toString();
    QString firings_out = params["firings_out"].toString();
    compute_amplitudes_opts opts;
    opts.clip_size = 40;
    return compute_amplitudes(timeseries, firings, firings_out, opts);
}
