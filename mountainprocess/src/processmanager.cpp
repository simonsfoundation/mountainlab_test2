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
#include <QTime>
#include <QEventLoop>
#include "textfile.h"

struct PMProcess {
    MLProcessInfo info;
    QProcess *qprocess;
};

class ProcessManagerPrivate {
public:
    ProcessManager *q;

    QMap<QString,MLProcessor> m_processors;
    QMap<QString,PMProcess> m_processes;

    void clear_all_processes();
    void update_process_info(QString id);

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
    d->clear_all_processes();
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
    QJsonParseError error;
    QJsonObject obj=QJsonDocument::fromJson(json.toLatin1(),&error).object();
    if (error.error!=QJsonParseError::NoError) {
        qWarning() << "Json parse error: " << error.errorString();
        return false;
    }
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
        return "";
    }
    MLProcessor P=d->m_processors[processor_name];

    qDebug() << P.inputs.keys();
    qDebug() << P.outputs.keys();
    qDebug() << P.parameters.keys();

    QString exe_command=P.exe_command;
    exe_command.replace(QRegExp("\\$\\(basepath\\)"),P.basepath);
    {
        QString ppp;
        {
            QStringList keys=P.inputs.keys();
            foreach (QString key,keys) {
                exe_command.replace(QRegExp(QString("\\$%1").arg(key)),parameters[key].toString());
                ppp+=QString("--%1=%2 ").arg(key).arg(parameters[key].toString());
            }
        }
        {
            QStringList keys=P.outputs.keys();
            foreach (QString key,keys) {
                exe_command.replace(QRegExp(QString("\\$%1").arg(key)),parameters[key].toString());
                ppp+=QString("--%1=%2 ").arg(key).arg(parameters[key].toString());
            }
        }
        {
            QStringList keys=P.parameters.keys();
            foreach (QString key,keys) {
                exe_command.replace(QRegExp(QString("$%1").arg(key)),parameters[key].toString());
                ppp+=QString("--%1=%2 ").arg(key).arg(parameters[key].toString());
            }
        }

        exe_command.replace(QRegExp("\\$\\(arguments\\)"),ppp);
    }

    QString id=make_random_id();
    PMProcess PP;
    PP.info.exe_command=exe_command;
    PP.info.parameters=parameters;
    PP.info.processor_name=processor_name;
    PP.info.finished=false;
    PP.info.exit_code=0;
    PP.info.exit_status=QProcess::NormalExit;
    PP.qprocess=new QProcess;
    QObject::connect(PP.qprocess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slot_process_finished()));
    qDebug()  << "STARTING: " << PP.info.exe_command;
    PP.qprocess->start(PP.info.exe_command);
    PP.qprocess->setProperty("pp_id",id);
    if (!PP.qprocess->waitForStarted(2000)) {
        qWarning() << "Problem starting process: "+exe_command;
        delete PP.qprocess;
        return "";
    }
    d->m_processes[id]=PP;
    return id;
}

bool ProcessManager::waitForFinished(const QString &process_id,int msecs)
{
    if (!d->m_processes.contains(process_id)) return false;
    QProcess *qprocess=d->m_processes[process_id].qprocess;
    return qprocess->waitForFinished(msecs);
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

QStringList ProcessManager::allProcessIds() const
{
    return d->m_processes.keys();
}

void ProcessManager::clearProcess(const QString &id)
{
    if (!d->m_processes.contains(id)) return;
    QProcess *qprocess=d->m_processes[id].qprocess;
    d->m_processes.remove(id);
    if (qprocess->state()==QProcess::Running) {
        d->m_processes[id].qprocess->kill();;
    }
    delete qprocess;
}

void ProcessManager::clearAllProcesses()
{
    d->clear_all_processes();
}

MLProcessInfo ProcessManager::processInfo(const QString &id)
{
    if (!d->m_processes.contains(id)) {
        MLProcessInfo dummy;
        dummy.finished=true;
        dummy.exit_code=0;
        dummy.exit_status=QProcess::NormalExit;
        return dummy;
    }
    d->update_process_info(id);
    return d->m_processes[id].info;
}

bool ProcessManager::isFinished(const QString &id)
{
    return processInfo(id).finished;
}

void ProcessManager::slot_process_finished()
{
    QProcess *qprocess=qobject_cast<QProcess *>(sender());
    if (!qprocess) {
        qWarning() << "Unexpected problem in slot_process_finished: qprocess is null.";
        return;
    }
    QString id=qprocess->property("pp_id").toString();
    if (!d->m_processes.contains(id)) {
        qWarning() << "Unexpected problem in slot_process_finished. id not found in m_processes: "+id;
        return;
    }
    d->update_process_info(id);
    emit this->processFinished(id);
}


void ProcessManagerPrivate::clear_all_processes()
{
    foreach (PMProcess P,m_processes) {
        delete P.qprocess;
    }
    m_processes.clear();
}

void ProcessManagerPrivate::update_process_info(QString id)
{
    if (!m_processes.contains(id)) return;
    PMProcess *PP=&m_processes[id];
    QProcess *qprocess=PP->qprocess;
    if (qprocess->state()==QProcess::NotRunning) {
        PP->info.finished=true;
        PP->info.exit_code=qprocess->exitCode();
        PP->info.exit_status=qprocess->exitStatus();
    }
    PP->info.standard_output+=qprocess->readAllStandardOutput();
    PP->info.standard_error+=qprocess->readAllStandardError();
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
        P.outputs[param.name]=param;
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

QChar make_random_alphanumeric() {
        static int val=0;
        val++;
        QTime time=QTime::currentTime();
        int num=qHash(time.toString("hh:mm:ss:zzz")+QString::number(qrand()+val));
        num=num%36;
        if (num<26) return QChar('A'+num);
        else return QChar('0'+num-26);
}
QString make_random_id(int numchars) {
        QString ret;
        for (int i=0; i<numchars; i++) {
                ret.append(make_random_alphanumeric());
        }
        return ret;
}
