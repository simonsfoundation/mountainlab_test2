#ifndef MV_DISCRIMHIST_GUIDE2_H
#define MV_DISCRIMHIST_GUIDE2_H

#include <QList>
#include <QSet>
#include <QString>
#include <QVector>

struct mv_discrimhist_guide2_opts {
    /// TODO clip_size is hard-coded here
    int clip_size = 80;
    double add_noise_level = 1;
    QList<int> cluster_numbers;
    int max_comparisons_per_cluster = 4;
};

bool mv_discrimhist_guide2(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_guide2_opts opts);

#endif // MV_DISCRIMHIST_GUIDE2_H
