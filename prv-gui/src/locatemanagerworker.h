#ifndef LOCATEMANAGERWORKER_H
#define LOCATEMANAGERWORKER_H

#include "prvgui.h"

struct LMResult {
    LMResult() {}
    LMResult(const LMResult& other)
    {
        state = other.state;
        path_or_url = other.path_or_url;
    }

    fuzzybool state = fuzzybool::UNKNOWN;
    QString path_or_url;
};

class LocateManagerWorkerPrivate;
class LocateManagerWorker : public QObject {
    Q_OBJECT
public:
    friend class LocateManagerWorkerPrivate;
    LocateManagerWorker();
    virtual ~LocateManagerWorker();

    void setInput(const PrvRecord& prv, QString server);
    bool matches(const PrvRecord& prv, QString server) const;
    PrvRecord prv() const;
    QString server() const;
    LMResult result() const;

    void startSearch();
    bool wasStarted() const;
    bool isFinished() const;
signals:
    void searchFinished();
private slots:
    void slot_process_finished();

private:
    LocateManagerWorkerPrivate* d;
};

#endif // LOCATEMANAGERWORKER_H
