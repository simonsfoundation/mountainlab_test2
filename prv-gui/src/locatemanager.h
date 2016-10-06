#ifndef LOCATEMANAGER_H
#define LOCATEMANAGER_H

#include <QObject>
#include "prvgui.h"

class LocateManagerPrivate;
class LocateManager : public QObject
{
    Q_OBJECT
public:
    friend class LocateManagerPrivate;
    LocateManager();
    virtual ~LocateManager();
    void startSearchForPrv(QString checksum,long size,QString server);
    fuzzybool getSearchState(const PrvRecord &prv,QString server);
    QString getResultPathOrUrl(const PrvRecord &prv,QString server);
signals:
    void searchStatesUpdated();

private slots:
    void slot_worker_finished();

private:
    LocateManagerPrivate *d;
};

#endif // LOCATEMANAGER_H
