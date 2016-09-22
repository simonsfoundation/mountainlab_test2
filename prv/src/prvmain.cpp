#include "clparams.h"
#include <QFile>
#include "sumit.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QJsonArray>
#include <QSettings>
#include <QProcess>
#include <QUrl>
#include <QStandardPaths>
#include <QHostInfo>
#include <QCommandLineParser>
#include <QUrlQuery>
#include "cachemanager.h"
#include "prvfile.h"

void usage() {
    printf("Usage:\n");
    printf("prv sha1sum [file_name]\n");
    printf("prv stat [file_name]\n");
    printf("prv create [src_file_name|folder_name] [dst_name.prv (optional)] [--create-temporary-files]\n");
    printf("prv locate [file_name.prv]\n");
    printf("prv locate --checksum=[] --checksum1000=[optional] --size=[]\n");
    printf("prv download --checksum=[] --checksum1000=[optional] --size=[]\n");
    printf("prv recover [src_file_name.prv] [dst_file_name|folder_name (optional)] \n");
    printf("prv list-subservers\n");
    printf("prv upload [file or folder name] [server name or url] --[custom_key]=[custom value] ...\n");
}

QJsonObject get_config();
QString get_tmp_path();
QString get_server_url(QString url_or_server_name);
QStringList get_local_search_paths();
QJsonArray get_remote_servers();

void print(QString str);
void println(QString str);
bool is_file(QString path);
bool is_folder(QString path);


QString get_user_name() {
#ifdef Q_OS_LINUX
    return qgetenv("USER");
#else
    // WW: not a standard way to fetch username
    QStringList home_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    return home_path.first().split(QDir::separator()).last();
#endif
}

namespace MLUtils {
class ApplicationCommand {
public:
    enum { ShowHelp = -1000 };
    virtual QString commandName() const = 0;
    virtual QString description() const { return QString(); }
    virtual void prepareParser(QCommandLineParser& ) {}
    virtual ~ApplicationCommand() {}
    virtual int execute(const QCommandLineParser &) { return 0; }
};

class ApplicationCommandParser {
public:
    ApplicationCommandParser() {}
    bool process(const QCoreApplication &app) {
        m_result = 0;
        QCommandLineParser parser;
        parser.addPositionalArgument("command", "Command to execute");
        parser.addHelpOption();
        // TODO: build description from list of commands
        QString desc;
        const QLatin1Char nl('\n');
        desc += nl;
        desc += "Commands:" + nl;
        int longestCommandName = 0;
        foreach(ApplicationCommand* cmd, m_commands) {
            longestCommandName = qMax(longestCommandName, cmd->commandName().size());
        }

        foreach(ApplicationCommand* cmd, m_commands) {
            desc += cmd->commandName();
            if (cmd->description().isEmpty()) {
                desc += nl;
            } else {
                int spaces = longestCommandName - cmd->commandName().size()+1;
                desc += QString(spaces, ' ');
                desc += '\t' + cmd->description() + nl;
            }
        }
        parser.setApplicationDescription(desc);
        parser.parse(app.arguments());
        if (parser.positionalArguments().isEmpty()) {
            parser.showHelp(1);
            return false;
        }
        QString command = parser.positionalArguments().first();
        foreach(ApplicationCommand* cmd, m_commands) {
            if (cmd->commandName() == command) {
                parser.clearPositionalArguments();
                parser.setApplicationDescription(QString());
                cmd->prepareParser(parser);
                parser.process(app);
                m_result = cmd->execute(parser);
                if (m_result == ApplicationCommand::ShowHelp) parser.showHelp(m_result);
                return true;
            }
        }
        // command not found
        parser.showHelp(0);
        return false;
    }

    ~ApplicationCommandParser() {
        qDeleteAll(m_commands);
    }

    void addCommand(ApplicationCommand* cmd) {
        m_commands << cmd;
    }
    int result() const { return m_result; }
private:
    QList<ApplicationCommand*> m_commands;
    int m_result = 0;
};
}

