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

#include <cachemanager.h>
#include "textfile.h"
#ifdef QT_GUI_LIB
#include <QMessageBox>
#endif

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
    //QString ret = mountainlabBasePath() + "/tmp";
    QString ret = QDir::tempPath() + "/mountainlab";
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
    } else
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



bool curl_is_installed()
{
    int exit_code = system("curl --version");
    return (exit_code == 0);
}

/// TODO handle this in a better way
static QMap<QString, QString> s_http_get_text_curl_cache;

QString http_get_text_curl(const QString& url)
{
    if (in_gui_thread()) {
        if (s_http_get_text_curl_cache.contains(url)) {
            qDebug() << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
            return s_http_get_text_curl_cache[url];
        }
    }
    if (!curl_is_installed()) {
        #ifdef QT_GUI_LIB
        QMessageBox::critical(0, "Problem in http request", "Problem in http request. It appears that curl is not installed.");
        #else
        qWarning() << "There is no reason we should be calling http_get_text_curl in a non-gui application!!!!!!!!!!!!!!!!!!!!!!!";
        #endif
        return "";
    }
    QString tmp_fname = CacheManager::globalInstance()->makeLocalFile("",CacheManager::ShortTerm);
    QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        #ifdef QT_GUI_LIB
        QMessageBox::critical(0, "Problem downloading text file", "Problem in http request. Are you connected to the internet?");
        #else
        qWarning() << "There is no reason we should be calling http_get_text_curl * in a non-gui application!!!!!!!!!!!!!!!!!!!!!!!";
        #endif
        return "";
    }
    QString ret = read_text_file(tmp_fname);
    QFile::remove(tmp_fname);
    if (in_gui_thread()) {
        s_http_get_text_curl_cache[url] = ret;
    }
    return ret;
}

