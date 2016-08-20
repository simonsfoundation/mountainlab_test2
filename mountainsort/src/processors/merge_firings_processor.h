#ifndef MERGE_FIRINGS_PROCESSOR_H
#define MERGE_FIRINGS_PROCESSOR_H

#include "msprocessor.h"

class merge_firings_ProcessorPrivate;
class merge_firings_Processor : public MSProcessor {
public:
    friend class merge_firings_ProcessorPrivate;
    merge_firings_Processor();
    virtual ~merge_firings_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    merge_firings_ProcessorPrivate* d;
};

#endif // MERGE_FIRINGS_PROCESSOR_H
