/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#ifndef MERGE_LABELS_H
#define MERGE_LABELS_H

#include <QString>

struct merge_labels_opts {
    double merge_threshold;
    int clip_size;
};

bool merge_labels(QString timeseries_path, QString firings_path, QString firings_out_path, merge_labels_opts opts);

#endif // MERGE_LABELS_H
