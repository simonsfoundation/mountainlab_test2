#ifndef MVUTILS_H
#define MVUTILS_H

#include "diskarraymodel.h"
#include "diskreadmda.h"
#include <QList>
#include <QColor>

struct MVEvent {
	double time;
	int label;
};

struct Epoch {
    QString name;
    double t_begin;
    double t_end;
};
QList<Epoch> read_epochs(const QString &path);

Mda compute_mean_waveform(DiskArrayModel *C);
Mda compute_mean_stdev_waveform(DiskArrayModel *C);
Mda compute_features(DiskArrayModel *C);
Mda compute_features(const QList<DiskArrayModel *> &C);
QColor get_heat_map_color(double val);


#endif // MVUTILS_H

