/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/
#include <QString>

#include "compute_templates_processor.h"
#include "compute_templates.h"

class compute_templates_ProcessorPrivate {
public:
    compute_templates_Processor* q;
};

compute_templates_Processor::compute_templates_Processor()
{
    d = new compute_templates_ProcessorPrivate;
    d->q = this;

    this->setName("compute_templates");
    this->setVersion("0.11");
    this->setInputFileParameters("timeseries", "firings");
    this->setOutputFileParameters("templates");
    this->setRequiredParameters("clip_size");
}

compute_templates_Processor::~compute_templates_Processor()
{
    delete d;
}

bool compute_templates_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool compute_templates_Processor::run(const QMap<QString, QVariant>& params)
{
    QString timeseries = params["timeseries"].toString();
    QString firings = params["firings"].toString();
    QString templates = params["templates"].toString();
    int clip_size = params["clip_size"].toInt();
    return compute_templates(timeseries, firings, templates, clip_size);
}
