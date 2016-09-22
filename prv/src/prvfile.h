/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/21/2016
*******************************************************/
#ifndef PRVFILE_H
#define PRVFILE_H

#define PRV_VERSION 0.1

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <functional>

struct PrvFileCreateOptions {
    bool create_temporary_files = false;
};

struct PrvFileLocateOptions {
    QStringList local_search_paths;
    bool search_remotely = false;
    QJsonArray remote_servers;
};

struct PrvFileRecoverOptions {
    bool recover_all_prv_files = false;
    PrvFileLocateOptions locate_opts;
};

class PrvFilePrivate;
class PrvFile {
public:
    friend class PrvFilePrivate;
    PrvFile(const QString& file_path = "");
    PrvFile(const QJsonObject& obj);
    virtual ~PrvFile();
    QJsonObject object() const;
    bool read(const QString& file_path);
    bool write(const QString& file_path) const;
    bool representsFile() const;
    bool representsFolder() const;
    bool createFromFile(const QString& file_path, const PrvFileCreateOptions& opts);
    bool createFromFolder(const QString& folder_path, const PrvFileCreateOptions& opts);
    bool recoverFile(const QString& dst_file_path, const PrvFileRecoverOptions& opts);
    bool recoverFolder(const QString& dst_folder_path, const PrvFileRecoverOptions& opts);
    QString locate(const PrvFileLocateOptions& opts);

private:
    PrvFilePrivate* d;
};

QString http_get_text_curl_0(const QString& url);
bool is_url(QString txt);

namespace NetUtils {
typedef std::function<void(qint64, qint64)> ProgressFunction;
QString httpPostFile(const QUrl& url, const QString& fileName, const ProgressFunction& f = ProgressFunction());
}

#endif // PRVFILE_H
