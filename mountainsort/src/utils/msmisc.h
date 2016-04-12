/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MSMISC_H
#define MSMISC_H

#include <QList>
#include "mda.h"

double compute_min(const QList<double> &X);
double compute_max(const QList<double> &X);
double compute_max(long N,double *X);
int compute_max(const QList<int> &X);
long compute_max(const QList<long>& X);
Mda compute_mean_clip(Mda &clips);
double compute_mean(const QList<double> &X);
double compute_stdev(const QList<double> &X);
Mda grab_clips_subset(Mda &clips,const QList<int> &inds);
double compute_norm(long N,double *X);
QString http_get_text(const QString &url);
QString http_get_binary_file(const QString &url);
QString compute_hash(const QString &str);

#endif // MSMISC_H
