/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/6/2016
*******************************************************/

#ifndef MLCOMMON_H
#define MLCOMMON_H

#include <QTextCodec>
#include <QDebug>

class TextFile {
public:
    static QString read(const QString& fname, QTextCodec* codec = 0);
    static bool write(const QString& fname, const QString& txt, QTextCodec* codec = 0);
};

class MLUtil {
public:
    static QString makeRandomId(int numchars = 10);
    static bool threadInterruptRequested();
    static bool inGuiThread();
    static QString tempPath();
};

class CLParams {
public:
    CLParams(int argc, char* argv[]);
    QMap<QString, QVariant> named_parameters;
    QList<QString> unnamed_parameters;
    bool success;
    QString error_message;
};

#endif // TEXTFILE_H
