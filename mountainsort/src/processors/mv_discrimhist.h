#ifndef MV_DISCRIMHIST_H
#define MV_DISCRIMHIST_H

#include <QList>
#include <QString>


struct mv_discrimhist_opts {
    QList<int> clusters;
};

bool mv_discrimhist(QString timeseries_path,QString firings_path,QString output_path,mv_discrimhist_opts opts);


#endif // MV_DISCRIMHIST_H

