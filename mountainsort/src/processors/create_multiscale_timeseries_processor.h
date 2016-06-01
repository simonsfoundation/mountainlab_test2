#ifndef CREATE_MULTISCALE_TIMESERIES_PROCESSOR_H
#define CREATE_MULTISCALE_TIMESERIES_PROCESSOR_H

#include "msprocessor.h"

class create_multiscale_timeseries_ProcessorPrivate;
class create_multiscale_timeseries_Processor : public MSProcessor {
public:
    friend class create_multiscale_timeseries_ProcessorPrivate;
    create_multiscale_timeseries_Processor();
    virtual ~create_multiscale_timeseries_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    create_multiscale_timeseries_ProcessorPrivate* d;
};

#endif // CREATE_MULTISCALE_TIMESERIES_PROCESSOR_H
