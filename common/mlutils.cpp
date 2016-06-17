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
#include <QTime>
#include "textfile.h"
#include "taskprogress.h"

#include <cachemanager.h>
#include "textfile.h"
#ifdef QT_GUI_LIB
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <mountainprocessrunner.h>
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
    QString tmp_fname = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
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

QString get_temp_fname()
{
    return CacheManager::globalInstance()->makeLocalFile();
    //long rand_num = qrand() + QDateTime::currentDateTime().toMSecsSinceEpoch();
    //return QString("%1/MdaClient_%2.tmp").arg(QDir::tempPath()).arg(rand_num);
}

QString http_get_binary_file_curl(const QString& url)
{
    QString tmp_fname = get_temp_fname();
    QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        return "";
    }
    return tmp_fname;
}

/*
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
    QString tmp_fname = get_temp_fname();
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
*/

QString http_get_binary_file(const QString& url)
{
#ifdef QT_GUI_LIB
    if (in_gui_thread()) {
        qCritical() << "Cannot call http_get_binary_file from within the GUI thread: " + url;
        exit(-1);
    }

    //TaskProgress task("Downloading binary file");
    //task.log(url);
    QTime timer;
    timer.start();
    QString fname = get_temp_fname();
    QNetworkAccessManager manager; // better make it a singleton
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QFile temp(fname);
    long num_bytes = 0;
    temp.open(QIODevice::WriteOnly);
    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
        if (thread_interrupt_requested()) {
            TaskProgress errtask("Download halted");
            errtask.error("Thread interrupt requested");
            errtask.log(url);
            reply->abort();
        }
        QByteArray X=reply->readAll();
        temp.write(X);
        num_bytes+=X.count();
    });
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    //task.setLabel(QString("Downloaded %1 MB in %2 sec").arg(num_bytes * 1.0 / 1e6).arg(timer.elapsed() * 1.0 / 1000));
    printf("RECEIVED BINARY (%d ms, %ld bytes) from %s\n", timer.elapsed(), num_bytes, url.toLatin1().data());
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_downloaded", num_bytes);
    if (thread_interrupt_requested()) {
        return "";
    }
    return fname;
#else
    Q_UNUSED(url)
    qWarning() << "Cannot download binary file in non-gui application.";
    return "";
#endif
}

QString abbreviate(const QString& str, int len1, int len2)
{
    if (str.count() <= len1 + len2 + 20)
        return str;
    return str.mid(0, len1) + "...\n...\n..." + str.mid(str.count() - len2);
}

QString http_get_text(const QString& url)
{
#ifdef QT_GUI_LIB
    if (in_gui_thread()) {
        return http_get_text_curl(url);
    }
    QTime timer;
    timer.start();
    QString fname = get_temp_fname();
    QNetworkAccessManager manager; // better make it a singleton
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QString ret;
    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
        ret+=reply->readAll();
    });
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    printf("RECEIVED TEXT (%d ms, %d bytes) from GET %s\n", timer.elapsed(), ret.count(), url.toLatin1().data());
    QString str = abbreviate(ret, 200, 200);
    printf("%s\n", (str.toLatin1().data()));

    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_downloaded", ret.count());
    return ret;
#else
    Q_UNUSED(url)
    qWarning() << "Cannot download text file in non-gui application.";
    return "";
#endif
}

/*
QString get_path_query(const QString& path)
{
    int ind1 = path.lastIndexOf("?");
    int ind2 = path.lastIndexOf("/");
    if (ind1 < 0)
        return "";
    if (ind2 > ind1)
        return "";
    return path.mid(ind1 + 1);
}

QString get_path_without_query(const QString& path)
{
    int count = get_path_query(path).count();
    if (count == 0)
        return path;
    else
        return path.mid(0, path.count() - count - 1);
}

QMap<QString, QString> parse_query(const QString& query)
{
    QMap<QString, QString> ret;
    QStringList list = query.split("&");
    foreach(QString str, list)
    {
        QStringList tmp = str.split("=");
        if (tmp.count() == 2) {
            ret[tmp[0]] = tmp[1];
        }
    }
    return ret;
}

QMap<QString, QString> parse_path_query(const QString &path)
{
    return parse_query(get_path_query(path));
}
*/

DiskReadMda compute_filtered_firings(QString mlproxy_url, const DiskReadMda& firings, MVEventFilter filter)
{
    if (!filter.use_event_filter)
        return firings;

    TaskProgress task(TaskProgress::Calculate, "compute filtered firings");
    MountainProcessRunner X;
    QString processor_name = "mv_firings_filter";
    X.setProcessorName(processor_name);

    QMap<QString, QVariant> params;
    params["firings"] = firings.makePath();
    params["use_shell_split"] = false;
    params["use_event_filter"] = filter.use_event_filter;
    params["min_amplitude"] = 0;
    params["min_detectability_score"] = filter.min_detectability_score;
    params["max_outlier_score"] = filter.max_outlier_score;
    X.setInputParameters(params);
    X.setMLProxyUrl(mlproxy_url);

    QString firings_out_path = X.makeOutputFilePath("firings_out");

    X.runProcess();
    DiskReadMda ret(firings_out_path);
    return ret;
}
