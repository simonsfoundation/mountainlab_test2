#ifndef MV_DISCRIMHIST_GUIDE_PROCESSOR_H
#define MV_DISCRIMHIST_GUIDE_PROCESSOR_H

#include "msprocessor.h"

class mv_discrimhist_guide_ProcessorPrivate;
class mv_discrimhist_guide_Processor : public MSProcessor {
public:
    friend class mv_discrimhist_guide_ProcessorPrivate;
    mv_discrimhist_guide_Processor();
    virtual ~mv_discrimhist_guide_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mv_discrimhist_guide_ProcessorPrivate* d;
};

#endif // MV_DISCRIMHIST_GUIDE_PROCESSOR_H
