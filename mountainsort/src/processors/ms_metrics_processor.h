/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef ms_metrics_PROCESSOR_H
#define ms_metrics_PROCESSOR_H

#include "msprocessor.h"

class ms_metrics_ProcessorPrivate;
class ms_metrics_Processor : public MSProcessor {
public:
    friend class ms_metrics_ProcessorPrivate;
    ms_metrics_Processor();
    virtual ~ms_metrics_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    ms_metrics_ProcessorPrivate* d;
};

#endif // ms_metrics_PROCESSOR_H
