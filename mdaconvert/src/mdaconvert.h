/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/4/2016
*******************************************************/

#ifndef MDACONVERT_H
#define MDACONVERT_H

#include <QDebug>

struct mdaconvert_opts {
    QString input_path;
    QString input_dtype; // uint16, float32, ...
    QString input_format; // mda, raw, ...

    QString output_path;
    QString output_dtype; // uint16, float32, ...
    QString output_format; // mda, raw, ...

    QList<long> dims;

    bool check_input_file_size = true;
};
bool mdaconvert(const mdaconvert_opts& opts);

#endif // MDACONVERT_H
