/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/2/2016
*******************************************************/

#ifndef MPDAEMONINTERFACE_H
#define MPDAEMONINTERFACE_H

#include <QJsonObject>
#include "mpdaemon.h"

class MPDaemonInterfacePrivate;
class MPDaemonInterface {
public:
    friend class MPDaemonInterfacePrivate;
    MPDaemonInterface();
    virtual ~MPDaemonInterface();
    bool start();
    bool stop();
    QJsonObject getInfo();
    bool queueScript(const MPDaemonPript& script);
    bool queueProcess(const MPDaemonPript& process);

private:
    MPDaemonInterfacePrivate* d;
};

#endif // MPDAEMONINTERFACE_H
