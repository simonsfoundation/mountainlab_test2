/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#include "msscriptcontroller.h"

#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include "mscachemanager.h"
#include "msprocessmanager.h"

class MSScriptControllerPrivate {
public:
    MSScriptController *q;
};

MSScriptController::MSScriptController()
{
    d=new MSScriptControllerPrivate;
    d->q=this;
}

MSScriptController::~MSScriptController()
{
    delete d;
}

QString MSScriptController::fileChecksum(const QString &fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly)) return "";
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);
    file.close();
    return QString(hash.result().toHex());
}

QString MSScriptController::stringChecksum(const QString &str)
{
    QByteArray X=str.toLatin1();
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(X);
    return QString(hash.result().toHex());
}

QString MSScriptController::createTemporaryFileName(const QString &code)
{
    return MSCacheManager::globalInstance()->makeLocalFile(code,MSCacheManager::LongTerm);
}

bool MSScriptController::runProcess(const QString &processor_name, const QString &parameters_json)
{
    QJsonObject params=QJsonDocument::fromJson(parameters_json.toLatin1()).object();
    QStringList keys=params.keys();
    QMap<QString,QVariant> parameters;
    foreach (QString key,keys) {
        parameters[key]=params[key].toString();
    }
    return MSProcessManager::globalInstance()->checkAndRunProcessIfNecessary(processor_name,parameters);
}
