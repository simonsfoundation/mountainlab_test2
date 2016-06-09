#ifndef EXTRACT_CHANNEL_VALUES_PROCESSOR_H
#define EXTRACT_CHANNEL_VALUES_PROCESSOR_H

#include "msprocessor.h"

class extract_channel_values_ProcessorPrivate;
class extract_channel_values_Processor : public MSProcessor {
public:
    friend class extract_channel_values_ProcessorPrivate;
    extract_channel_values_Processor();
    virtual ~extract_channel_values_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    extract_channel_values_ProcessorPrivate* d;
};

#endif // EXTRACT_CHANNEL_VALUES_PROCESSOR_H
