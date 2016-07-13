/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DETECT3_PROCESSOR_H
#define DETECT3_PROCESSOR_H

#include "msprocessor.h"

class detect3_ProcessorPrivate;
class detect3_Processor : public MSProcessor {
public:
    friend class detect3_ProcessorPrivate;
    detect3_Processor();
    virtual ~detect3_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    detect3_ProcessorPrivate* d;
};

#endif // DETECT3_PROCESSOR_H
