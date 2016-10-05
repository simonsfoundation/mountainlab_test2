/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/21/2016
*******************************************************/

#include "cachemanager.h"
#include "prvfile.h"
#include "mlcommon.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QTime>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QJsonArray>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class PrvFilePrivate {
public:
    PrvFile* q;
    QJsonObject m_object;
    QString m_prv_file_path;

    bool should_store_content(QString file_path);
    bool should_store_binary_content(QString file_path);
    static void print(QString str);
    static void println(QString str);
    static QByteArray read_binary_file(const QString& fname);
    static bool write_binary_file(const QString& fname, const QByteArray& data);
    QString find_file(long size, const QString& checksum, const QString& checksum1000_optional, const PrvFileLocateOptions& opts);
    QString find_remote_file(long size, const QString& checksum, const QString& checksum1000_optional, const PrvFileLocateOptions& opts);
    QString find_local_file(long size, const QString& checksum, const QString& checksum1000_optional, const PrvFileLocateOptions& opts);
    void copy_from(const PrvFile& other);
};

PrvFile::PrvFile(const QString& file_path)
{
    d = new PrvFilePrivate;
    d->q = this;
    if (!file_path.isEmpty()) {
        this->read(file_path);
    }
    d->m_prv_file_path = file_path;
}

PrvFile::PrvFile(const QJsonObject& obj)
{
    d = new PrvFilePrivate;
    d->q = this;
    d->m_object = obj;
}

PrvFile::PrvFile(const PrvFile& other)
{
    d = new PrvFilePrivate;
    d->q = this;
    d->copy_from(other);
}

PrvFile::~PrvFile()
{
    delete d;
}

void PrvFile::operator=(const PrvFile& other)
{
    d->copy_from(other);
}

QJsonObject PrvFile::object() const
{
    return d->m_object;
}

bool PrvFile::read(const QString& file_path)
{
    QString json = TextFile::read(file_path);
    QJsonParseError err;
    d->m_object = QJsonDocument::fromJson(json.toUtf8(), &err).object();
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing .prv file: " + file_path + ": " + err.errorString();
        return false;
    }
    d->m_prv_file_path = file_path;
    return true;
}

bool PrvFile::write(const QString& file_path) const
{
    QString json = QJsonDocument(d->m_object).toJson();
    return TextFile::write(file_path, json);
}

bool PrvFile::representsFile() const
{
    return d->m_object.contains("original_checksum");
}

bool PrvFile::representsFolder() const
{
    return !representsFile();
}

bool PrvFile::createFromFile(const QString& file_path, const PrvFileCreateOptions& opts)
{
    QJsonObject obj;
    obj["prv_version"] = PRV_VERSION;
    obj["original_path"] = file_path;

    obj["original_checksum"] = MLUtil::computeSha1SumOfFile(file_path);
    obj["original_checksum_1000"] = MLUtil::computeSha1SumOfFileHead(file_path, 1000);
    obj["original_size"] = QFileInfo(file_path).size();

    if (opts.create_temporary_files) {
        QString tmp = CacheManager::globalInstance()->localTempPath();
        if (!tmp.isEmpty()) {
            QString checksum = obj["original_checksum"].toString();
            QFile::copy(file_path, tmp + "/" + checksum + ".prvdat");
        }
    }
    d->m_object = obj;
    return true;
}

bool PrvFile::createFromFolder(const QString& folder_path, const PrvFileCreateOptions& opts)
{
    QJsonObject obj;
    obj["prv_version"] = PRV_VERSION;
    obj["original_path"] = folder_path;

    QJsonArray files_array;
    QStringList file_list = QDir(folder_path).entryList(QStringList("*"), QDir::Files, QDir::Name);
    foreach (QString file, file_list) {
        QJsonObject obj0;
        obj0["file_name"] = file;
        if (d->should_store_content(folder_path + "/" + file)) {
            if (file.endsWith(".prv")) {
                QString tmp = obj0["file_name"].toString();
                obj0["file_name"] = tmp.mid(0, tmp.count() - 4); //remove the .prv extension
                d->println("storing prv::::: " + folder_path + "/" + file);
                obj0["prv"] = QJsonDocument::fromJson(TextFile::read(folder_path + "/" + file).toUtf8()).object();
                obj0["originally_a_prv_file"] = true;
            }
            else if (d->should_store_binary_content(folder_path + "/" + file)) {
                d->println("storing binary:: " + folder_path + "/" + file);
                QByteArray bytes = d->read_binary_file(folder_path + "/" + file);
                obj0["content_base64"] = QString(bytes.toBase64());
            }
            else {
                d->println("storing text:::: " + folder_path + "/" + file);
                obj0["content"] = TextFile::read(folder_path + "/" + file);
            }
        }
        else {
            d->println("making file prv: " + folder_path + "/" + file);
            PrvFile PF0;
            if (!PF0.createFromFile(folder_path + "/" + file, opts))
                return false;
            obj0["prv"] = PF0.object();
        }
        files_array.push_back(obj0);
    }
    obj["files"] = files_array;

    QJsonArray folders_array;
    QStringList folder_list = QDir(folder_path).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    foreach (QString folder, folder_list) {
        PrvFile PF0;
        if (!PF0.createFromFolder(folder_path + "/" + folder, opts))
            return false;
        QJsonObject obj0 = PF0.object();
        obj0["folder_name"] = folder;
        folders_array.push_back(obj0);
    }
    obj["folders"] = folders_array;

    d->m_object = obj;

    return true;
}

