/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/5/2016
*******************************************************/

#ifndef SYNTHESIZE1_H
#define SYNTHESIZE1_H

#include <QString>

struct synthesize1_opts {
    double samplerate = 30000;
    long N = 1000;
    double noise_level = 1;
    int waveforms_oversamp = 1;
};

bool synthesize1(
    const QString& waveforms_in_path,
    const QString& info_in_path,
    const QString& timeseries_out_path,
    const QString& firings_true_path,
    const synthesize1_opts& opts);

#endif // SYNTHESIZE1_H
