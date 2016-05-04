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
    static bool waitForFileToAppear(QString fname, qint64 timeout_ms = -1, bool remove_on_appear = false);
    static void wait(qint64 msec);

private
slots:
    void slot_commands_directory_changed();
    void slot_script_qprocess_finished();
    void slot_process_qprocess_finished();

private:
    MPDaemonPrivate* d;
};

struct MPDaemonScript {
    MPDaemonScript()
    {
        is_running = false;
        is_finished = false;
        success=false;
    }
    QString script_id;
    QString script_output_file;
    QStringList script_paths;
    QVariantMap parameters;
    bool is_running;
    bool is_finished;
    bool success;
    QString error;
    QJsonObject run_time_results;
};

struct MPDaemonProcess {
    MPDaemonProcess()
    {
        is_running = false;
        is_finished = false;
        success=false;
    }
    QString process_id;
    QString process_output_file;
    QString processor_name;
    QVariantMap parameters;
    bool is_running;
    bool is_finished;
    bool success;
    QString error;
    QJsonObject run_time_results;
};

QJsonObject script_struct_to_obj(MPDaemonScript S);
MPDaemonScript script_obj_to_struct(QJsonObject obj);
QJsonObject process_struct_to_obj(MPDaemonProcess P);
MPDaemonProcess process_obj_to_struct(QJsonObject obj);
QJsonArray stringlist_to_json_array(QStringList list);
QStringList json_array_to_stringlist(QJsonArray X);
QJsonObject variantmap_to_json_obj(QVariantMap map);
QVariantMap json_obj_to_variantmap(QJsonObject obj);

#endif // MPDAEMON_H
