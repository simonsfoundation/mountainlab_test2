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
#include "textfile.h"

class ProcessManagerPrivate {
public:
    ProcessManager *q;

    QMap<QString,MLProcessor> m_processors;

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
        if (P.name.isEmpty()) {
            qWarning() << "Problem with processor file: processor error: "+path;
            return false;
        }
        d->m_processors[P.name]=P;
    }
    return true;
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
