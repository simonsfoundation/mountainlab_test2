/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef COMPUTE_OUTLIER_SCORES_PROCESSOR_H
#define COMPUTE_OUTLIER_SCORES_PROCESSOR_H

#include "msprocessor.h"

class compute_outlier_scores_ProcessorPrivate;
class compute_outlier_scores_Processor : public MSProcessor {
public:
    friend class compute_outlier_scores_ProcessorPrivate;
    compute_outlier_scores_Processor();
    virtual ~compute_outlier_scores_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    compute_outlier_scores_ProcessorPrivate* d;
};

#endif // COMPUTE_OUTLIER_SCORES_PROCESSOR_H
