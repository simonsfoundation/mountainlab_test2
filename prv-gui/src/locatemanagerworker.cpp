#include "locatemanagerworker.h"

#include <QFile>
#include <QProcess>
#include <QDebug>

class LocateManagerWorkerPrivate {
public:
    LocateManagerWorker* q;

    PrvRecord m_prv;
    QString m_server;

    LMResult m_result;

    QProcess m_process;
    bool m_is_finished = false;
    bool m_was_started = false;
};

LocateManagerWorker::LocateManagerWorker()
{
    d = new LocateManagerWorkerPrivate;
    d->q = this;
}

LocateManagerWorker::~LocateManagerWorker()
{
    delete d;
}

void LocateManagerWorker::setInput(const PrvRecord& prv, QString server)
{
    d->m_prv = prv;
    d->m_server = server;
}

bool LocateManagerWorker::matches(const PrvRecord& prv, QString server) const
{
    return ((d->m_server == server) && (prv.checksum == d->m_prv.checksum) && (prv.size == d->m_prv.size));
}

PrvRecord LocateManagerWorker::prv() const
{
    return d->m_prv;
}

LMResult LocateManagerWorker::result() const
{
    return d->m_result;
}

QString LocateManagerWorker::server() const
{
    return d->m_server;
}

void LocateManagerWorker::startSearch()
{
    d->m_result = LMResult();
    d->m_is_finished = false;
    d->m_was_started = true;
    QObject::connect(&d->m_process, SIGNAL(finished(int)), this, SLOT(slot_process_finished()));
    d->m_process.setReadChannelMode(QProcess::MergedChannels);
    QString cmd = QString("prv locate --checksum=%1 --checksum1000=%2 --size=%3").arg(d->m_prv.checksum).arg(d->m_prv.checksum1000).arg(d->m_prv.size);
    if (d->m_server.isEmpty()) {
        cmd += " --local-only";
    }
    else {
        cmd += " --server=" + d->m_server;
    }
    qDebug() << cmd;

    d->m_process.start(cmd);
    if (!d->m_process.waitForStarted()) {
        qWarning() << "Error starting process";
        emit searchFinished();
    }
}

bool LocateManagerWorker::wasStarted() const
{
    return d->m_was_started;
}

bool LocateManagerWorker::isFinished() const
{
    return d->m_is_finished;
}

void LocateManagerWorker::slot_process_finished()
{
    QString output = d->m_process.readAll().trimmed();
    qDebug() << output;
    if (output.isEmpty()) {
        d->m_result.state = fuzzybool::NO;
    }
    else {
        if (d->m_server.isEmpty()) {
            if (QFile::exists(output)) {
                d->m_result.state = fuzzybool::YES;
                d->m_result.path_or_url = output;
            }
            else {
                qWarning() << "Output of process is not an existing file: " + output;
                d->m_result.state = fuzzybool::NO;
            }
        }
        else {
            if (output.startsWith("http")) {
                d->m_result.state = fuzzybool::YES;
                qDebug() << "----------------------------------------------" << output;
                d->m_result.path_or_url = output;
            }
            else {
                qWarning() << "Output of process does not start with http: " + output;
                d->m_result.state = fuzzybool::NO;
            }
        }
    }

    d->m_is_finished = true;
    emit searchFinished();
}
