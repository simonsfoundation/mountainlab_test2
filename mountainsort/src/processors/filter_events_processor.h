/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#ifndef FILTER_EVENTS_PROCESSOR_H
#define FILTER_EVENTS_PROCESSOR_H

#include "msprocessor.h"

class filter_events_ProcessorPrivate;
class filter_events_Processor : public MSProcessor {
public:
    friend class filter_events_ProcessorPrivate;
    filter_events_Processor();
    virtual ~filter_events_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    filter_events_ProcessorPrivate* d;
};

#endif // FILTER_EVENTS_PROCESSOR_H
