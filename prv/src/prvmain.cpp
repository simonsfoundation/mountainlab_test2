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

int main_sha1sum(QString path,const QVariantMap &params);
int main_stat(QString path,const QVariantMap &params);
int main_create_file_prv(QString src_path,QString dst_path,const QVariantMap &params);
int main_create_folder_prv(QString src_path,QString dst_path,const QVariantMap &params);
int main_locate_file(const QJsonObject &obj,const QVariantMap &params);
int main_download_file(const QJsonObject &obj,const QVariantMap &params);
int main_list_subservers(const QVariantMap &params);
int main_upload(QString src_path,QString server_url,const QVariantMap &params);

QJsonObject get_config();
QString get_tmp_path();
QString get_server_url(QString url_or_server_name);
QStringList get_local_search_paths();
QJsonArray get_remote_servers();

void print(QString str);
void println(QString str);
bool is_file(QString path);
bool is_folder(QString path);

int main(int argc,char *argv[]) {
    QCoreApplication app(argc,argv);

    QJsonObject config=get_config();
    CacheManager::globalInstance()->setLocalBasePath(get_tmp_path());

    CLParams CLP(argc,argv);
    QString arg1=CLP.unnamed_parameters.value(0);
    QString arg2=CLP.unnamed_parameters.value(1);
    QString arg3=CLP.unnamed_parameters.value(2);

    if (arg1=="sha1sum") {
        QString path=arg2;
        if (path.isEmpty()) {
            usage();
            return -1;
        }
        if (!QFile::exists(path)) {
            qWarning() << "No such file: "+path;
            return -1;
        }
        return main_sha1sum(path,CLP.named_parameters);
    }
    else if (arg1=="stat") {
        QString path=arg2;
        if (path.isEmpty()) {
            usage();
            return -1;
        }
        if (!QFile::exists(path)) {
            qWarning() << "No such file: "+path;
            return -1;
        }
        return main_stat(path,CLP.named_parameters);
    }
    else if (arg1=="create") {
        QString src_path=arg2;
        QString dst_path=arg3;
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
        if (is_file(src_path)) {
            return main_create_file_prv(src_path,dst_path,CLP.named_parameters);
        }
        else if (is_folder(src_path)) {
            return main_create_folder_prv(src_path,dst_path,CLP.named_parameters);
        }
        else {
            qWarning() << "not sure why file is not a file nor a folder.";
            return -1;
        }
    }
    else if ((arg1=="locate")||(arg1=="download")) {
        QJsonObject obj;
        if (CLP.named_parameters.contains("checksum")) {
            obj["original_checksum"]=CLP.named_parameters["checksum"].toString();
            obj["original_checksum_1000"]=CLP.named_parameters["checksum1000"].toString();
            obj["original_size"]=CLP.named_parameters["size"].toLongLong();
        }
        else {
            QString src_path=arg2;
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
                if (arg1=="locate") {
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
            if (arg1=="locate")
                main_locate_file(obj,CLP.named_parameters);
            else
                main_download_file(obj,CLP.named_parameters);
        }
        else {
            printf("Only files can be found using 'locate' or 'download'.\n");
            return -1;
        }
    }
    else if (arg1=="list-subservers") {
        return main_list_subservers(CLP.named_parameters);
    }
    else if (arg1=="recover") {
        QString src_path=arg2;
        QString dst_path=arg3;
        if (src_path.isEmpty()) {
            usage();
            return -1;
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
            dst_path=f0.mid(0,f0.count()-4); //remove .prv extension
        }
        PrvFile prv_file(src_path);
        PrvFileRecoverOptions opts;
        opts.recover_all_prv_files=CLP.named_parameters.contains("recover-all-prv-files");
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
    else if (arg1=="upload") {
        QString src_path=arg2;
        QString server_url=get_server_url(arg3);
        if (src_path.isEmpty()) {
            usage();
            return -1;
        }
        if (!QFile::exists(src_path)) {
            qWarning() << "No such file: "+src_path;
            return -1;
        }
        if (src_path.endsWith(".prv")) {
            qWarning() << "Cannot upload a .prv file.";
            return -1;
        }
        main_upload(src_path,server_url,CLP.named_parameters);
    }
    else {
        usage();
        return -1;
    }

    return 0;
}

void print(QString str) {
    printf("%s",str.toUtf8().data());
}

void println(QString str) {
    printf("%s\n",str.toUtf8().data());
}

int main_sha1sum(QString path,const QVariantMap &params) {
    Q_UNUSED(params)
    QString checksum=sumit(path);
    if (checksum.isEmpty()) return -1;
    println(checksum);
    return 0;
}

int main_stat(QString path,const QVariantMap &params) {
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

int main_create_file_prv(QString src_path,QString dst_path,const QVariantMap &params) {
    println("making prv file: "+dst_path);
    PrvFile PF;
    PrvFileCreateOptions opts;
    opts.create_temporary_files=params.contains("create-temporary-files");
    PF.createFromFile(src_path,opts);
    if (!PF.write(dst_path)) return -1;
    return 0;
}

int main_create_folder_prv(QString src_path,QString dst_path,const QVariantMap &params) {
    println("making folder prv: "+src_path);
    PrvFile PF;
    PrvFileCreateOptions opts;
    opts.create_temporary_files=params.contains("create-temporary-files");
    PF.createFromFolder(src_path,opts);
    if (!PF.write(dst_path)) return -1;
    return 0;
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

int main_locate_file(const QJsonObject &obj,const QVariantMap &params) {
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

int main_list_subservers(const QVariantMap &params) {
    Q_UNUSED(params)
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

int main_download_file(const QJsonObject &obj,const QVariantMap &params) {
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

QString get_user_name() {
    QStringList home_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    return home_path.first().split(QDir::separator()).last();
}

static long s_total_uploaded_bytes=0;

int main_upload(QString src_path,QString server_url,const QVariantMap &params) {

    if (QFileInfo(src_path).isDir()) {
        QStringList flist=QDir(src_path).entryList(QStringList("*"),QDir::Files,QDir::Name);
        foreach (QString f,flist) {
            int ret=main_upload(src_path+"/"+f,server_url,params);
            if (ret!=0) return ret;
        }
        QStringList dlist=QDir(src_path).entryList(QStringList("*"),QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
        foreach (QString d,dlist) {
            int ret=main_upload(src_path+"/"+d,server_url,params);
            if (ret!=0) return ret;
        }
        return 0;
    }

    long size0=QFileInfo(src_path).size();
    if (size0==0) {
        println("File is empty... skipping: "+src_path);
        return 0;
    }
    QString checksum00=sumit(src_path);
    if (checksum00.isEmpty()) {
        println("checksum is empty for file: "+src_path);
        return -1;
    }
    QString url=server_url+"?a=upload"+"&checksum="+checksum00+"&size="+QString::number(size0);
    QJsonObject info;
    info["src_path"]=QDir::current().absoluteFilePath(src_path);
    info["server_url"]=server_url;
    info["user"]=get_user_name();
    info["local_host_name"]=QHostInfo::localHostName();
    info["date_uploaded"]=QDateTime::currentDateTime().toString("yyyy-MM-dd:hh-mm-ss");
    info["params"]=QJsonObject::fromVariantMap(params);
    QString info_json=QJsonDocument(info).toJson();
    url+="&info="+QUrl::toPercentEncoding(info_json.toUtf8());

    QString ret=NetUtils::httpPostFile(url,src_path);
    if (ret.isEmpty()) {
        qWarning() << "Problem posting file to: "+url;
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
    s_total_uploaded_bytes+=size0;
    println(QString("Uploaded file %1 (%2 MB, total %3 MB).").arg(src_path).arg(size0*1.0/1e6).arg(s_total_uploaded_bytes*1.0/1e6));
    return 0;
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
