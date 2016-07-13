#ifndef ONEFILE_H
#define ONEFILE_H

#include <QStringList>

QString create_one_file_text(const QStringList& file_paths, QString type0);
QString create_temporary_file(QString type0);
void open_editor(QString editor_exe, QString fname);
void onefile_save_changes(QString fname);

#endif // ONEFILE_H
