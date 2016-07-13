/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef REMOVE_DUPLICATE_CLUSTERS_PROCESSOR_H
#define REMOVE_DUPLICATE_CLUSTERS_PROCESSOR_H

#include "msprocessor.h"

class remove_duplicate_clusters_ProcessorPrivate;
class remove_duplicate_clusters_Processor : public MSProcessor {
public:
    friend class remove_duplicate_clusters_ProcessorPrivate;
    remove_duplicate_clusters_Processor();
    virtual ~remove_duplicate_clusters_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    remove_duplicate_clusters_ProcessorPrivate* d;
};

#endif // REMOVE_DUPLICATE_CLUSTERS_PROCESSOR_H
