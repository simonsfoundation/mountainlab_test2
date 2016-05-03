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

private slots:
    void slot_commands_directory_changed();

private:
    MPDaemonPrivate* d;
};

struct MPDaemonScript {
    QString script_id;
    QStringList script_paths;
    QVariantMap parameters;
    bool is_running;
    bool is_finished;
};
QJsonObject script_struct_to_obj(MPDaemonScript S);
MPDaemonScript script_obj_to_struct(QJsonObject obj);
/// Witold there is probably a better way to set the default struct. is this a struct constructor?
MPDaemonScript default_daemon_script();

#endif // MPDAEMON_H
