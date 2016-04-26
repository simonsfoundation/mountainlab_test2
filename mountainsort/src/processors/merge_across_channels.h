/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#ifndef MERGE_ACROSS_CHANNELS_H
#define MERGE_ACROSS_CHANNELS_H

#include <QString>

struct merge_across_channels_opts {
    double min_peak_ratio;
    int max_dt;
    double min_coinc_frac;
    int min_coinc_num;
    double max_corr_stddev;
    double min_template_corr_coef;
    int clip_size;
};

bool merge_across_channels(const QString& timeseries, const QString& firings, const QString& firings_out, const merge_across_channels_opts& opts);

#endif // MERGE_ACROSS_CHANNELS_H
