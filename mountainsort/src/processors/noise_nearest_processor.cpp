/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "noise_nearest_processor.h"
#include "noise_nearest.h"

class noise_nearest_ProcessorPrivate {
public:
    noise_nearest_Processor* q;
};

noise_nearest_Processor::noise_nearest_Processor()
{
    d = new noise_nearest_ProcessorPrivate;
    d->q = this;

    this->setName("noise_nearest");
    this->setVersion("0.16");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("confusion_matrix");
    this->setRequiredParameters("clip_size");
    this->setRequiredParameters("add_noise_level");
    this->setOptionalParameters("cluster_numbers");
}

noise_nearest_Processor::~noise_nearest_Processor()
{
    delete d;
}

bool noise_nearest_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool noise_nearest_Processor::run(const QMap<QString, QVariant>& params)
{
    noise_nearest_opts opts;
    opts.clip_size = params["clip_size"].toInt();
    opts.add_noise_level = params["add_noise_level"].toDouble();
    QStringList cluster_numbers_str = params["cluster_numbers"].toString().split(",", QString::SkipEmptyParts);
    foreach (QString num, cluster_numbers_str) {
        opts.cluster_numbers << num.toInt();
    }

    QString timeseries_path = params["timeseries"].toString();
    QString firings_path = params["firings"].toString();
    QString confusion_matrix_path = params["confusion_matrix"].toString();

    return NoiseNearest::noise_nearest(timeseries_path, firings_path, confusion_matrix_path, opts);
}
