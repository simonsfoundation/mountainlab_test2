/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef COMMANDLINEPARAMS_H
#define COMMANDLINEPARAMS_H

#include <QMap>
#include <QString>
#include <QList>
#include <QVariant>
#include <QStringList>

struct CLParams {
	QMap<QString,QVariant> named_parameters;
	QList<QString> unnamed_parameters;
	bool success;
	QString error_message;
};

CLParams commandlineparams(int argc,char *argv[]);
CLParams commandlineparams(const QStringList &args);

#endif // GET_COMMAND_LINE_PARAMS_H
