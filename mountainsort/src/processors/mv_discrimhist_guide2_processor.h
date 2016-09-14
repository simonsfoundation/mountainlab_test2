#ifndef MV_DISCRIMHIST_GUIDE2_PROCESSOR_H
#define MV_DISCRIMHIST_GUIDE2_PROCESSOR_H

#include "msprocessor.h"

class mv_discrimhist_guide2_ProcessorPrivate;
class mv_discrimhist_guide2_Processor : public MSProcessor {
public:
    friend class mv_discrimhist_guide2_ProcessorPrivate;
    mv_discrimhist_guide2_Processor();
    virtual ~mv_discrimhist_guide2_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mv_discrimhist_guide2_ProcessorPrivate* d;
};

#endif // MV_DISCRIMHIST_GUIDE2PROCESSOR_H
