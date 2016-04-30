/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/27/2016
*******************************************************/

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QString>
#include <QVariant>
#include <QProcess>

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

struct MLProcessInfo {
    QString processor_name;
    QVariantMap parameters;
    QString exe_command;
    bool finished;
    int exit_code;
    QProcess::ExitStatus exit_status;
    QByteArray standard_output;
    QByteArray standard_error;
};


class ProcessManagerPrivate;
class ProcessManager : public QObject
{
    Q_OBJECT
public:
    friend class ProcessManagerPrivate;
    ProcessManager();
    virtual ~ProcessManager();

    bool loadProcessors(const QString &path,bool recursive=true);
    bool loadProcessorFile(const QString &path);

    bool checkParameters(const QString &processor_name,const QVariantMap &parameters);
    QString startProcess(const QString &processor_name,const QVariantMap &parameters); //returns the process id/handle (a random string)
    bool waitForFinished(const QString &process_id,int msecs);
    MLProcessInfo processInfo(const QString &id);
    void clearProcess(const QString &id);
    void clearAllProcesses();

    QStringList allProcessIds() const;

    bool isFinished(const QString &id);

signals:
    void processFinished(QString id);
private slots:
    void slot_process_finished();
private:
    ProcessManagerPrivate *d;
};

QString make_random_id(int numchars=20);

#endif // PROCESSMANAGER_H

