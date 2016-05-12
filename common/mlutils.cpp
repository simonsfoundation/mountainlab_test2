/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/11/2016
*******************************************************/

#include "mlutils.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QCoreApplication>

QString cfp(const QString &path)
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


QString compute_checksum_of_file(const QString &path)
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

QString find_ancestor_path_with_name(QString path,QString name) {
    if (name.isEmpty()) return "";
    while (QFileInfo(path).fileName()!=name) {
        path=QFileInfo(path).path();
        if (!path.contains(name)) return ""; //guarantees that we eventually exit the while loop
    }
    return path; //the directory name must equal the name argument
}

QString mountainlabBasePath()
{
    return find_ancestor_path_with_name(qApp->applicationDirPath(),"mountainlab");
}
