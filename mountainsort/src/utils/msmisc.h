/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MSMISC_H
#define MSMISC_H

#include <QVector>
#include "mda.h"
#include <math.h>

Mda compute_mean_clip(Mda& clips);
Mda grab_clips_subset(Mda& clips, const QVector<int>& inds);

#endif // MSMISC_H
