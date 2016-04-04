/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#include "mv_firings_filter_processor.h"
#include "mv_firings_filter.h"

class mv_firings_filter_ProcessorPrivate
{
public:
    mv_firings_filter_Processor *q;
};

mv_firings_filter_Processor::mv_firings_filter_Processor() {
    d=new mv_firings_filter_ProcessorPrivate;
    d->q=this;

    this->setName("mv_firings_filter");
    this->setVersion("0.1");
    this->setInputFileParameters("firings");
    this->setOutputFileParameters("firings_out","original_cluster_numbers");
    this->setRequiredParameters("use_shell_split","shell_width","min_per_shell");
    this->setRequiredParameters("use_event_filter","min_amplitude","max_outlier_score");
}

mv_firings_filter_Processor::~mv_firings_filter_Processor() {
    delete d;
}

bool mv_firings_filter_Processor::check(const QMap<QString, QVariant> &params)
{
    if (!this->checkParameters(params)) return false;
    return true;
}

bool mv_firings_filter_Processor::run(const QMap<QString, QVariant> &params)
{
    QString firings=params["firings"].toString();
    QString firings_out=params["firings_out"].toString();
    QString original_cluster_numbers=params["original_cluster_numbers"].toString();

    mv_firings_filter_opts opts;
    opts.use_shell_split=params["use_shell_split"].toBool();
    opts.shell_width=params["shell_width"].toDouble();
    opts.min_per_shell=params["min_per_shell"].toInt();
    opts.use_event_filter=params["use_event_filter"].toBool();
    opts.min_amplitude=params["min_amplitude"].toDouble();
    opts.max_outlier_score=params["max_outlier_score"].toDouble();

    return mv_firings_filter(firings,firings_out,original_cluster_numbers,opts);
}


