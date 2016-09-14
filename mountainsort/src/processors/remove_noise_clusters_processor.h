/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#ifndef REMOVE_NOISE_CLUSTERS_PROCESSOR_H
#define REMOVE_NOISE_CLUSTERS_PROCESSOR_H

#include "msprocessor.h"

class remove_noise_clusters_ProcessorPrivate;
class remove_noise_clusters_Processor : public MSProcessor {
public:
    friend class remove_noise_clusters_ProcessorPrivate;
    remove_noise_clusters_Processor();
    virtual ~remove_noise_clusters_Processor();

    bool check(const QMap<QString, QVariant>& params);
    bool run(const QMap<QString, QVariant>& params);

private:
    remove_noise_clusters_ProcessorPrivate* d;
};

#endif // REMOVE_NOISE_CLUSTERS_PROCESSOR_H
