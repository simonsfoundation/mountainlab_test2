/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef isolation_metrics_PROCESSOR_H
#define isolation_metrics_PROCESSOR_H

#include "msprocessor.h"

class isolation_metrics_ProcessorPrivate;
class isolation_metrics_Processor : public MSProcessor {
public:
    friend class isolation_metrics_ProcessorPrivate;
    isolation_metrics_Processor();
    virtual ~isolation_metrics_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    isolation_metrics_ProcessorPrivate* d;
};

#endif // isolation_metrics_PROCESSOR_H
