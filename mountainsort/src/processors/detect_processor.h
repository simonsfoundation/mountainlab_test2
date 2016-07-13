/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DETECT_PROCESSOR_H
#define DETECT_PROCESSOR_H

#include "msprocessor.h"

class detect_ProcessorPrivate;
class detect_Processor : public MSProcessor {
public:
    friend class detect_ProcessorPrivate;
    detect_Processor();
    virtual ~detect_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    detect_ProcessorPrivate* d;
};

#endif // DETECT_PROCESSOR_H
