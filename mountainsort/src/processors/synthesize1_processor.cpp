/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/5/2016
*******************************************************/

#include "synthesize1_processor.h"
#include "synthesize1.h"

#include <QFile>
#include <QFileInfo>
#include "mlcommon.h"

class synthesize1_ProcessorPrivate {
public:
    synthesize1_Processor* q;
};

synthesize1_Processor::synthesize1_Processor()
{
    d = new synthesize1_ProcessorPrivate;
    d->q = this;

    this->setName("synthesize1");
    this->setVersion("0.1");
    this->setInputFileParameters("waveforms", "info");
    this->setOutputFileParameters("timeseries_out");
    this->setRequiredParameters("N");
    this->setOptionalParameters("noise_level");
}

synthesize1_Processor::~synthesize1_Processor()
{
    delete d;
}

bool synthesize1_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool synthesize1_Processor::run(const QMap<QString, QVariant>& params)
{
    QString waveforms_path = params["waveforms"].toString();
    QString info_path = params["info"].toString();
    QString timeseries_out_path = params["timeseries_out"].toString();

    synthesize1_opts opts;
    opts.N = params["N"].toLongLong();
    opts.noise_level = params.value("noise_level", 1).toDouble();

    return synthesize1(waveforms_path, info_path, timeseries_out_path, opts);
}
