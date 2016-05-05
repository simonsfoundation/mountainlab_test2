/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/2/2016
*******************************************************/

#ifndef MPDAEMON_H
#define MPDAEMON_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QJsonObject>
#include <QProcess>

class MPDaemonPrivate;
class MPDaemon : public QObject {
    Q_OBJECT
public:
    friend class MPDaemonPrivate;
    MPDaemon();
    virtual ~MPDaemon();
    bool run();

    static QString daemonPath();
    static QString makeTimestamp(const QDateTime& dt = QDateTime::currentDateTime());
    static QDateTime parseTimestamp(const QString& timestamp);
    static bool waitForFileToAppear(QString fname, qint64 timeout_ms = -1, bool remove_on_appear = false, qint64 parent_pid=0);
    static void wait(qint64 msec);
    static bool pidExists(qint64 pid);

private
slots:
    void slot_commands_directory_changed();
    void slot_pript_qprocess_finished();

private:
    MPDaemonPrivate* d;
};

enum PriptType {
    ScriptType,
    ProcessType
};

struct MPDaemonPript {
    //Represents a process or a script
    MPDaemonPript()
    {
        is_running = false;
        is_finished = false;
        success=false;
        parent_pid=0;
        qprocess=0;
        prtype=ScriptType;
    }
    PriptType prtype;
    QString id;
    QString output_file;
    QVariantMap parameters;
    bool is_running;
    bool is_finished;
    bool success;
    QString error;
    QJsonObject run_time_results;
    qint64 parent_pid;
    QProcess *qprocess;

    //For a script:
    QStringList script_paths;

    //For a process:
    QString processor_name;
};

QJsonObject pript_struct_to_obj(MPDaemonPript S);
MPDaemonPript pript_obj_to_struct(QJsonObject obj);
QJsonArray stringlist_to_json_array(QStringList list);
QStringList json_array_to_stringlist(QJsonArray X);
QJsonObject variantmap_to_json_obj(QVariantMap map);
QVariantMap json_obj_to_variantmap(QJsonObject obj);

#endif // MPDAEMON_H
