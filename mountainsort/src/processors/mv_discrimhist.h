#ifndef MV_DISCRIMHIST_H
#define MV_DISCRIMHIST_H

#include <QList>
#include <QString>
#include <QVector>

struct mv_discrimhist_opts {
    QVector<int> clusters;
    int clip_size;
};

bool mv_discrimhist(QString timeseries_path, QString firings_path, QString output_path, mv_discrimhist_opts opts);

#endif // MV_DISCRIMHIST_H
