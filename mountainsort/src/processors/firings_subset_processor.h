/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/26/2016
*******************************************************/

#ifndef FIRINGS_SUBSET_PROCESSOR_H
#define FIRINGS_SUBSET_PROCESSOR_H

#include "msprocessor.h"

class firings_subset_ProcessorPrivate;
class firings_subset_Processor : public MSProcessor {
public:
    friend class firings_subset_ProcessorPrivate;
    firings_subset_Processor();
    virtual ~firings_subset_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    firings_subset_ProcessorPrivate* d;
};

#endif // FIRINGS_SUBSET_PROCESSOR_H
