/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/24/2016
*******************************************************/

#ifndef DETECT3_H
#define DETECT3_H

#include <QString>

struct Detect3_Opts {
    double detect_threshold;
    int detect_interval;
    int clip_size;
    int sign;
    bool individual_channels;
    int beta;
};

bool detect3(const QString& timeseries_path, const QString& detect_path, const Detect3_Opts& opts);

#endif // DETECT3_H
