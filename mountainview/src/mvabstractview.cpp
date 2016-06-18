/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/15/2016
*******************************************************/

#include "mvabstractview.h"
#include <QAction>
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
    bool m_calculation_scheduled;
    bool m_recalculate_suggested;

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
        QAction* a = new QAction("Recalculate", this);
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(recalculate()));
    }

    /*
     // just thinking out loud
    QToolButton *recalculate_button=new QToolButton(this);
    recalculate_button->setText("Recalculate");
    recalculate_button->setAutoRaise(true);
    recalculate_button->setBackgroundRole(QPalette::Window);
    connect(recalculate_button,SIGNAL(clicked(bool)),this,SLOT(recalculate()));
    */

    setContextMenuPolicy(Qt::ActionsContextMenu);
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

bool MVAbstractView::recalculateSuggested()
{
    return d->m_recalculate_suggested;
}

MVAbstractViewFactory *MVAbstractView::viewFactory() const
{
    return 0;
}

MVViewAgent* MVAbstractView::viewAgent()
{
    return d->m_view_agent;
}

MVAbstractView::ViewFeatures MVAbstractView::viewFeatures() const
{
    return NoFeatures; // no features by default
}

void MVAbstractView::renderView(QPainter *painter)
{
    // do nothing in the base class
}

void MVAbstractView::recalculateOnOptionChanged(QString name)
{
    d->m_recalculate_on_option_names.insert(name);
}

void MVAbstractView::recalculateOn(QObject* obj, const char* signal)
{
    QObject::connect(obj, signal, this, SLOT(slot_suggest_recalculate()));
}

void MVAbstractView::recalculate()
{
    d->stop_calculation();
    d->schedule_calculation();
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
    if (m_recalculate_suggested == val)
        return;
    m_recalculate_suggested = val;
    emit q->recalculateSuggestedChanged();
}
