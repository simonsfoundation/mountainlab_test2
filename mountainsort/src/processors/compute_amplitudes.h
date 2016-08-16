#ifndef COMPUTE_AMPLITUDES_H
#define COMPUTE_AMPLITUDES_H

#include <QString>

struct compute_amplitudes_opts {
    int clip_size = 50;
};

bool compute_amplitudes(QString timeseries_path, QString firings_path, QString firings_out_path, compute_amplitudes_opts opts);

#endif // COMPUTE_AMPLITUDES_H
