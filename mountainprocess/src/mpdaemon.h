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

class MPDaemonPrivate;
class MPDaemon : public QObject
{
    Q_OBJECT
public:
    friend class MPDaemonPrivate;
    MPDaemon();
    virtual ~MPDaemon();
    bool run();

    static QString daemonPath();
    static QString makeTimestamp(const QDateTime &dt=QDateTime::currentDateTime());
    static QDateTime parseTimestamp(const QString &timestamp);

private slots:
    void slot_timer();
private:
    MPDaemonPrivate *d;
};

#endif // MPDAEMON_H

