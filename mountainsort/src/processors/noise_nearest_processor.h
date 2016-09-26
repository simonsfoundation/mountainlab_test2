/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef NOISE_NEAREST_PROCESSOR_H
#define NOISE_NEAREST_PROCESSOR_H

#include "msprocessor.h"

class noise_nearest_ProcessorPrivate;
class noise_nearest_Processor : public MSProcessor {
public:
    friend class noise_nearest_ProcessorPrivate;
    noise_nearest_Processor();
    virtual ~noise_nearest_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    noise_nearest_ProcessorPrivate* d;
};

#endif // NOISE_NEAREST_PROCESSOR_H
