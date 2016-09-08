/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef BRANCH_CLUSTER_V3_PROCESSOR_H
#define BRANCH_CLUSTER_V3_PROCESSOR_H

#include "msprocessor.h"

class branch_cluster_v3_ProcessorPrivate;
class branch_cluster_v3_Processor : public MSProcessor {
public:
    friend class branch_cluster_v3_ProcessorPrivate;
    branch_cluster_v3_Processor();
    virtual ~branch_cluster_v3_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    branch_cluster_v3_ProcessorPrivate* d;
};

#endif // BRANCH_CLUSTER_V3_PROCESSOR_H
