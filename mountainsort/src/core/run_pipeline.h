/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/15/2016
*******************************************************/

#ifndef RUN_PIPELINE_H
#define RUN_PIPELINE_H

#include "msprocessmanager.h"
#include <QJsonObject>

bool run_pipeline(MSProcessManager* PM, QJsonObject pipeline);

#endif // RUN_PIPELINE_H
