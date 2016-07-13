/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef WHITEN_PROCESSOR_H
#define WHITEN_PROCESSOR_H

#include "msprocessor.h"

class whiten_ProcessorPrivate;
class whiten_Processor : public MSProcessor {
public:
    friend class whiten_ProcessorPrivate;
    whiten_Processor();
    virtual ~whiten_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    whiten_ProcessorPrivate* d;
};

#endif // WHITEN_PROCESSOR_H
