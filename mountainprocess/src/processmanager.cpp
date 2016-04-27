/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#include "processmanager.h"
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include "textfile.h"

struct RunningProcess {
    QProcess *qprocess;
    QString processor_name;
    QVariantMap parameters;
    QString exe_command;
};

class ProcessManagerPrivate {
public:
    ProcessManager *q;

    QMap<QString,MLProcessor> m_processors;
    QMap<QString,RunningProcess> m_running_processes;

    void clear_running_processes();

    static MLProcessor create_processor_from_json_object(QJsonObject obj);
    static MLParameter create_parameter_from_json_object(QJsonObject obj);
};

ProcessManager::ProcessManager()
{
    d=new ProcessManagerPrivate;
    d->q=this;
}

ProcessManager::~ProcessManager()
{
    d->clear_running_processes();
    delete d;
}

bool ProcessManager::loadProcessors(const QString &path, bool recursive)
{
    QStringList fnames=QDir(path).entryList(QStringList("*.mp"),QDir::Files,QDir::Name);
    foreach (QString fname,fnames) {
        if (!this->loadProcessorFile(path+"/"+fname)) return false;
    }
    if (recursive) {
        QStringList subdirs=QDir(path).entryList(QStringList("*"),QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
        foreach (QString subdir,subdirs) {
            if (!this->loadProcessors(path+"/"+subdir)) return false;
        }
    }
    return true;
}

bool ProcessManager::loadProcessorFile(const QString &path)
{
    if (QFileInfo(path).isExecutable()) {
        qWarning() << "Case of executable processor file is not yet supported: "+path;
        return false;
    }
    QString json=read_text_file(path);
    if (json.isEmpty()) {
        qWarning() << "Processor file is empty: "+path;
        return false;
    }
    QJsonObject obj=QJsonDocument::fromJson(json.toLatin1()).object();
    if (!obj["processors"].isArray()) {
        qWarning() << "Problem with processor file: processors field is missing or not any array: "+path;
        return false;
    }
    QJsonArray processors=obj["processors"].toArray();
    for (int i=0; i<processors.count(); i++) {
        if (!processors[i].isObject()) {
            qWarning() << "Problem with processor file: processor is not an object: "+path;
            return false;
        }
        MLProcessor P=d->create_processor_from_json_object(processors[i].toObject());
        P.basepath=QFileInfo(path).path();
        if (P.name.isEmpty()) {
            qWarning() << "Problem with processor file: processor error: "+path;
            return false;
        }
        d->m_processors[P.name]=P;
    }
    return true;
}

QString ProcessManager::startProcess(const QString &processor_name, const QVariantMap &parameters)
{
    if (!this->checkParameters(processor_name,parameters)) return "";

    if (!d->m_processors.contains(processor_name)) {
        qWarning() << "Unable to find processor: "+processor_name;
        return false;
    }
    MLProcessor P=d->m_processors[processor_name];

    QString exe_command=P.exe_command;
    exe_command.replace(QRegExp("$basepath"),P.basepath);
    {
        QString ppp;
        QStringList keys=P.inputs.keys();
        foreach (QString key,keys) {
            ppp+=QString("--%1=%2 ").arg(key).arg(parameters[key].toString());
        }

        exe_command.replace(QRegExp("$input"),ppp);
    }
    {
        QString ppp;
        QStringList keys=P.outputs.keys();
        foreach (QString key,keys) {
            ppp+=QString("--%1=%2 ").arg(key).arg(parameters[key].toString());
        }
        exe_command.replace(QRegExp("$output"),ppp);
    }
    {
        QString ppp;
        QStringList keys=P.parameters.keys();
        foreach (QString key,keys) {
            QVariant default_val=P.parameters[key].default_value;
            ppp+=QString("--%1=%2 ").arg(key).arg(parameters.value(key,default_val).toString());
        }
        exe_command.replace(QRegExp("$parameters"),ppp);
    }

    QString id=make_random_id();
    RunningProcess RP;
    RP.exe_command=exe_command;
    RP.parameters=parameters;
    RP.processor_name=processor_name;
    RP.qprocess=new QProcess;
    RP.qprocess->start(RP.exe_command);
    d->m_running_processes[id]=RP;
    return id;
}

bool ProcessManager::checkParameters(const QString &processor_name, const QVariantMap &parameters)
{
    if (!d->m_processors.contains(processor_name)) {
        qWarning() << "checkProcess: Unable to find processor: "+processor_name;
        return false;
    }
    MLProcessor P=d->m_processors[processor_name];
    {
        QStringList keys=P.inputs.keys();
        foreach (QString key,keys) {
            if (!parameters.contains(key)) {
                qWarning() << QString("checkProcess: Missing input in %1: %2").arg(processor_name).arg(key);
                return false;
            }
        }
    }
    {
        QStringList keys=P.outputs.keys();
        foreach (QString key,keys) {
            if (!parameters.contains(key)) {
                qWarning() << QString("checkProcess: Missing output in %1: %2").arg(processor_name).arg(key);
                return false;
            }
        }
    }
    {
        QStringList keys=P.parameters.keys();
        foreach (QString key,keys) {
            if (!P.parameters[key].optional) {
                if (!parameters.contains(key)) {
                    qWarning() << QString("checkProcess: Missing required parameter in %1: %2").arg(processor_name).arg(key);
                    return false;
                }
            }
        }
    }
    return true;
}


void ProcessManagerPrivate::clear_running_processes()
{
    foreach (RunningProcess P,m_running_processes) {
        delete P.qprocess;
    }
    m_running_processes.clear();
}

MLProcessor ProcessManagerPrivate::create_processor_from_json_object(QJsonObject obj)
{
    MLProcessor P;
    P.name=obj["name"].toString();
    P.version=obj["version"].toString();
    P.description=obj["description"].toString();

    QJsonArray inputs=obj["inputs"].toArray();
    for (int i=0; i<inputs.count(); i++) {
        MLParameter param=create_parameter_from_json_object(inputs[i].toObject());
        P.inputs[param.name]=param;
    }

    QJsonArray outputs=obj["outputs"].toArray();
    for (int i=0; i<outputs.count(); i++) {
        MLParameter param=create_parameter_from_json_object(outputs[i].toObject());
        P.inputs[param.name]=param;
    }

    QJsonArray parameters=obj["parameters"].toArray();
    for (int i=0; i<parameters.count(); i++) {
        MLParameter param=create_parameter_from_json_object(parameters[i].toObject());
        P.parameters[param.name]=param;
    }

    P.exe_command=obj["exe_command"].toString();

    return P;
}

MLParameter ProcessManagerPrivate::create_parameter_from_json_object(QJsonObject obj)
{
    MLParameter param;
    param.name=obj["name"].toString();
    param.ptype=obj["ptype"].toString();
    param.description=obj["description"].toString();
    param.optional=obj["optional"].toBool();
    param.default_value=obj["default_value"].toVariant();
    return param;
}
