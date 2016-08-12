/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#ifndef MERGE_STAGE_PROCESSOR_H
#define MERGE_STAGE_PROCESSOR_H

#include "msprocessor.h"

class merge_stage_ProcessorPrivate;
class merge_stage_Processor : public MSProcessor {
public:
    friend class merge_stage_ProcessorPrivate;
    merge_stage_Processor();
    virtual ~merge_stage_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    merge_stage_ProcessorPrivate* d;
};

#endif // MERGE_STAGE_PROCESSOR_H
