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
#include "cachemanager.h"
#include "msprocessmanager.h"
#include <QTime>

class MSScriptControllerPrivate {
public:
    MSScriptController* q;
};

MSScriptController::MSScriptController()
{
    d = new MSScriptControllerPrivate;
    d->q = this;
}

MSScriptController::~MSScriptController()
{
    delete d;
}

QString MSScriptController::fileChecksum(const QString& fname)
{
    QTime timer;
    timer.start();
    printf("Computing checksum for file %s\n", fname.toLatin1().data());
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly))
        return "";
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);
    file.close();
    QString ret = QString(hash.result().toHex());
    printf("%s -- Elapsed: %g sec\n", ret.toLatin1().data(), timer.elapsed() * 1.0 / 1000);
    return ret;
}

QString MSScriptController::stringChecksum(const QString& str)
{
    QByteArray X = str.toLatin1();
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(X);
    return QString(hash.result().toHex());
}

QString MSScriptController::createTemporaryFileName(const QString& code)
{
    return CacheManager::globalInstance()->makeLocalFile(code, CacheManager::LongTerm);
}

bool MSScriptController::runProcess(const QString& processor_name, const QString& parameters_json, bool force_run)
{
    QJsonObject params = QJsonDocument::fromJson(parameters_json.toLatin1()).object();
    QStringList keys = params.keys();
    QMap<QString, QVariant> parameters;
    foreach (QString key, keys) {
        parameters[key] = params[key].toVariant();
    }
    return MSProcessManager::globalInstance()->checkAndRunProcessIfNecessary(processor_name, parameters, force_run);
}

void MSScriptController::log(const QString& message)
{
    printf("SCRIPT: %s\n", message.toLatin1().data());
}
