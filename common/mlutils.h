/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/11/2016
*******************************************************/

#ifndef MLUTILS_H
#define MLUTILS_H

#include <QString>
#include <QDebug>

QString cfp(const QString& path); //cannonical file path
QString compute_checksum_of_file(const QString& path);
QString mountainlabBasePath();
QString mlTmpPath();
QString mlLogPath();
QString mlConfigPath();
void mkdir_if_doesnt_exist(const QString& path);
QString resolve_path(QString basepath, QString path);
bool in_gui_thread();
bool thread_interrupt_requested();
QString http_get_text_curl(const QString& url);

#endif // MLUTILS_H
