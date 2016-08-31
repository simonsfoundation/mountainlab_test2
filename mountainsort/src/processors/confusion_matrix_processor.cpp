/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#include "confusion_matrix_processor.h"
#include "confusion_matrix.h"

class confusion_matrix_ProcessorPrivate {
public:
    confusion_matrix_Processor* q;
};

confusion_matrix_Processor::confusion_matrix_Processor()
{
    d = new confusion_matrix_ProcessorPrivate;
    d->q = this;

    this->setName("confusion_matrix");
    this->setVersion("0.14");
    this->setInputFileParameters("firings1", "firings2");
    this->setOutputFileParameters("output", "optimal_assignments", "event_correspondence");
    this->setRequiredParameters("max_matching_offset");
}

confusion_matrix_Processor::~confusion_matrix_Processor()
{
    delete d;
}

bool confusion_matrix_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool confusion_matrix_Processor::run(const QMap<QString, QVariant>& params)
{
    QString firings1_path = params["firings1"].toString();
    QString firings2_path = params["firings2"].toString();
    QString output_path = params["output"].toString();
    QString optimal_assignments_path = params["optimal_assignments"].toString();
    QString event_correspondence_path = params["event_correspondence"].toString();
    int max_matching_offset = params["max_matching_offset"].toInt();
    return confusion_matrix(firings1_path, firings2_path, output_path, optimal_assignments_path, event_correspondence_path, max_matching_offset);
}
