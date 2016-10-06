#include "locatemanager.h"
#include "locatemanagerworker.h"
#include <QDebug>

class LocateManagerPrivate {
public:
    LocateManager *q;
    QList<LocateManagerWorker *> m_workers;
    QMap<QString,LMResult> m_results;
    int m_max_simultaneous=5;

    void collect_results();
    void start_workers_as_needed();
    QString get_code(const PrvRecord &prv,QString server);
    bool start_next_worker();
};

LocateManager::LocateManager()
{
    d=new LocateManagerPrivate;
    d->q=this;
}

LocateManager::~LocateManager()
{
    delete d;
}

void LocateManager::startSearchForPrv(QString checksum,long size, QString server)
{
    PrvRecord prv;
    prv.checksum=checksum;
    prv.size=size;

    //see if we are already doing that search, and cancel it
    for (int i=0; i<d->m_workers.count(); i++) {
        if (d->m_workers[i]->matches(prv,server)) {
            delete d->m_workers[i];
            d->m_workers.removeAt(i);
            i--;
        }
    }

    LocateManagerWorker *W=new LocateManagerWorker;
    QObject::connect(W,SIGNAL(searchFinished()),this,SLOT(slot_worker_finished()));
    W->setInput(prv,server);
    d->m_workers << W;

    QString code=d->get_code(prv,server);
    d->m_results[code].state=fuzzybool::UNKNOWN;
    d->m_results[code].path_or_url="";

    d->start_workers_as_needed();
}

fuzzybool LocateManager::getSearchState(const PrvRecord &prv, QString server)
{
    QString code=d->get_code(prv,server);
    if (d->m_results.contains(code)) {
        return d->m_results[code].state;
    }
    else {
        return fuzzybool::UNKNOWN;
    }
}

QString LocateManager::getResultPathOrUrl(const PrvRecord &prv, QString server)
{
    QString code=d->get_code(prv,server);
    if (d->m_results.contains(code)) {
        return d->m_results.value(code).path_or_url;
    }
    else {
        return "";
    }
}

void LocateManager::slot_worker_finished()
{
    d->collect_results();
    d->start_workers_as_needed();
}

void LocateManagerPrivate::collect_results()
{
    bool something_finished=false;
    for (int i=0; i<m_workers.count(); i++) {
        LocateManagerWorker *W=m_workers[i];
        if (W->isFinished()) {
            QString code=get_code(W->prv(),W->server());
            m_results[code]=W->result();
            something_finished=true;
            //delete W;
            //m_workers.removeAt(i);
            //i--;
        }
    }
    if (something_finished) {
        emit q->searchStatesUpdated();
    }
}

void LocateManagerPrivate::start_workers_as_needed()
{
    int num_running=0;
    for (int i=0; i<m_workers.count(); i++) {
        LocateManagerWorker *W=m_workers[i];
        if ((W->wasStarted())&&(!W->isFinished())) {
            num_running++;
        }
    }
    while (num_running<m_max_simultaneous) {
        if (start_next_worker()) {
            num_running++;
        }
        else {
            break;
        }
    }
    for (int i=0; i<m_workers.count(); i++) {
        LocateManagerWorker *W=m_workers[i];
        if (W->isFinished()) {
            W->deleteLater();
            m_workers.removeAt(i);
                i--;
        }
    }
    qDebug() << "NUM WORKERS: " << m_workers.count();
}

QString LocateManagerPrivate::get_code(const PrvRecord &prv, QString server)
{
    return QString("%1:%2:%3").arg(prv.checksum).arg(prv.size).arg(server);
}

bool LocateManagerPrivate::start_next_worker()
{
    for (int i=0; i<m_workers.count(); i++) {
        LocateManagerWorker *W=m_workers[i];
        if (!W->wasStarted()) {
            W->startSearch();
            return true;
        }
    }
    return false; //nothing to start
}
