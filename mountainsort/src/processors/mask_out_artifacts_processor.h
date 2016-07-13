/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MASK_OUT_ARTIFACTS_PROCESSOR_H
#define MASK_OUT_ARTIFACTS_PROCESSOR_H

#include "msprocessor.h"

class mask_out_artifacts_ProcessorPrivate;
class mask_out_artifacts_Processor : public MSProcessor {
public:
    friend class mask_out_artifacts_ProcessorPrivate;
    mask_out_artifacts_Processor();
    virtual ~mask_out_artifacts_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mask_out_artifacts_ProcessorPrivate* d;
};

#endif // MASK_OUT_ARTIFACTS_PROCESSOR_H