bool PrvFile::recoverFolder(const QString& dst_path, const PrvFileRecoverOptions& opts)
{
    QJsonObject obj = d->m_object;
    if (QFile::exists(dst_path)) {
        d->println("Cannot write to directory that already exists: " + dst_path);
        return false;
    }
    QString abs_dst_path = QDir::current().absoluteFilePath(dst_path);
    QString parent_path = QFileInfo(abs_dst_path).path();
    QString name = QFileInfo(abs_dst_path).fileName();
    if (!QDir(parent_path).mkdir(name)) {
        d->println("Unable to create directory. Aborting. " + abs_dst_path);
        return false;
    }

    QJsonArray files = obj["files"].toArray();
    for (int i = 0; i < files.count(); i++) {
        QJsonObject obj0 = files[i].toObject();
        QString fname0 = obj0["file_name"].toString();
        if (fname0.isEmpty()) {
            d->println("File name is empty. Aborting. " + fname0);
            return false;
        }
        d->println("Recovering " + abs_dst_path + "/" + fname0);
        if (obj0.contains("content")) {
            if (!TextFile::write(abs_dst_path + "/" + fname0, obj0["content"].toString())) {
                d->println("Unable to write file. Aborting. " + fname0);
                return false;
            }
        }
        else if (obj0.contains("content_base64")) {
            QByteArray data0 = QByteArray::fromBase64(obj0["content_base64"].toString().toUtf8());
            if (!d->write_binary_file(abs_dst_path + "/" + fname0, data0)) {
                d->println("Unable to write file. Aborting. " + fname0);
                return false;
            }
        }
        else if (obj0.contains("prv")) {
            bool to_recover = false;
            if (opts.recover_all_prv_files)
                to_recover = true;
            if (!obj0["originally_a_prv_file"].toBool())
                to_recover = true;
            if (to_recover) {
                d->println("**** RECOVERING .prv file: " + abs_dst_path + "/" + fname0);
                QJsonObject obj1 = obj0["prv"].toObject();
                PrvFile PF0(obj1);
                if (!PF0.recoverFile(abs_dst_path + "/" + fname0, opts))
                    return false;
            }
            else {
                QString json = QJsonDocument(obj0["prv"].toObject()).toJson();
                if (!TextFile::write(abs_dst_path + "/" + fname0 + ".prv", json)) {
                    d->println("Unable to write file. Aborting. " + fname0);
                    return false;
                }
            }
        }
    }

    QJsonArray folders = obj["folders"].toArray();
    for (int i = 0; i < folders.count(); i++) {
        QJsonObject obj0 = folders[i].toObject();
        QString fname0 = obj0["folder_name"].toString();
        if (fname0.isEmpty()) {
            d->println("Folder name is empty. Aborting. " + fname0);
            return false;
        }
        PrvFile PF0(obj0);
        if (!PF0.recoverFolder(dst_path + "/" + fname0, opts))
            return false;
    }
    return true;
}

QString PrvFile::locate(const PrvFileLocateOptions& opts)
{
    QJsonObject obj = d->m_object;
    if (!representsFile()) {
        qWarning() << "Problem with prv object. Does not represent a file, so cannot attempt to locate.";
        return "";
    }
    QString checksum = obj["original_checksum"].toString();
    QString checksum1000 = obj["original_checksum_1000"].toString();
    long original_size = obj["original_size"].toVariant().toLongLong();
    QString fname_or_url = d->find_file(original_size, checksum, checksum1000, opts);
    return fname_or_url;
}

QString PrvFile::prvFilePath() const
{
    return d->m_prv_file_path;
}

QString PrvFile::checksum() const
{
    return d->m_object["original_checksum"].toString();
}

QString PrvFile::checksum1000() const
{
    return d->m_object["original_checksum_1000"].toString();
}

long PrvFile::size() const
{
    return d->m_object["original_size"].toVariant().toLongLong();
}

