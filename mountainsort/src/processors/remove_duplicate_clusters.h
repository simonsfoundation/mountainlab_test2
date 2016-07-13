/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef REMOVE_DUPLICATE_CLUSTERS_H
#define REMOVE_DUPLICATE_CLUSTERS_H

#include <QString>

struct remove_duplicate_clusters_Opts {
    int clip_size;
};

bool remove_duplicate_clusters(const QString& timeseries_path, const QString& firings_path, const QString& firings_out_path, const remove_duplicate_clusters_Opts& opts);

#endif // REMOVE_DUPLICATE_CLUSTERS_H
