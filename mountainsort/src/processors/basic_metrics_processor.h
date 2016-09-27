/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef basic_metrics_PROCESSOR_H
#define basic_metrics_PROCESSOR_H

#include "msprocessor.h"

class basic_metrics_ProcessorPrivate;
class basic_metrics_Processor : public MSProcessor {
public:
    friend class basic_metrics_ProcessorPrivate;
    basic_metrics_Processor();
    virtual ~basic_metrics_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    basic_metrics_ProcessorPrivate* d;
};

#endif // basic_metrics_PROCESSOR_H
