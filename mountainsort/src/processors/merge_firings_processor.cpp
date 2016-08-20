/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#include "merge_firings_processor.h"
#include "merge_firings.h"

class merge_firings_ProcessorPrivate {
public:
    merge_firings_Processor* q;
};

merge_firings_Processor::merge_firings_Processor()
{
    d = new merge_firings_ProcessorPrivate;
    d->q = this;

    this->setName("merge_firings");
    this->setVersion("0.1");
    this->setInputFileParameters("firings1", "firings2");
    this->setOutputFileParameters("firings_merged", "confusion_matrix", "optimal_label_map");
    this->setRequiredParameters("max_matching_offset");
}

merge_firings_Processor::~merge_firings_Processor()
{
    delete d;
}

bool merge_firings_Processor::check(const QMap<QString, QVariant>& params)
{
    if (!this->checkParameters(params))
        return false;
    return true;
}

bool merge_firings_Processor::run(const QMap<QString, QVariant>& params)
{
    QString firings1_path = params["firings1"].toString();
    QString firings2_path = params["firings2"].toString();
    QString firings_merged_path = params["firings_merged"].toString();
    QString confusion_matrix_path = params["confusion_matrix"].toString();
    QString optimal_label_map_path = params["optimal_label_map"].toString();
    int max_matching_offset = params["max_matching_offset"].toInt();
    return merge_firings(firings1_path, firings2_path, firings_merged_path, confusion_matrix_path, optimal_label_map_path, max_matching_offset);
}
