#ifndef MSMISC_H
#define MSMISC_H

#include <QList>
#include "mda.h"

double compute_min(const QList<double> &X);
double compute_max(const QList<double> &X);
double compute_max(long N,double *X);
int compute_max(const QList<int> &X);
Mda compute_mean_clip(Mda &clips);
double compute_mean(const QList<double> &X);
double compute_stdev(const QList<double> &X);
Mda grab_clips_subset(Mda &clips,const QList<int> &inds);

#endif // MSMISC_H

