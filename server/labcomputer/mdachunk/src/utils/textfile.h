/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <QString>
#include <QTextCodec>

QString read_text_file(const QString &fname, QTextCodec *codec=0);
bool write_text_file(const QString &fname,const QString &txt, QTextCodec *codec=0);

QString read_parameter(const QString &fname,const QString &pname);
void write_parameter(const QString &fname,const QString &pname,const QString &pvalue);

#endif // TEXTFILE_H
