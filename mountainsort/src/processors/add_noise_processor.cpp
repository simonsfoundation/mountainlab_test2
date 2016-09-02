#include "add_noise_processor.h"

#include <mda32.h>
#include "synthesize1.h"

class add_noise_ProcessorPrivate {
public:
    add_noise_Processor* q;
};

add_noise_Processor::add_noise_Processor()
{
    d = new add_noise_ProcessorPrivate;
    d->q = this;

    this->setName("add_noise");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries");
    this->setOutputFileParameters("timeseries_out");
    this->setRequiredParameters("noise_level");
}

add_noise_Processor::~add_noise_Processor()
{
    delete d;
}

bool add_noise_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool add_noise_Processor::run(const QMap<QString, QVariant>& params)
{
    QString input = params["timeseries"].toString();
    QString output = params["timeseries_out"].toString();
    double noise_level = params["noise_level"].toDouble();
    Mda32 X(input);
    Mda32 noise(X.N1(), X.N2());
    generate_randn(X.totalSize(), noise.dataPtr());
    for (long i = 0; i < X.totalSize(); i++) {
        X.set(X.get(i) + noise.get(i) * noise_level, i);
    }
    return X.write32(output);
}
