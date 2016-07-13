/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef EXAMPLE_PROCESSOR_H
#define EXAMPLE_PROCESSOR_H

#include "msprocessor.h"

class example_ProcessorPrivate;
class example_Processor : public MSProcessor {
public:
    friend class example_ProcessorPrivate;
    example_Processor();
    virtual ~example_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    example_ProcessorPrivate* d;
};

#endif // EXAMPLE_PROCESSOR_H
