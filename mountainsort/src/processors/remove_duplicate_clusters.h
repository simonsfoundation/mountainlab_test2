#ifndef REMOVE_DUPLICATE_CLUSTERS_H
#define REMOVE_DUPLICATE_CLUSTERS_H

#include <QString>

bool remove_duplicate_clusters(const QString &firings_path,const QString &firings_out_path,int max_dt=6,float overlap_threshold=0.25);

#endif // REMOVE_DUPLICATE_CLUSTERS_H
