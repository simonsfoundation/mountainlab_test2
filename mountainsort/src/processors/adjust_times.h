/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#ifndef ADJUST_TIMES_H
#define ADJUST_TIMES_H

#include <QString>

struct adjust_times_opts {
    int upsampling_factor;
    int sign;
    int num_pca_denoise_components;
    int pca_denoise_jiggle;
};

bool adjust_times(const QString &timeseries_path,const QString &detect_path,const QString &detect_out_path,const adjust_times_opts &opts);

#endif // ADJUST_TIMES_H

