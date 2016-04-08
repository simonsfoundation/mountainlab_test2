/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef REMOVE_NOISE_SUBCLUSTERS_H
#define REMOVE_NOISE_SUBCLUSTERS_H

#include <QList>
#include <QString>
#include "compute_detectability_scores.h"

struct Remove_noise_subclusters_opts {
	int clip_size;
	double detectability_threshold;
	double shell_increment;
    int min_shell_size;
};

bool remove_noise_subclusters(const QString &timeseries_path,const QString &firings_path,const QString &firings_out_path,const Remove_noise_subclusters_opts &opts);

#endif // REMOVE_NOISE_SUBCLUSTERS_H