namespace PrvCommands {

class Sha1SumCommand : public MLUtils::ApplicationCommand {
public:
    QString commandName() const { return QStringLiteral("sha1sum"); }
    QString description() const { return QStringLiteral("Calculate sha1 hash for file"); }
    void prepareParser(QCommandLineParser& parser) {
        parser.addPositionalArgument("file_name", "File name");

    }
    int execute(const QCommandLineParser& parser) {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        if (args.isEmpty()) {
            return ShowHelp; // show help
        }
        QString path = args.first();
        if (!QFile::exists(path)) {
            qWarning() << "No such file: "+path;
            return -1;
        }
        return sha1sum(path,QVariantMap());
    }
private:
    int sha1sum(QString path,const QVariantMap &params) const {
        Q_UNUSED(params)
        QString checksum=sumit(path);
        if (checksum.isEmpty()) return -1;
        println(checksum);
        return 0;
    }
};

class StatCommand : public MLUtils::ApplicationCommand {
public:
    QString commandName() const { return "stat"; }
    QString description() const { return "Stat a file"; }

    void prepareParser(QCommandLineParser &parser)
    {
        parser.addPositionalArgument("file_name", "File name");
    }
    int execute(const QCommandLineParser &parser)
    {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        if (args.isEmpty()) {
            return ShowHelp; // show help
        }
        QString path = args.first();
        if (!QFile::exists(path)) {
            qWarning() << "No such file: "+path;
            return -1;
        }
        return stat(path, QVariantMap());
    }
private:
    int stat(QString path,const QVariantMap &params) const {
        Q_UNUSED(params)
        QString checksum=sumit(path);
        if (checksum.isEmpty()) return -1;
        QJsonObject obj;
        obj["checksum"]=checksum;
        obj["checksum1000"]=sumit(path,1000);
        obj["size"]=QFileInfo(path).size();
        println(QJsonDocument(obj).toJson());
        return 0;
    }
};

class ListSubserversCommand : public MLUtils::ApplicationCommand {
public:
    QString commandName() const { return QStringLiteral("list-subservers"); }
    QString description() const { return QStringLiteral("List sub servers"); }

