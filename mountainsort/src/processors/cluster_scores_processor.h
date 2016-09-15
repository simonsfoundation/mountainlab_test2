/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#ifndef CLUSTER_SCORES_PROCESSOR_H
#define CLUSTER_SCORES_PROCESSOR_H

#include "msprocessor.h"

class cluster_scores_ProcessorPrivate;
class cluster_scores_Processor : public MSProcessor {
public:
    friend class cluster_scores_ProcessorPrivate;
    cluster_scores_Processor();
    virtual ~cluster_scores_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    cluster_scores_ProcessorPrivate* d;
};

#endif // CLUSTER_SCORES_PROCESSOR_H
