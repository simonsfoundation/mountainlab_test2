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
    this->setVersion("0.12");
    this->setInputFileParameters("firings1", "firings2");
    this->setOutputFileParameters("output");
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
    int max_matching_offset = params["max_matching_offset"].toInt();
    return confusion_matrix(firings1_path.toLatin1().data(), firings2_path.toLatin1().data(), output_path.toLatin1().data(), max_matching_offset);
}
