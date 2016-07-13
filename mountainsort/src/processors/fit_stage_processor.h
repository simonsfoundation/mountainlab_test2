/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#ifndef FIT_STAGE_PROCESSOR_H
#define FIT_STAGE_PROCESSOR_H

#include "msprocessor.h"

class fit_stage_ProcessorPrivate;
class fit_stage_Processor : public MSProcessor {
public:
    friend class fit_stage_ProcessorPrivate;
    fit_stage_Processor();
    virtual ~fit_stage_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    fit_stage_ProcessorPrivate* d;
};

#endif // FIT_STAGE_PROCESSOR_H
