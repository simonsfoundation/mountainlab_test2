/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#ifndef MERGE_LABELS_PROCESSOR_H
#define MERGE_LABELS_PROCESSOR_H

#include "msprocessor.h"

class merge_labels_ProcessorPrivate;
class merge_labels_Processor : public MSProcessor {
public:
    friend class merge_labels_ProcessorPrivate;
    merge_labels_Processor();
    virtual ~merge_labels_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    merge_labels_ProcessorPrivate* d;
};

#endif // MERGE_LABELS_PROCESSOR_H
