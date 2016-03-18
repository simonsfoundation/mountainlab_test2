#ifndef OUTLIER_SCORES_V1_H
#define OUTLIER_SCORES_V1_H

#include <QString>

struct Compute_Outlier_Scores_Opts {
    int clip_size;
    int min_shell_size;
    double shell_increment;
};

bool compute_outlier_scores(const QString &signal_path,const QString &firings_path,const QString &firings_out_path,const Compute_Outlier_Scores_Opts &opts);

#endif // OUTLIER_SCORES_V1_H
