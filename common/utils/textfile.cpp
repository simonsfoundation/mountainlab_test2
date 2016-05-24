#include "textfile.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>
#include <QTime>

QChar make_random_alphanumeric_tf();
QString make_random_id_tf(int numchars);

QString read_text_file(const QString& fname, QTextCodec* codec)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream ts(&file);
    if (codec != 0)
        ts.setCodec(codec);
    QString ret = ts.readAll();
    file.close();
    return ret;
}

bool write_text_file(const QString& fname, const QString& txt, QTextCodec* codec)
{

    /*
     * Modification on 5/23/16 by jfm
     * We don't want an program to try to read this while we have only partially completed writing the file.
     * Therefore we now create a temporary file and then copy it over
     */

    QString tmp_fname = fname + ".tf." + make_random_id_tf(6) + ".tmp";

    //if a file with this name already exists, we need to remove it
    //(should we really do this before testing whether writing is successful? I think yes)
    if (QFile::exists(fname)) {
        if (!QFile::remove(fname)) {
            qWarning() << "Problem in write_text_file" << __FUNCTION__ << __FILE__ << __LINE__;
            return false;
        }
    }

    //write text to temporary file
    QFile file(tmp_fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Problem in write_text_file could not open for writing... " << __FUNCTION__ << __FILE__ << __LINE__ << tmp_fname;
        return false;
    }
    QTextStream ts(&file);
    if (codec != 0) {
        ts.setAutoDetectUnicode(false);
        ts.setCodec(codec);
    }
    ts << txt;
    ts.flush();
    file.close();

    //check the contents of the file (is this overkill?)
    QString txt_test = read_text_file(tmp_fname, codec);
    if (txt_test != txt) {
        QFile::remove(tmp_fname);
        qWarning() << "Problem in write_text_file" << __FUNCTION__ << __FILE__ << __LINE__;
        return false;
    }

    //finally, rename the file
    if (!QFile::rename(tmp_fname, fname)) {
        qWarning() << "Problem in write_text_file" << __FUNCTION__ << __FILE__ << __LINE__;
        return false;
    }

    return true;
}
QString read_parameter(const QString& fname, const QString& pname)
{
    QSettings settings(fname, QSettings::IniFormat);
    return settings.value(pname).toString();
}
void write_parameter(const QString& fname, const QString& pname, const QString& pvalue)
{
    QSettings settings(fname, QSettings::IniFormat);
    settings.setValue(pname, pvalue);
}

QChar make_random_alphanumeric_tf()
{
    static int val = 0;
    val++;
    QTime time = QTime::currentTime();
    int num = qHash(time.toString("hh:mm:ss:zzz") + QString::number(qrand() + val));
    if (num<0) num=-num;
    num = num % 36;
    if (num < 26)
        return QChar('A' + num);
    else
        return QChar('0' + num - 26);
}
QString make_random_id_tf(int numchars)
{
    QString ret;
    for (int i = 0; i < numchars; i++) {
        ret.append(make_random_alphanumeric_tf());
    }
    return ret;
}
