#ifndef GET_COMMAND_LINE_PARAMS
#define GET_COMMAND_LINE_PARAMS

#include <QMap>
#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>

struct CLParams {
	QMap<QString,QString> named_parameters;
	QList<QString> unnamed_parameters;
	bool success;
	QString error_message;
};

CLParams get_command_line_params(int argc,char *argv[],const QStringList &required_params,QStringList &optional_params);

#endif // GET_COMMAND_LINE_PARAMS

