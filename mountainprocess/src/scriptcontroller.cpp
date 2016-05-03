/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/19/2016
*******************************************************/

#include "scriptcontroller.h"

#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include "cachemanager.h"
#include "processmanager.h"
#include <QTime>

class ScriptControllerPrivate {
public:
    ScriptController* q;
};

ScriptController::ScriptController()
{
    d = new ScriptControllerPrivate;
    d->q = this;
}

ScriptController::~ScriptController()
{
    delete d;
}

QString ScriptController::fileChecksum(const QString& fname)
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

QString ScriptController::stringChecksum(const QString& str)
{
    QByteArray X = str.toLatin1();
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(X);
    return QString(hash.result().toHex());
}

QString ScriptController::createTemporaryFileName(const QString& code)
{
    return CacheManager::globalInstance()->makeLocalFile(code, CacheManager::LongTerm);
}

bool ScriptController::runProcess(const QString& processor_name, const QString& parameters_json)
{
    QJsonObject params = QJsonDocument::fromJson(parameters_json.toLatin1()).object();
    QStringList keys = params.keys();
    QMap<QString, QVariant> parameters;
    foreach (QString key, keys) {
        parameters[key] = params[key].toVariant();
    }
    ProcessManager* PM = ProcessManager::globalInstance();
    if (!PM->checkParameters(processor_name, parameters)) {
        return false;
    }
    QString process_id = PM->startProcess(processor_name, parameters);
    if (process_id.isEmpty())
        return false;
    if (!PM->waitForFinished(process_id))
        return false;
    PM->clearProcess(process_id);
    return true;
}

void ScriptController::log(const QString& message)
{
    printf("SCRIPT: %s\n", message.toLatin1().data());
}
