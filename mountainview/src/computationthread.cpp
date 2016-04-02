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
    ComputationThread *q;
    bool m_is_computing;
    bool m_stop_requested;
    bool m_is_finished;
    QString m_error_message;
    QMutex m_mutex;
    bool m_start_scheduled;

    void schedule_start();

};

ComputationThread::ComputationThread()
{
    d=new ComputationThreadPrivate;
    d->q=this;
    d->m_is_computing=false;
    d->m_stop_requested=false;
    d->m_is_finished=false;
    d->m_start_scheduled=false;
}

ComputationThread::~ComputationThread()
{
    delete d;
}


void ComputationThread::startComputation()
{

	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    stopComputation();
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
	{
		QMutexLocker locker(&d->m_mutex);
		d->m_is_computing=true;
		d->m_stop_requested=false;
		d->m_is_finished=false;
		d->m_error_message="";
	}
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    d->schedule_start();
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
}

void ComputationThread::stopComputation()
{
    {
        QMutexLocker locker(&d->m_mutex);
        if (!d->m_is_computing) return;
        d->m_stop_requested=true; //attempt to end gracefully
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

void ComputationThread::setErrorMessage(const QString &error)
{
    QMutexLocker locker(&d->m_mutex);
    d->m_error_message=error;
}

void ComputationThread::run()
{
	{
		compute();
	}
    if (!stopRequested()) {
		QMutexLocker locker(&d->m_mutex);
        d->m_is_finished=true;
        d->m_is_computing=false;
        emit computationFinished();
    }
}

void ComputationThread::slot_start()
{
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    {
        QMutexLocker locker(&d->m_mutex);
        if (d->m_stop_requested) return;
        d->m_start_scheduled=false;
    }
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    this->start();
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
}

void ComputationThreadPrivate::schedule_start()
{
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    QMutexLocker locker(&m_mutex);
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    if (m_start_scheduled) return;
    m_stop_requested=false;
    QTimer::singleShot(100,q,SLOT(slot_start()));
    m_start_scheduled=true;
	qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
}
