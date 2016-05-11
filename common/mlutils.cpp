/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/11/2016
*******************************************************/

#include "mlutils.h"

#include <QFileInfo>

QString cfp(const QString &path)
{
    return QFileInfo(path).canonicalFilePath();
}
