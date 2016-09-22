#ifndef MV_DISCRIMHIST_GUIDE_H
#define MV_DISCRIMHIST_GUIDE_H

#include <QList>
#include <QSet>
#include <QString>
#include <QVector>

struct mv_discrimhist_guide_opts {
    /// TODO clip_size is hard-coded here
    int clip_size = 80;
    int num_histograms = 100;
    QSet<int> clusters_to_exclude;
    QString method = "centroid"; //centroid or svm
    int num_features = 20;
};

bool mv_discrimhist_guide(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_guide_opts opts);

#endif // MV_DISCRIMHIST_H
