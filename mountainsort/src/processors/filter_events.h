/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/13/2016
*******************************************************/

#ifndef FILTER_EVENTS_H
#define FILTER_EVENTS_H

#include <QString>

struct filter_events_opts {
    double detectability_threshold;
    double outlier_threshold;
};

bool filter_events(QString firings_path, QString firings_out_path, filter_events_opts opts);

#endif // FILTER_EVENTS_H
