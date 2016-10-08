/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/13/2016
*******************************************************/
#ifndef NOISE_NEAREST_H
#define NOISE_NEAREST_H

#include <QList>
#include <diskreadmda.h>
#include <diskreadmda32.h>

struct noise_nearest_opts {
    int clip_size = 50;
    QList<int> cluster_numbers;
    double add_noise_level = 0.25;
};

namespace NoiseNearest {
Mda compute_isolation_matrix(QString timeseries_path, QString firings_path, noise_nearest_opts opts);
bool noise_nearest(QString timeseries, QString firings, QString confusion_matrix, noise_nearest_opts opts);
}

#endif // NOISE_NEAREST_H
