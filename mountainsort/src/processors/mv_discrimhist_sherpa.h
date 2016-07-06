#ifndef MV_DISCRIMHIST_H
#define MV_DISCRIMHIST_H

#include <QList>
#include <QString>
#include <QVector>

struct mv_discrimhist_sherpa_opts {
    /// TODO clip_size is hard-coded here
    int clip_size = 80;
    /// TODO num_histograms is hard-coded here
    int num_histograms = 100;
};

bool mv_discrimhist_sherpa(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_sherpa_opts opts);

#endif // MV_DISCRIMHIST_H
