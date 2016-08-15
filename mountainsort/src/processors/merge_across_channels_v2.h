/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#ifndef MERGE_ACROSS_CHANNELS_V2_H
#define MERGE_ACROSS_CHANNELS_V2_H

#include <QString>

struct merge_across_channels_v2_opts {
    int clip_size = 100;
    double min_peak_ratio_to_consider = 0.7;
    double event_fraction_threshold = 0.5;
};

bool merge_across_channels_v2(const QString& timeseries, const QString& firings, const QString& firings_out, const merge_across_channels_v2_opts& opts);

#endif // MERGE_ACROSS_CHANNELS_V2_H
