/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef COMPUTE_DETECTABILITY_SCORES_PROCESSOR_H
#define COMPUTE_DETECTABILITY_SCORES_PROCESSOR_H

#include "msprocessor.h"

class compute_detectability_scores_ProcessorPrivate;
class compute_detectability_scores_Processor : public MSProcessor {
public:
    friend class compute_detectability_scores_ProcessorPrivate;
    compute_detectability_scores_Processor();
    virtual ~compute_detectability_scores_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    compute_detectability_scores_ProcessorPrivate* d;
};

#endif // COMPUTE_DETECTABILITY_SCORES_PROCESSOR_H
