/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef GET_COMMAND_LINE_PARAMS_H
#define GET_COMMAND_LINE_PARAMS_H

#include <QMap>
#include <QString>
#include <QList>
#include <QVariant>
#include <QStringList>

struct CLParams {
    QMap<QString, QVariant> named_parameters;
    QList<QString> unnamed_parameters;
    bool success;
    QString error_message;
};

CLParams get_command_line_params(int argc, char* argv[]);
CLParams get_command_line_params(const QStringList& args);

#endif // GET_COMMAND_LINE_PARAMS_H
