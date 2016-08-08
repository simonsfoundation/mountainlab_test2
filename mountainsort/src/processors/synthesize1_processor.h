/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/5/2016
*******************************************************/

#ifndef SYNTHESIZE1_PROCESSOR_H
#define SYNTHESIZE1_PROCESSOR_H

#include "msprocessor.h"

class synthesize1_ProcessorPrivate;
class synthesize1_Processor : public MSProcessor {
public:
    friend class synthesize1_ProcessorPrivate;
    synthesize1_Processor();
    virtual ~synthesize1_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    synthesize1_ProcessorPrivate* d;
};

#endif // SYNTHESIZE1_PROCESSOR_H