QString PrvFile::originalPath() const
{
    return d->m_object["original_path"].toString();
}

bool PrvFile::recoverFile(const QString& dst_file_path, const PrvFileRecoverOptions& opts)
{
    QString checksum = d->m_object["original_checksum"].toString();
    QString checksum1000 = d->m_object["original_checksum_1000"].toString();
    long original_size = d->m_object["original_size"].toVariant().toLongLong();
    QString fname_or_url = d->find_file(original_size, checksum, checksum1000, opts.locate_opts);
    if (fname_or_url.isEmpty()) {
        d->println("Unable to find file: size=" + QString::number(original_size) + " checksum=" + checksum + " checksum1000=" + checksum1000);
        return false;
    }
    if (QFile::exists(dst_file_path)) {
        if (!QFile::remove(dst_file_path)) {
            qWarning() << "Unable to remove file or folder: " + dst_file_path;
            return false;
        }
    }
    if (!is_url(fname_or_url)) {
        d->println(QString("Copying %1 to %2").arg(fname_or_url).arg(dst_file_path));
        if (!QFile::copy(fname_or_url, dst_file_path)) {
            qWarning() << "Unable to copy file: " + fname_or_url + " " + dst_file_path;
            return false;
        }
        return true;
    }
    else {
        d->println(QString("Downloading %1 to %2").arg(fname_or_url).arg(dst_file_path));
        QString fname_tmp = dst_file_path + ".tmp." + MLUtil::makeRandomId(5);
        QString cmd = QString("curl %1 > %2").arg(fname_or_url).arg(fname_tmp);
        int ret = system(cmd.toUtf8().data());
        if (ret < 0)
            return false;
        if ((QFileInfo(fname_tmp).size() != original_size) || ((!checksum1000.isEmpty()) && (MLUtil::computeSha1SumOfFileHead(fname_tmp, 1000) != checksum1000))) {
            if (QFileInfo(fname_tmp).size() < 10000) {
                QString txt0 = TextFile::read(fname_tmp);
                if (txt0.startsWith("{")) {
                    //must be an error message from the server
                    qWarning() << txt0;
                    return false;
                }
            }
            qWarning() << QString("Problem with size or checksum1000 of downloaded file: %1 <> %2").arg(QFileInfo(fname_tmp).size()).arg(original_size);
            return false;
        }
        else {
            return QFile::rename(fname_tmp, dst_file_path);
        }
    }
}

bool PrvFilePrivate::should_store_content(QString file_path)
{
    if ((file_path.endsWith(".mda")) || (file_path.endsWith(".dat")))
        return false;
    return true;
}

bool PrvFilePrivate::should_store_binary_content(QString file_path)
{
    QStringList text_file_extensions;
    text_file_extensions << "txt"
                         << "csv"
                         << "ini"
                         << "cfg"
                         << "json"
                         << "h"
                         << "cpp"
                         << "pro"
                         << "sh"
                         << "js"
                         << "m"
                         << "py";
    foreach (QString ext, text_file_extensions)
        if (file_path.endsWith("." + ext))
            return false;
    return true;
}

void PrvFilePrivate::print(QString str)
{
    printf("%s", str.toUtf8().data());
}

void PrvFilePrivate::println(QString str)
{
    printf("%s\n", str.toUtf8().data());
}

QByteArray PrvFilePrivate::read_binary_file(const QString& fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file for reading: " + fname;
        return QByteArray();
    }
    QByteArray ret = file.readAll();
    file.close();
    return ret;
}

bool PrvFilePrivate::write_binary_file(const QString& fname, const QByteArray& data)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Unable to open file for writing: " + fname;
        return false;
    }
    bool ret = true;
    if (file.write(data) != data.count())
        ret = false;
    file.close();
    return ret;
}

QString PrvFilePrivate::find_remote_file(long size, const QString& checksum, const QString& checksum1000_optional, const PrvFileLocateOptions& opts)
{
    QJsonArray remote_servers = opts.remote_servers;
    for (int i = 0; i < remote_servers.count(); i++) {
        QJsonObject server0 = remote_servers[i].toObject();
        QString host = server0["host"].toString();
        int port = server0["port"].toInt();
        QString url_path = server0["path"].toString();
        QString passcode = server0["passcode"].toString();
        QString url0 = host + ":" + QString::number(port) + url_path + QString("/?a=locate&checksum=%1&checksum1000=%2&size=%3&passcode=%4").arg(checksum).arg(checksum1000_optional).arg(size).arg(passcode);
        url0 += "&passcode=" + server0["passcode"].toString();
        if (opts.verbose) {
            qDebug() << url0;
        }
        QString txt = http_get_text_curl_0(url0);
        if (!txt.isEmpty()) {
            if (!txt.contains(" ")) { //filter out error messages (good idea, or not?)
                if (!is_url(txt)) {
                    txt = host + ":" + QString::number(port) + url_path + "/" + txt;
                }
                return txt;
            }
        }
    }
    return "";
}

