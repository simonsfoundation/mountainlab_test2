#ifndef CONFUSION_MATRIX_H
#define CONFUSION_MATRIX_H

#include <QString>

bool confusion_matrix(QString firings1_path, QString firings2_path, QString output_path, QString optimal_assignments_path, int max_matching_offset);

#endif // CONFUSION_MATRIX_H
