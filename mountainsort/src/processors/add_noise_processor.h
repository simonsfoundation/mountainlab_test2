#ifndef ADD_NOISE_PROCESSOR_H
#define ADD_NOISE_PROCESSOR_H

#include "msprocessor.h"

class add_noise_ProcessorPrivate;
class add_noise_Processor : public MSProcessor {
public:
    friend class add_noise_ProcessorPrivate;
    add_noise_Processor();
    virtual ~add_noise_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    add_noise_ProcessorPrivate* d;
};

#endif // ADD_NOISE_PROCESSOR_H
