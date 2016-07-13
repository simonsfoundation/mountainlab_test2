/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef MV_FIRINGS_FILTER_H
#define MV_FIRINGS_FILTER_H

#include <QString>

struct mv_firings_filter_opts {
public:
    bool use_shell_split;
    double shell_width;
    int min_per_shell;
    bool use_event_filter;
    double min_amplitude;
    double min_detectability_score;
    double max_outlier_score;
};

bool mv_firings_filter(const QString& firings, const QString& firings_out, const QString& original_cluster_numbers, const mv_firings_filter_opts& opts);

#endif // MV_FIRINGS_FILTER_H
