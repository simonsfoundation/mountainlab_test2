/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#ifndef FIT_STAGE_H
#define FIT_STAGE_H

#include <QString>
#include "mda.h"
#include "compute_detectability_scores.h"

struct fit_stage_opts {
    int clip_size;
    double shell_increment;
    int min_shell_size;
};

bool fit_stage(const QString &timeseries_path,const QString &firings_path,const QString &firings_out_path,const fit_stage_opts &opts);
bool fit_stage_new(const QString &timeseries_path,const QString &firings_path,const QString &firings_out_path,const fit_stage_opts &opts);

double compute_score(long N, double* X, double* template0);
QList<int> find_events_to_use(const QList<long>& times, const QList<double>& scores, const fit_stage_opts& opts);
void subtract_scaled_template(long N, double* X, double* template0);
Mda split_into_shells(const Mda& firings, Define_Shells_Opts opts);
Mda sort_firings_by_time(const Mda& firings);

#endif // FIT_STAGE_H

