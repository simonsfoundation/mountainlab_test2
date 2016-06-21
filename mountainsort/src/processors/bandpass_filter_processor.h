/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef BANDPASS_FILTER_PROCESSOR_H
#define BANDPASS_FILTER_PROCESSOR_H

#include "msprocessor.h"

class bandpass_filter_ProcessorPrivate;
class bandpass_filter_Processor : public MSProcessor {
public:
    friend class bandpass_filter_ProcessorPrivate;
    bandpass_filter_Processor();
    virtual ~bandpass_filter_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    bandpass_filter_ProcessorPrivate* d;
};

#endif // BANDPASS_FILTER_PROCESSOR_H
