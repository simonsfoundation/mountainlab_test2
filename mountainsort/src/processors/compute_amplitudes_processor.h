#ifndef COMPUTE_AMPLITUDES_PROCESSOR_H
#define COMPUTE_AMPLITUDES_PROCESSOR_H

#include "msprocessor.h"

class compute_amplitudes_ProcessorPrivate;
class compute_amplitudes_Processor : public MSProcessor {
public:
    friend class compute_amplitudes_ProcessorPrivate;
    compute_amplitudes_Processor();
    virtual ~compute_amplitudes_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    compute_amplitudes_ProcessorPrivate* d;
};

#endif // COMPUTE_AMPLITUDES_PROCESSOR_H
