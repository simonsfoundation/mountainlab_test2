#ifndef MV_DISCRIMHIST_SHERPA_PROCESSOR_H
#define MV_DISCRIMHIST_SHERPA_PROCESSOR_H

#include "msprocessor.h"

class mv_discrimhist_sherpa_ProcessorPrivate;
class mv_discrimhist_sherpa_Processor : public MSProcessor {
public:
    friend class mv_discrimhist_sherpa_ProcessorPrivate;
    mv_discrimhist_sherpa_Processor();
    virtual ~mv_discrimhist_sherpa_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mv_discrimhist_sherpa_ProcessorPrivate* d;
};

#endif // MV_DISCRIMHIST_SHERPA_PROCESSOR_H
