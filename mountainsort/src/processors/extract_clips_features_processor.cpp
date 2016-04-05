/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/5/2016
*******************************************************/


#include "extract_clips_features_processor.h"
#include "extract_clips.h"

class extract_clips_features_ProcessorPrivate
{
public:
    extract_clips_features_Processor *q;
};

extract_clips_features_Processor::extract_clips_features_Processor() {
    d=new extract_clips_features_ProcessorPrivate;
    d->q=this;

    this->setName("extract_clips_features");
    this->setVersion("0.1");
    this->setInputFileParameters("timeseries","firings");
    this->setOutputFileParameters("features");
    this->setRequiredParameters("clip_size","num_features");
}

extract_clips_features_Processor::~extract_clips_features_Processor() {
    delete d;
}

bool extract_clips_features_Processor::check(const QMap<QString, QVariant> &params)
{
    if (!this->checkParameters(params)) return false;
    return true;
}

bool extract_clips_features_Processor::run(const QMap<QString, QVariant> &params)
{
    QString timeseries_path=params["timeseries"].toString();
    QString firings_path=params["firings"].toString();
    QString features_path=params["features"].toString();
    int clip_size=params["clip_size"].toInt();
    int num_features=params["num_features"].toInt();
    return extract_clips_features(timeseries_path,firings_path,features_path,clip_size,num_features);
}

