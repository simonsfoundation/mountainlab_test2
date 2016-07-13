/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef MV_FIRINGS_FILTER_PROCESSOR_H
#define MV_FIRINGS_FILTER_PROCESSOR_H

#include "msprocessor.h"

class mv_firings_filter_ProcessorPrivate;
class mv_firings_filter_Processor : public MSProcessor {
public:
    friend class mv_firings_filter_ProcessorPrivate;
    mv_firings_filter_Processor();
    virtual ~mv_firings_filter_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    mv_firings_filter_ProcessorPrivate* d;
};

#endif // MV_FIRINGS_FILTER_PROCESSOR_H
