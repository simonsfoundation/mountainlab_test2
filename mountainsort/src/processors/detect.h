/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef DETECT_H
#define DETECT_H

#include <QString>
#include <QList>

struct Detect_Opts {
    double detect_threshold;
    int detect_interval;
    int clip_size;
    int sign;
    bool individual_channels;
};

bool detect(const QString& timeseries_path, const QString& detect_path, const Detect_Opts& opts);
//the following used by detect3()
QVector<double> do_detect(const QVector<double>& vals, int detect_interval, double detect_threshold);

#endif // DETECT_H
