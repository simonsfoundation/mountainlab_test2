/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/11/2016
*******************************************************/

#include "mlutils.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>
#include <QThread>

QString cfp(const QString& path)
{
    /// Witold. How can I return a can a canonical file path when the file does not yet exist? This can be important for consistency!
    //for now I just return the path
    return path;
    /*
    if (QFile::exists(path)) {
        return QFileInfo(path).canonicalFilePath();
    }
    else {
        /// Witold. How can I return a can a canonical file path when the file does not yet exist?
        return path;
    }
    */
}

QString compute_checksum_of_file(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return "";
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);
    file.close();
    QString ret = QString(hash.result().toHex());
    return ret;
}

QString find_ancestor_path_with_name(QString path, QString name)
{
    if (name.isEmpty())
        return "";
    while (QFileInfo(path).fileName() != name) {
        path = QFileInfo(path).path();
        if (!path.contains(name))
            return ""; //guarantees that we eventually exit the while loop
    }
    return path; //the directory name must equal the name argument
}

QString mountainlabBasePath()
{
    return find_ancestor_path_with_name(qApp->applicationDirPath(), "mountainlab");
}

void mkdir_if_doesnt_exist(const QString& path)
{
    if (!QDir(path).exists()) {
        QDir(QFileInfo(path).path()).mkdir(QFileInfo(path).fileName());
    }
}

QString mlTmpPath()
{
    QString ret = mountainlabBasePath() + "/tmp";
    mkdir_if_doesnt_exist(ret);
    return ret;
}

QString mlLogPath()
{
    QString ret = mountainlabBasePath() + "/log";
    mkdir_if_doesnt_exist(ret);
    return ret;
}

QString mlConfigPath()
{
    QString ret = mountainlabBasePath() + "/config";
    mkdir_if_doesnt_exist(ret);
    return ret;
}

QString resolve_path(QString basepath, QString path)
{
    if (QFileInfo(path).isRelative()) {
        return basepath + "/" + path;
    }
    else
        return path;
}

bool in_gui_thread()
{
    #ifdef QT_GUI_LIB
    return (QThread::currentThread() == QCoreApplication::instance()->thread());
    #else
    //not even a gui app
    return false;
    #endif
}

bool thread_interrupt_requested()
{
    return QThread::currentThread()->isInterruptionRequested();
}
