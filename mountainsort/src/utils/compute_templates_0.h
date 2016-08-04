/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/29/2016
*******************************************************/

#ifndef COMPUTE_TEMPLATES_0_H
#define COMPUTE_TEMPLATES_0_H

#include "diskreadmda.h"
#include "diskreadmda32.h"

void compute_templates_stdevs(Mda32& ret_templates, Mda32& ret_stdevs, DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int clip_size);

Mda32 compute_templates_0(DiskReadMda32& X, Mda64& firings, int clip_size);
Mda32 compute_templates_0(DiskReadMda32& X, const QVector<double>& times, const QVector<int>& labels, int clip_size);

#endif // COMPUTE_TEMPLATES_0_H
