/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef BRANCH_CLUSTER_V2_PROCESSOR_H
#define BRANCH_CLUSTER_V2_PROCESSOR_H

#include "msprocessor.h"

class branch_cluster_v2_ProcessorPrivate;
class branch_cluster_v2_Processor : public MSProcessor {
public:
    friend class branch_cluster_v2_ProcessorPrivate;
    branch_cluster_v2_Processor();
    virtual ~branch_cluster_v2_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    branch_cluster_v2_ProcessorPrivate* d;
};

#endif // BRANCH_CLUSTER_V2_PROCESSOR_H
