#ifndef EXTRACT_CLIPS_H
#define EXTRACT_CLIPS_H

#include "mda.h"
#include "diskreadmda.h"

Mda extract_clips(DiskReadMda &X,const QList<double> &times,int clip_size);
Mda extract_clips(DiskReadMda &X,const QList<double> &times,const QList<int> &channels,int clip_size);

#endif // EXTRACT_CLIPS_H