    int execute(const QCommandLineParser &)
    {
        return list_subservers();
    }
private:
    int list_subservers() const {
        QJsonArray remote_servers=get_remote_servers();
        for (int i=0; i<remote_servers.count(); i++) {
            QJsonObject server0=remote_servers[i].toObject();
            QString host=server0["host"].toString();
            int port=server0["port"].toInt();
            QString url_path=server0["path"].toString();
            QString url0=host+":"+QString::number(port)+url_path+QString("/?a=list-subservers");
            url0+="&passcode="+server0["passcode"].toString();
            println("Connecting to "+url0);
            QString txt=http_get_text_curl_0(url0);
            print(txt+"\n\n");
        }
        return 0;
    }
};

class UploadCommand : public MLUtils::ApplicationCommand {
public:
    UploadCommand() {}
    QString commandName() const { return QStringLiteral("upload"); }
    QString description() const { return QStringLiteral("Upload files to server"); }
    void prepareParser(QCommandLineParser &parser)
    {
        parser.addPositionalArgument(QStringLiteral("path"), QStringLiteral("File or directory name"));
        parser.addPositionalArgument(QStringLiteral("server"), QStringLiteral("Server name or URL"));
        parser.addOption(QCommandLineOption(
                             QStringList() << QStringLiteral("param") << QStringLiteral("p"),
                             QStringLiteral("Custom parameter which will be set while uploading"),
                             QStringLiteral("param")));

        parser.addOption(QCommandLineOption(
                             QStringList() << QStringLiteral("include") << QStringLiteral("i"),
                             QStringLiteral("file patterns to include in upload (can be given multiple times)"), "include"));
        parser.addOption(QCommandLineOption(
                             QStringList() << QStringLiteral("exclude") << QStringLiteral("e"),
                             QStringLiteral("file patterns to exclude from upload (can be given multiple times)"), "exclude"));

    }
    int execute(const QCommandLineParser &parser)
    {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        if (args.size() < 2) return -1;

        QString src_path=args.at(0);
        QString server_url=get_server_url(args.at(1));
        if (src_path.isEmpty()) {
            return ShowHelp;
        }
        if (!QFile::exists(src_path)) {
            qWarning() << "No such file: "+src_path;
            return -1;
        }
        if (src_path.endsWith(".prv")) {
            qWarning() << "Cannot upload a .prv file.";
            return -1;
        }
        // parser custom options
        QVariantMap customParams;
        if (parser.isSet("param")) {
            foreach (const QString param, parser.values("param")) {
                int idx = param.indexOf('=');
                if (idx <= 0) continue; // TODO: error?
                QString key = param.left(idx);
                QVariant value = param.mid(idx+1);
                customParams[key] = value;
            }
        }
        QStringList includes = parser.values("include");
        QStringList excludes = parser.values("exclude");
        m_totalSize = 0;
        upload(src_path, server_url, customParams, includes, excludes);
        return 0;
    }

private:
    int64_t upload(const QString &path, const QString &server_url, const QVariantMap &params,
                const QStringList &includes = QStringList(), const QStringList &excludes = QStringList()) {
        int64_t totalSize = 0;
        QFileInfo pathInfo = path;
        if (!pathInfo.isDir()) {
            return uploadFile(path, server_url, params);
        }
        QDir dir = path;
        QFileInfoList entries = dir.entryInfoList(includes, QDir::Files,QDir::Name);
        foreach(const QFileInfo& fileInfo, entries) {
            bool exclude = false;
            foreach(const QString &exPattern, excludes) {
                QRegExp rx(exPattern);
                rx.setPatternSyntax(QRegExp::WildcardUnix);
                if (rx.exactMatch(fileInfo.fileName())) {
                    exclude = true;
                }
                if (exclude) break;
            }
            if (exclude) continue;
            int64_t res = uploadFile(fileInfo.absoluteFilePath(), server_url, params);
            if (res > 0) {
                totalSize += res;
            }
        }
        // recursive into subdirectories unless explicitly excluded
        entries = dir.entryInfoList(QStringList(), QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
        foreach(const QFileInfo& fileInfo, entries) {
            bool exclude = false;
            foreach(const QString &exPattern, excludes) {
                QRegExp rx(exPattern);
                rx.setPatternSyntax(QRegExp::WildcardUnix);
                if (rx.exactMatch(fileInfo.fileName())) {
                    exclude = true;
                }
                if (exclude) break;
            }
            if (exclude) continue;
                int64_t res = upload(fileInfo.absoluteFilePath(), server_url, params, includes, excludes);
                if (res > 0) {
                    totalSize += res;
                }
        }
        return totalSize;
    }

    int64_t uploadFile(const QString &path, const QString &server_url, const QVariantMap &params) {
        QFileInfo finfo = path;
        long size0=finfo.size();
        if (size0==0) {
            println("File is empty... skipping: "+path);
            return 0;
        }
        QString checksum00=sumit(path);
        if (checksum00.isEmpty()) {
            println("checksum is empty for file: "+path);
            return -1;
        }
        QUrl url(server_url);
        QUrlQuery query;
        query.addQueryItem("a", "upload");
        query.addQueryItem("checksum", checksum00);
        query.addQueryItem("size", QString::number(size0));
        QJsonObject info;
        info["src_path"]=QDir::current().absoluteFilePath(path);
        info["server_url"]=server_url;
        info["user"]=get_user_name();
        info["local_host_name"]=QHostInfo::localHostName();
        info["date_uploaded"]=QDateTime::currentDateTime().toString("yyyy-MM-dd:hh-mm-ss");
        info["params"]=QJsonObject::fromVariantMap(params);
        QString info_json=QJsonDocument(info).toJson();
        query.addQueryItem("info", QUrl::toPercentEncoding(info_json.toUtf8()));
        url.setQuery(query);
        QString ret=NetUtils::httpPostFile(url,path, [](qint64 bytesSent, qint64 bytesTotal) {
                if (bytesTotal == 0) {
                printf("\33[2K\r");
                return;
                }
                printf("\33[2K\r");
                unsigned int prc = qRound(50*((double)bytesSent/(double)bytesTotal));
                for(unsigned int i = 0; i < prc; ++i) printf("#");
                for(unsigned int i = prc; i < 50; ++i) printf(".");
                fflush(stdout);
        });
        if (ret.isEmpty()) {
            qWarning() << "Problem posting file to: "+url.toString();
            return -1;
        }
        QJsonParseError err0;
        QJsonObject obj=QJsonDocument::fromJson(ret.toUtf8(),&err0).object();
        if (err0.error!=QJsonParseError::NoError) {
            println(QString("Error uploading file."));
            return -1;
        }
        if (!obj["success"].toBool()) {
            QString error=obj["error"].toString();
            println(QString("Error uploading file: %1").arg(error));
            return -1;
        }
        m_totalSize += size0;
        println(QString("Uploaded file %1 (%2 MB, total %3 MB).").arg(finfo.fileName()).arg(size0*1.0/1e6).arg(m_totalSize*1.0/1e6));
        return size0;
    }
    int64_t m_totalSize = 0;
};

class CreateCommand : public MLUtils::ApplicationCommand {
public:
    QString commandName() const { return "create"; }
    QString description() const { return "Creates a new prv file"; }
    void prepareParser(QCommandLineParser &parser)
    {
        parser.addPositionalArgument("source", "Source file or directory name");
        parser.addPositionalArgument("dest", "Destination file or directory name", "[dest]");
        parser.addOption(QCommandLineOption("create-temporary-files"));
    }
    int execute(const QCommandLineParser &parser)
    {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        if (args.isEmpty()) {
            return ShowHelp; // show help
        }

        QString src_path=args.at(0);
        QString dst_path=args.at(1);
        if (src_path.isEmpty()) {
            return -1;
        }
        if (!QFile::exists(src_path)) {
            qWarning() << "No such file: "+src_path;
            return -1;
        }
        if (dst_path.isEmpty()) {
            dst_path=QFileInfo(src_path).fileName()+".prv";
        }
        if (!dst_path.endsWith(".prv")) {
            printf("Destination file must end with .prv");
            return -1;
        }
        QVariantMap params;
        if (parser.isSet("create-temporary-files")) {
            params.insert("create-temporary-files", true);
        }
        if (is_file(src_path)) {
            return create_file_prv(src_path,dst_path,params);
        }
        else if (is_folder(src_path)) {
            return create_folder_prv(src_path,dst_path,params);
        }
        else {
            qWarning() << "not sure why file is not a file nor a folder.";
            return -1;
        }
    }
private:
    int create_file_prv(QString src_path,QString dst_path,const QVariantMap &params) const {
        println("making prv file: "+dst_path);
        PrvFile PF;
        PrvFileCreateOptions opts;
        opts.create_temporary_files=params.contains("create-temporary-files");
        PF.createFromFile(src_path,opts);
        if (!PF.write(dst_path)) return -1;
        return 0;
    }