QString PrvFilePrivate::find_file(long size, const QString& checksum, const QString& checksum1000_optional, const PrvFileLocateOptions& opts)
{
    if (opts.search_locally) {
        if (opts.verbose)
            printf("Searching locally...\n");
        QString local_fname = find_local_file(size, checksum, checksum1000_optional, opts);
        if (!local_fname.isEmpty()) {
            if (opts.verbose) {
                printf("Found file: %s\n", local_fname.toUtf8().data());
            }
            return local_fname;
        }
    }

    if (opts.search_remotely) {
        if (opts.verbose)
            printf("Searching remotely...\n");
        QString remote_url = find_remote_file(size, checksum, checksum1000_optional, opts);
        if (!remote_url.isEmpty()) {
            if (opts.verbose) {
                printf("Found remote file: %s\n", remote_url.toUtf8().data());
            }
            return remote_url;
        }
    }

    return "";
}

QString find_file_2(QString directory, QString checksum, QString checksum1000_optional, long size, bool recursive)
{
    QStringList files = QDir(directory).entryList(QStringList("*"), QDir::Files);
    foreach (QString file, files) {
        QString path = directory + "/" + file;
        if (QFileInfo(path).size() == size) {
            if (!checksum1000_optional.isEmpty()) {
                QString checksum0 = MLUtil::computeSha1SumOfFileHead(path, 1000);
                if (checksum0 == checksum1000_optional) {
                    QString checksum1 = MLUtil::computeSha1SumOfFile(path);
                    if (checksum1 == checksum) {
                        return path;
                    }
                }
            }
            else {
                QString checksum1 = MLUtil::computeSha1SumOfFile(path);
                if (checksum1 == checksum) {
                    return path;
                }
            }
        }
    }
    if (recursive) {
        QStringList dirs = QDir(directory).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString dir, dirs) {
            QString path = find_file_2(directory + "/" + dir, checksum, checksum1000_optional, size, recursive);
            if (!path.isEmpty())
                return path;
        }
    }
    return "";
}

QString PrvFilePrivate::find_local_file(long size, const QString& checksum, const QString& checksum1000_optional, const PrvFileLocateOptions& opts)
{
    QStringList local_search_paths = opts.local_search_paths;
    for (int i = 0; i < local_search_paths.count(); i++) {
        QString search_path = local_search_paths[i];
        QString fname = find_file_2(search_path, checksum, checksum1000_optional, size, true);
        if (!fname.isEmpty())
            return fname;
    }
    return "";
}

void PrvFilePrivate::copy_from(const PrvFile& other)
{
    m_object = other.d->m_object;
    m_prv_file_path = other.d->m_prv_file_path;
}

bool is_url(QString txt)
{
    return ((txt.startsWith("http://")) || (txt.startsWith("https://")));
}

bool curl_is_installed()
{
    QProcess P;
    P.start("curl --version");
    P.waitForStarted();
    P.waitForFinished(-1);
    int exit_code = P.exitCode();
    return (exit_code == 0);
}

QString http_get_text_curl_0(const QString& url)
{
    if (!curl_is_installed()) {
        qWarning() << "Problem in http request. It appears that curl is not installed.";
        return "";
    }
    //QString tmp_fname = make_temporary_file()+".curl";
    QString cmd = QString("curl --connect-timeout 2 \"%1\"").arg(url);
    //int exit_code = system(cmd.toLatin1().data());
    QProcess P;
    P.start(cmd);
    P.waitForStarted();
    P.waitForFinished(-1);
    int exit_code = P.exitCode();
    if (exit_code != 0) {
        //QFile::remove(tmp_fname);
        return "";
    }
    P.readAllStandardError();
    return P.readAllStandardOutput();
}

namespace NetUtils {

QString httpPostFile(const QUrl& url, const QString& fileName, const ProgressFunction& progressFunction)
{
    QNetworkAccessManager manager;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return QString::null;
    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    QNetworkReply* reply = manager.post(request, &file);
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    // TODO: handle timeout
    if (progressFunction) {
        QObject::connect(reply, &QNetworkReply::uploadProgress, [reply, progressFunction](qint64 bytesSent, qint64 bytesTotal) {
            progressFunction(bytesSent, bytesTotal);
        });
    }
    loop.exec();
    printf("\n");
    QTextStream stream(reply);
    return stream.readAll();
}

QString httpPostFile(const QString& url, const QString& fileName)
{
    return httpPostFile(QUrl(url), fileName);
}
}
