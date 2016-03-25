/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#ifndef ADJUST_TIMES_PROCESSOR_H
#define ADJUST_TIMES_PROCESSOR_H

#include "msprocessor.h"

class adjust_times_ProcessorPrivate;
class adjust_times_Processor : public MSProcessor
{
public:
    friend class adjust_times_ProcessorPrivate;
    adjust_times_Processor();
    virtual ~adjust_times_Processor();

    bool check(const QMap<QString,QVariant> &params);
    bool run(const QMap<QString,QVariant> &params);
private:
    adjust_times_ProcessorPrivate *d;
};

#endif // ADJUST_TIMES_PROCESSOR_H

