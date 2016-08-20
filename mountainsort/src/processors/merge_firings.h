#ifndef MERGE_FIRINGS_H
#define MERGE_FIRINGS_H

#include <QString>

bool merge_firings(
    QString firings1_path, QString firings2_path,
    QString firings_merged_path, QString confusion_matrix_path,
    QString optimal_label_map_path, int max_matching_offset);

#endif // MERGE_FIRINGS_H
