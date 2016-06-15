/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/15/2016
*******************************************************/

#include "mvabstractview.h"
#include <QThread>
#include <QTimer>

class CalculationThread : public QThread {
public:
    void run();
    MVAbstractView* q;
};

class MVAbstractViewPrivate {
public:
    MVAbstractView* q;
    MVViewAgent* m_view_agent;
    QSet<QString> m_recalculate_on_option_names;
    bool m_calculation_scheduled;

    CalculationThread m_calculation_thread;

    void stop_calculation();
    void schedule_calculation();
};

MVAbstractView::MVAbstractView(MVViewAgent* view_agent)
{
    d = new MVAbstractViewPrivate;
    d->q = this;
    d->m_calculation_thread.q = this;

    d->m_view_agent = view_agent;
    d->m_calculation_scheduled = false;

    QObject::connect(&d->m_calculation_thread, SIGNAL(finished()), this, SLOT(slot_calculation_finished()));
    QObject::connect(view_agent, SIGNAL(optionChanged(QString)), this, SLOT(slot_view_agent_option_changed(QString)));
}

MVAbstractView::~MVAbstractView()
{
    d->stop_calculation();
    delete d;
}

bool MVAbstractView::isCalculating()
{
    return d->m_calculation_thread.isRunning();
}

MVViewAgent* MVAbstractView::viewAgent()
{
    return d->m_view_agent;
}

void MVAbstractView::recalculateOnOptionChanged(QString name)
{
    d->m_recalculate_on_option_names.insert(name);
}

void MVAbstractView::recalculateOn(QObject* obj, const char* signal)
{
    QObject::connect(obj, signal, this, SLOT(recalculate()));
}

void MVAbstractView::recalculate()
{
    qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    d->stop_calculation();
    qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    d->schedule_calculation();
    qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
}

void MVAbstractView::slot_do_calculation()
{
    qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    emit this->calculationStarted();
    prepareCalculation();
    qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
    d->m_calculation_thread.start();
    qDebug() << __FUNCTION__  << __FILE__ << __LINE__;
}

void MVAbstractView::slot_calculation_finished()
{
    if (d->m_calculation_thread.isInterruptionRequested()) {
        qWarning() << "Calculation interrupted";
        emit this->calculationFinished();
        return;
    }
    this->onCalculationFinished();
    emit this->calculationFinished();
}

void MVAbstractView::slot_view_agent_option_changed(QString name)
{
    if (d->m_recalculate_on_option_names.contains(name)) {
        recalculate();
    }
}

void CalculationThread::run()
{
    q->runCalculation();
}

void MVAbstractViewPrivate::stop_calculation()
{
    if (m_calculation_thread.isRunning()) {
        m_calculation_thread.requestInterruption();
        m_calculation_thread.wait();
    }
}

void MVAbstractViewPrivate::schedule_calculation()
{
    if (m_calculation_scheduled)
        return;
    m_calculation_scheduled = true;
    QTimer::singleShot(100, q, SLOT(slot_do_calculation()));
}
