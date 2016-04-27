/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QString>
#include <QVariant>

struct MLParameter {
    QString name;
    QString ptype;
    QString description;
    bool optional;
    QVariant default_value;
};

struct MLProcessor {
    QString name;
    QString version;
    QString description;
    QMap<QString,MLParameter> inputs;
    QMap<QString,MLParameter> outputs;
    QMap<QString,MLParameter> parameters;
    QString exe_command;

    QString basepath;
};

class ProcessManagerPrivate;
class ProcessManager
{
public:
    friend class ProcessManagerPrivate;
    ProcessManager();
    virtual ~ProcessManager();

    bool loadProcessors(const QString &path,bool recursive=true);
    bool loadProcessorFile(const QString &path);
    QString startProcess(const QString &processor_name,const QVariantMap &parameters); //returns the process id/handle (a random string)
    bool checkParameters(const QString &processor_name,const QVariantMap &parameters);
private:
    ProcessManagerPrivate *d;
};

#endif // PROCESSMANAGER_H

