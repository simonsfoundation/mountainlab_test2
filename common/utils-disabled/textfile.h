/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <QString>
#include <QTextCodec>

QString TextFile::read(const QString& fname, QTextCodec* codec = 0);
bool TextFile::write(const QString& fname, const QString& txt, QTextCodec* codec = 0);

QString read_parameter(const QString& fname, const QString& pname);
void write_parameter(const QString& fname, const QString& pname, const QString& pvalue);

#endif // TEXTFILE_H