    int create_folder_prv(QString src_path,QString dst_path,const QVariantMap &params) const {
        println("making folder prv: "+src_path);
        PrvFile PF;
        PrvFileCreateOptions opts;
        opts.create_temporary_files=params.contains("create-temporary-files");
        PF.createFromFolder(src_path,opts);
        if (!PF.write(dst_path)) return -1;
        return 0;
    }
};

class RecoverCommand : public MLUtils::ApplicationCommand {
public:
    QString commandName() const { return "recover"; }

    void prepareParser(QCommandLineParser &parser)
    {
        parser.addPositionalArgument("source", "Source PRV file name");
        parser.addPositionalArgument("dest", "Destination file or directory name", "[dest]");
        parser.addOption(QCommandLineOption("recover-all-prv-files", ""));
    }
    int execute(const QCommandLineParser &parser)
    {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name
        if (args.isEmpty()) {
            return ShowHelp; // show help
        }
        QString src_path=args.at(0);
        QString dst_path=args.at(1);
        if (src_path.isEmpty()) {
            return ShowHelp;
        }
        if (!QFile::exists(src_path)) {
            qWarning() << "No such file: "+src_path;
            return -1;
        }
        if (!src_path.endsWith(".prv")) {
            println("prv file must have .prv extension");
            return -1;
        }
        if (dst_path.isEmpty()) {
            QString f0=QFileInfo(src_path).fileName();
            dst_path=f0.left(f0.count()-4); //remove .prv extension
        }
        PrvFile prv_file(src_path);
        PrvFileRecoverOptions opts;
        opts.recover_all_prv_files=parser.isSet("recover-all-prv-files");
        opts.locate_opts.local_search_paths=get_local_search_paths();
        opts.locate_opts.search_remotely=true;
        opts.locate_opts.remote_servers=get_remote_servers();
        if (prv_file.representsFile()) {
            if (!prv_file.recoverFile(dst_path,opts))
                return -1;
        }
        else {
            if (!prv_file.recoverFolder(dst_path,opts))
                return -1;
        }
        return 0;
    }
};

class LocateDownloadCommand : public MLUtils::ApplicationCommand {
public:
    LocateDownloadCommand(const QString &cmd) : m_cmd(cmd) {}
public:
    QString commandName() const { return m_cmd; }
    void prepareParser(QCommandLineParser &parser)
    {
        if (m_cmd == "locate") {
            parser.addPositionalArgument("file_name", "PRV file name", "[file_name]");
        }
        // --checksum=[] --checksum1000=[optional] --size=[]
        parser.addOption(QCommandLineOption("checksum", "checksum", "checksum"));
        parser.addOption(QCommandLineOption("checksum1000", "checksum", "checksum1000"));
        parser.addOption(QCommandLineOption("size", "size", "size"));
        QCommandLineOption pathOption("path", "path to search", "size");
        pathOption.setHidden(true);
        parser.addOption(pathOption);
    }
    int execute(const QCommandLineParser &parser)
    {
        QStringList args = parser.positionalArguments();
        args.removeFirst(); // remove command name

        QJsonObject obj;
        if (parser.isSet("checksum")) {
            obj["original_checksum"]=parser.value("checksum");
            obj["original_checksum_1000"]=parser.value("checksum1000");
            obj["original_size"]=parser.value("size").toLongLong();
        }
        else {
            QString src_path=args.at(0);
            if (src_path.isEmpty()) {
                println("Source path is empty");
                return -1;
            }
            if (!QFile::exists(src_path)) {
                qWarning() << "No such file: "+src_path;
                return -1;
            }
            if (src_path.endsWith(".prv")) {
                obj=QJsonDocument::fromJson(read_text_file(src_path).toUtf8()).object();
            }
            else {
                if (m_cmd=="locate") {
                    obj["original_checksum"]=sumit(src_path);
                    obj["original_checksum_1000"]=sumit(src_path,1000);
                    obj["original_size"]=QFileInfo(src_path).size();
                }
                else {
                    println("prv file must have .prv extension");
                    return -1;
                }
            }
        }
        if (obj.contains("original_checksum")) {
            QVariantMap params;
            if (parser.isSet("path"))
                params.insert("path", parser.value("path"));
            if (m_cmd=="locate")
                locate_file(obj,params);
            else
                download_file(obj,params);
        }
        else {
            printf("Only files can be found using 'locate' or 'download'.\n");
            return -1;
        }
        return 0;
    }
private:
    QString m_cmd;

