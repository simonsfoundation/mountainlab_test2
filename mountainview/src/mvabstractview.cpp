/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/15/2016
*******************************************************/

#include "mvabstractview.h"
#include <QAction>
#include <QJsonArray>
#include <QMenu>
#include <QThread>
#include <QTimer>
#include <QToolButton>

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
    QSet<QString> m_suggest_recalculate_on_option_names;
    bool m_calculation_scheduled;
    bool m_recalculate_suggested;
    bool m_never_suggest_recalculate = false;

    CalculationThread m_calculation_thread;

    void stop_calculation();
    void schedule_calculation();
    void set_recalculate_suggested(bool val);
};

MVAbstractView::MVAbstractView(MVViewAgent* view_agent)
{
    d = new MVAbstractViewPrivate;
    d->q = this;
    d->m_calculation_thread.q = this;
    d->m_recalculate_suggested = false;

    d->m_view_agent = view_agent;
    d->m_calculation_scheduled = false;

    QObject::connect(&d->m_calculation_thread, SIGNAL(finished()), this, SLOT(slot_calculation_finished()));
    QObject::connect(view_agent, SIGNAL(optionChanged(QString)), this, SLOT(slot_view_agent_option_changed(QString)));

    {
        QAction* a = new QAction(QIcon(":/image/gear.png"), "Force recalculate", this);
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(recalculate()));
    }
}

MVAbstractView::~MVAbstractView()
{
    d->stop_calculation();
    delete d;
}

bool MVAbstractView::isCalculating() const
{
    return d->m_calculation_thread.isRunning();
}

bool MVAbstractView::recalculateSuggested() const
{
    return ((!d->m_never_suggest_recalculate) && (d->m_recalculate_suggested));
}

MVViewAgent* MVAbstractView::viewAgent()
{
    return d->m_view_agent;
}

void MVAbstractView::recalculateOnOptionChanged(QString name, bool suggest_only)
{
    if (suggest_only)
        d->m_suggest_recalculate_on_option_names.insert(name);
    else
        d->m_recalculate_on_option_names.insert(name);
}

void MVAbstractView::recalculateOn(QObject* obj, const char* signal, bool suggest_only)
{
    if (suggest_only)
        QObject::connect(obj, signal, this, SLOT(slot_suggest_recalculate()));
    else
        QObject::connect(obj, signal, this, SLOT(recalculate()));
}

void MVAbstractView::recalculate()
{
    d->stop_calculation();
    d->schedule_calculation();
}

void MVAbstractView::neverSuggestRecalculate()
{
    d->m_never_suggest_recalculate = true;
    if (d->m_recalculate_suggested) {
        d->m_recalculate_suggested = false;
        emit recalculateSuggestedChanged();
    }
}

void MVAbstractView::slot_do_calculation()
{
    d->set_recalculate_suggested(false);
    emit this->calculationStarted();
    this->update();
    prepareCalculation();
    d->m_calculation_thread.start();
    d->m_calculation_scheduled = false;
    this->update();
}

void MVAbstractView::slot_calculation_finished()
{
    if (d->m_calculation_thread.isInterruptionRequested()) {
        qWarning() << "Calculation interrupted";
        emit this->calculationFinished();
        return;
    }
    this->onCalculationFinished();
    this->update();
    emit this->calculationFinished();
    d->set_recalculate_suggested(false);
}

void MVAbstractView::slot_view_agent_option_changed(QString name)
{
    if (d->m_recalculate_on_option_names.contains(name)) {
        recalculate();
    }
    if (d->m_suggest_recalculate_on_option_names.contains(name)) {
        slot_suggest_recalculate();
    }
}

void MVAbstractView::slot_suggest_recalculate()
{
    d->set_recalculate_suggested(true);
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

void MVAbstractViewPrivate::set_recalculate_suggested(bool val)
{
    if (m_never_suggest_recalculate) {
        val = false;
    }
    if (m_recalculate_suggested == val)
        return;
    m_recalculate_suggested = val;
    emit q->recalculateSuggestedChanged();
}
