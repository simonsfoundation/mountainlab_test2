/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef COMPUTE_TEMPLATES_H
#define COMPUTE_TEMPLATES_H

#include <QString>

bool compute_templates(const QString& timeseries_path, const QString& firings_path, const QString& templates_out_path, int clip_size);

#endif // COMPUTE_TEMPLATES_H
