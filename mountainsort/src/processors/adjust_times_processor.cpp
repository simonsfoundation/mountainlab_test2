/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#include "adjust_times_processor.h"
#include "adjust_times.h"

class adjust_times_ProcessorPrivate
{
public:
    adjust_times_Processor *q;
};

adjust_times_Processor::adjust_times_Processor() {
    d=new adjust_times_ProcessorPrivate;
    d->q=this;

    this->setName("adjust_times");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries","detect");
    this->setOutputFileParameters("detect_out");
    this->setRequiredParameters("upsampling_factor","sign");
    this->setOptionalParameters("num_pca_denoise_components","pca_denoise_jiggle");
}

adjust_times_Processor::~adjust_times_Processor() {
    delete d;
}

bool adjust_times_Processor::check(const QMap<QString, QVariant> &params)
{
    if (!this->checkParameters(params)) return false;
    return true;
}

bool adjust_times_Processor::run(const QMap<QString, QVariant> &params)
{
    QString timeseries_path=params["timeseries"].toString();
    QString detect_path=params["detect"].toString();
    QString detect_out_path=params["detect_out"].toString();
    int upsampling_factor=params["upsampling_factor"].toInt();
    int sign=params["sign"].toInt();
    int num_pca_denoise_components=params.value("num_pca_denoise_components").toInt();
    int pca_denoise_jiggle=params.value("pca_denoise_jiggle").toInt();
    adjust_times_opts opts;
    opts.upsampling_factor=upsampling_factor;
    opts.sign=sign;
    opts.num_pca_denoise_components=num_pca_denoise_components;
    opts.pca_denoise_jiggle=pca_denoise_jiggle;
    return adjust_times(timeseries_path,detect_path,detect_out_path,opts);
}

