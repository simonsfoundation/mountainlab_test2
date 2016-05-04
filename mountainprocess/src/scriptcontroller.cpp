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
#include <QCoreApplication>

class ScriptControllerPrivate {
public:
    ScriptController* q;

    bool queue_process_and_wait_for_finished(QString processor_name, const QVariantMap& parameters);
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
    if (PM->processAlreadyCompleted(processor_name, parameters)) {
        printf("Process already completed: %s\n", processor_name.toLatin1().data());
        return true;
    }

    return d->queue_process_and_wait_for_finished(processor_name, parameters);
    /*
    QString process_id = PM->startProcess(processor_name, parameters);
    if (process_id.isEmpty())
        return false;
    if (!PM->waitForFinished(process_id))
        return false;
    PM->clearProcess(process_id);
    return true;
    */
}

void ScriptController::log(const QString& message)
{
    printf("SCRIPT: %s\n", message.toLatin1().data());
}

bool ScriptControllerPrivate::queue_process_and_wait_for_finished(QString processor_name, const QVariantMap& parameters)
{
    QString exe = qApp->applicationDirPath();
    QStringList args;
    args << "queue-process";
    QStringList pkeys = parameters.keys();
    foreach (QString pkey, pkeys) {
        args << QString("--%1=%2").arg(pkey).arg(parameters[pkey].toString());
    }
    QProcess P1;
    P1.start(exe, args);
    if (!P1.waitForFinished(30000)) {
        printf("Error waiting for queue-process to finish: %s\n", processor_name.toLatin1().data());
        return false;
    }
    if (P1.exitStatus() == QProcess::CrashExit) {
        printf("Error -- queue-process crashed: %s\n", processor_name.toLatin1().data());
        return false;
    }
    if (P1.exitCode() != 0) {
        printf("Error -- queue-process returned non-zero exit code: %s\n", processor_name.toLatin1().data());
        return false;
    }
    /////// FINISH!!!!!!!!!!!!!!!
    return true;
}
