/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef qjson_H
#define qjson_H

#include <QString>
#include <QVariant>

QVariant parseJSON(const QString &txt);
QString toJSON(const QVariant &map_or_list);
QByteArray toJSON_bytearray(const QVariant &map_or_list);

#endif
