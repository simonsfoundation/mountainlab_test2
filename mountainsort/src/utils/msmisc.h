/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MSMISC_H
#define MSMISC_H

#include <QVector>
#include "mda.h"
#include "mda32.h"
#include <math.h>
#include "pca.h"

Mda compute_mean_clip(Mda& clips);
Mda32 compute_mean_clip(Mda32& clips);
Mda grab_clips_subset(Mda& clips, const QVector<int>& inds);
Mda32 grab_clips_subset(Mda32& clips, const QVector<int>& inds);
bool eigenvalue_decomposition_sym(Mda& U, Mda& S, Mda& X);

#endif // MSMISC_H
