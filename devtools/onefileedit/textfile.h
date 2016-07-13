/******************************************************
**
** Copyright (C) 2016 by Jeremy Magland
**
** This file is part of the MountainSort C++ project
**
** Some rights reserved.
** See accompanying LICENSE and README files.
**
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
