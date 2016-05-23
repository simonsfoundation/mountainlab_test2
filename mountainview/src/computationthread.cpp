/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#include "computationthread.h"

#include <QMutex>
#include <QTimer>
#include <QDebug>

class ComputationThreadPrivate {
public:
    ComputationThread* q;
    bool m_delete_on_complete;
    bool m_is_computing;
    bool m_stop_requested;
    bool m_is_finished;
    QString m_error_message;
    QMutex m_mutex;
    QMutex m_status_mutex;
    bool m_start_scheduled;
    long m_randomization_seed;

    void schedule_start();
};

ComputationThread::ComputationThread()
{
    d = new ComputationThreadPrivate;
    d->q = this;
    d->m_is_computing = false;
    d->m_stop_requested = false;
    d->m_is_finished = false;
    d->m_start_scheduled = false;
    d->m_delete_on_complete = false;
}

ComputationThread::~ComputationThread()
{
    delete d;
}

void ComputationThread::setDeleteOnComplete(bool val)
{
    d->m_delete_on_complete = val;
}

void ComputationThread::startComputation()
{
    stopComputation();
    {
        QMutexLocker locker(&d->m_mutex);
        d->m_is_computing = true;
        d->m_stop_requested = false;
        d->m_is_finished = false;
        d->m_error_message = "";
    }
    d->schedule_start();
}

void ComputationThread::stopComputation()
{
    {
        QMutexLocker locker(&d->m_mutex);
        if (!d->m_is_computing)
            return;
        d->m_stop_requested = true; //attempt to end gracefully
    }
    if (this->isRunning()) {
        this->wait(1000); //we will wait a second
    }
    if (this->isRunning()) {
        qWarning() << "TERMINATING COMPUTATION";
        this->terminate();
    }
    if (this->isRunning()) {
        this->wait(1000);
    }
    if (this->isRunning()) {
        qWarning() << "Unable to terminate computation thread";
    }
}

bool ComputationThread::isComputing()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_is_computing;
}

bool ComputationThread::isFinished()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_is_finished;
}

bool ComputationThread::hasError()
{
    QMutexLocker locker(&d->m_mutex);
    return !d->m_error_message.isEmpty();
}

QString ComputationThread::errorMessage()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_error_message;
}

bool ComputationThread::stopRequested()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_stop_requested;
}

void ComputationThread::setErrorMessage(const QString& error)
{
    QMutexLocker locker(&d->m_mutex);
    d->m_error_message = error;
}

void ComputationThread::run()
{
    {
        qsrand(d->m_randomization_seed);
        compute();
    }
    if (!stopRequested()) {
        QMutexLocker locker(&d->m_mutex);
        d->m_is_finished = true;
        d->m_is_computing = false;
        emit computationFinished();
    }
    else {
    }
    if (d->m_delete_on_complete)
        this->deleteLater();
}

void ComputationThread::slot_start()
{
    d->m_randomization_seed = qrand();
    {
        QMutexLocker locker(&d->m_mutex);
        if (d->m_stop_requested)
            return;
        d->m_start_scheduled = false;
    }
    this->start();
}

void ComputationThreadPrivate::schedule_start()
{
    QMutexLocker locker(&m_mutex);
    if (m_start_scheduled)
        return;
    m_stop_requested = false;
    QTimer::singleShot(100, q, SLOT(slot_start()));
    m_start_scheduled = true;
}
