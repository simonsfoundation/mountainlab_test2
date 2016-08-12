/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/22/2016
*******************************************************/

#ifndef MERGE_STAGE_H
#define MERGE_STAGE_H

#include <QString>

struct merge_stage_opts {
    int clip_size;
    double min_peak_ratio;
    double min_template_corr_coef;
};

bool merge_stage(const QString& timeseries, const QString& firings, const QString& firings_out, const merge_stage_opts& opts);

#endif // MERGE_STAGE_H
