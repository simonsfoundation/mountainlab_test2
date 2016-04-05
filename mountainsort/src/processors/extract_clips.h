/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef EXTRACT_CLIPS_H
#define EXTRACT_CLIPS_H

#include "mda.h"
#include "diskreadmda.h"

bool extract_clips(const QString &timeseries_path,const QString &firings_path,const QString &clips_path,int clip_size);
bool extract_clips_features(const QString &timeseries_path,const QString &firings_path,const QString &features_path,int clip_size,int num_features);

Mda extract_clips(DiskReadMda &X,const QList<double> &times,int clip_size);
Mda extract_clips(DiskReadMda &X,const QList<double> &times,const QList<int> &channels,int clip_size);

#endif // EXTRACT_CLIPS_H
