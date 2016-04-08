/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#ifndef FIT_STAGE_H
#define FIT_STAGE_H

#include <QString>

struct fit_stage_opts {
    int clip_size;
    double shell_increment;
    int min_shell_size;
};

bool fit_stage(const QString &timeseries_path,const QString &firings_path,const QString &firings_out_path,const fit_stage_opts &opts);

#endif // FIT_STAGE_H