    int locate_file(const QJsonObject &obj,const QVariantMap &params) const {
        PrvFile prvf(obj);
        PrvFileLocateOptions opts;
        opts.local_search_paths=get_local_search_paths();
        opts.search_remotely=true;
        opts.remote_servers=get_remote_servers();

        if (params.contains("path")) {
            opts.local_search_paths.clear();
            opts.local_search_paths << params["path"].toString();
            opts.search_remotely=false;
        }

        qDebug() << "---------------------------" << obj << opts.local_search_paths;

        QString fname_or_url=prvf.locate(opts);
        if (fname_or_url.isEmpty())
            return -1;
        println(fname_or_url);
        return 0;
    }

    int download_file(const QJsonObject &obj,const QVariantMap &params) const {
        PrvFile prvf(obj);
        PrvFileLocateOptions opts;
        opts.local_search_paths=get_local_search_paths();
        opts.search_remotely=true;
        opts.remote_servers=get_remote_servers();

        if (params.contains("path")) {
            opts.local_search_paths.clear();
            opts.local_search_paths << params["path"].toString();
            opts.search_remotely=false;
        }

        QString fname_or_url=prvf.locate(opts);
        if (fname_or_url.isEmpty())
            return -1;

        QString cmd;
        if (is_url(fname_or_url)) {
            cmd=QString("curl %1").arg(fname_or_url);
        }
        else {
            cmd=QString("cat %1").arg(fname_or_url);
        }
        return system(cmd.toUtf8().data());
    }
};

} // namespace PrvCommands

int main(int argc,char *argv[]) {
    QCoreApplication app(argc,argv);

    QJsonObject config=get_config();
    CacheManager::globalInstance()->setLocalBasePath(get_tmp_path());

    MLUtils::ApplicationCommandParser cmdParser;
    cmdParser.addCommand(new PrvCommands::Sha1SumCommand);
    cmdParser.addCommand(new PrvCommands::StatCommand);
    cmdParser.addCommand(new PrvCommands::CreateCommand);
    cmdParser.addCommand(new PrvCommands::LocateDownloadCommand("locate"));
    cmdParser.addCommand(new PrvCommands::LocateDownloadCommand("download"));
    cmdParser.addCommand(new PrvCommands::RecoverCommand);
    cmdParser.addCommand(new PrvCommands::ListSubserversCommand);
    cmdParser.addCommand(new PrvCommands::UploadCommand);
    if (!cmdParser.process(app)) {
        return cmdParser.result();
    }
    return cmdParser.result();
}

void print(QString str) {
    printf("%s",str.toUtf8().constData());
}

void println(QString str) {
    printf("%s\n",str.toUtf8().constData());
}

QStringList get_local_search_paths() {
    QJsonObject config=get_config();
    QJsonArray local_search_paths0=config.value("local_search_paths").toArray();
    QStringList local_search_paths;
    for (int i=0; i<local_search_paths0.count(); i++)
        local_search_paths << local_search_paths0[0].toString();
    QString temporary_path=config.value("temporary_path").toString();
    if (!temporary_path.isEmpty()) {
        local_search_paths << temporary_path;
    }
    return local_search_paths;
}

QJsonObject get_config() {
    QString fname1=qApp->applicationDirPath()+"/../prv.json.default";
    QString fname2=qApp->applicationDirPath()+"/../prv.json";
    QJsonParseError err1;
    QJsonObject obj1=QJsonDocument::fromJson(read_text_file(fname1).toUtf8(),&err1).object();
    if (err1.error!=QJsonParseError::NoError) {
        qWarning() << "Error parsing configuration file: "+fname1+": "+err1.errorString();
        abort();
    }
    QJsonObject obj2;
    if (QFile::exists(fname2)) {
        QJsonParseError err2;
        obj2=QJsonDocument::fromJson(read_text_file(fname2).toUtf8(),&err2).object();
        if (err2.error!=QJsonParseError::NoError) {
            qWarning() << "Error parsing configuration file: "+fname2+": "+err2.errorString();
            abort();
        }
    }
    obj1=obj1["prv"].toObject();
    obj2=obj2["prv"].toObject();
    QStringList keys2=obj2.keys();
    foreach (QString key,keys2) {
        obj1[key]=obj2[key];
    }
    return obj1;
}

QString get_tmp_path() {
    QJsonObject config=get_config();
    QString temporary_path=config["temporary_path"].toString();
    if (temporary_path.isEmpty()) return "";
    QDir(temporary_path).mkdir("prv");
    return temporary_path+"/prv";
}

QString make_temporary_file() {
    QString file_name=make_random_id(10)+".tmp";
    return get_tmp_path()+"/"+file_name;
}

bool is_file(QString path) {
    return QFileInfo(path).isFile();
}
bool is_folder(QString path) {
    return QFileInfo(path).isDir();
}


QString get_server_url(QString url_or_server_name) {
    QJsonObject config=get_config();
    QJsonArray remote_servers=config.value("servers").toArray();
    for (int i=0; i<remote_servers.count(); i++) {
        QJsonObject server0=remote_servers[i].toObject();
        if (server0["name"].toString()==url_or_server_name) {
            QString host=server0["host"].toString();
            int port=server0["port"].toInt();
            QString url_path=server0["path"].toString();
            QString url0=host+":"+QString::number(port)+url_path;
            return url0;
        }
    }
    return url_or_server_name;
}

QJsonArray get_remote_servers() {
    QJsonObject config=get_config();
    QJsonArray remote_servers=config.value("servers").toArray();
    return remote_servers;
}
