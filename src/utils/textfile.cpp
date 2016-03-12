#include "textfile.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>

QString read_text_file(const QString & fname, QTextCodec *codec) {
    QFile file(fname);
	if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        return QString();
	}
    QTextStream ts(&file);
    if (codec != 0)
        ts.setCodec(codec);
    QString ret = ts.readAll();
    file.close();
    return ret;
}

bool write_text_file(const QString & fname,const QString &txt, QTextCodec *codec) {
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream ts(&file);
    if (codec != 0){
        ts.setAutoDetectUnicode(false);
        ts.setCodec(codec);
    }
    ts << txt;
    ts.flush();
    file.close();
    //if (!QFile(fname).exists()) return false;
    return true;
}
QString read_parameter(const QString &fname,const QString &pname) {
    QSettings settings(fname,QSettings::IniFormat);
    return settings.value(pname).toString();
}
void write_parameter(const QString &fname,const QString &pname,const QString &pvalue) {
    QSettings settings(fname,QSettings::IniFormat);
    settings.setValue(pname,pvalue);
}



